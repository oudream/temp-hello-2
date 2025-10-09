#include "yaml_helper.h"


#include <ccxx/datetime.h>

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <unordered_set>


namespace cfg
{
    static std::string typeErr(const std::string &k, const char *expect)
    {
        return "type error at key: " + k + ", expect " + expect;
    }

    // 统一的整型读取模板
    template<typename T>
    static bool loadIntegralScalar(const YAML::Node &m, T &out, std::string &errKey)
    {
        if (!m.IsScalar()) return false;

        try
        {
            using Lim = std::numeric_limits<T>;
            if constexpr (std::is_signed_v<T>)
            {
                auto v = m.as<long long>();
                if (v < static_cast<long long>((Lim::min)()) || v > static_cast<long long>((Lim::max)())) return false;
                out = static_cast<T>(v);
            }
            else
            {
                // 注意：负号要先拦截，否则 YAML 会把 "-1" 也当成 unsigned long long 溢出
                auto s = m.as<std::string>();
                if (!s.empty() && s[0] == '-') return false;
                auto v = m.as<unsigned long long>();
                if (v > static_cast<unsigned long long>((Lim::max)())) return false;
                out = static_cast<T>(v);
            }
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    // 统一的整型数组读取模板
    template<typename T>
    static bool loadIntegralArray(const YAML::Node &m, std::vector<T> &out, bool nonEmpty, bool unique, std::string &errKey)
    {
        if (!m.IsSequence()) return false;
        out.clear();
        out.reserve(m.size());
        std::unordered_set<long long> uniqS;
        std::unordered_set<unsigned long long> uniqU;

        for (auto it: m)
        {
            T tmp{};
            if (!loadIntegralScalar<T>(it, tmp, errKey)) return false;
            out.push_back(tmp);
            if (unique)
            {
                if constexpr (std::is_signed_v<T>) uniqS.insert(static_cast<long long>(tmp));
                else uniqU.insert(static_cast<unsigned long long>(tmp));
            }
        }
        if (nonEmpty && out.empty()) return false;
        if (unique)
        {
            const bool ok = (std::is_signed_v<T> ? uniqS.size() == out.size() : uniqU.size() == out.size());
            if (!ok) return false;
        }
        return true;
    }

    // 统一的浮点读取模板（float/double 都能用）
    template<typename T>
    static bool loadFloatScalar(const YAML::Node &m, T &out)
    {
        if (!m.IsScalar()) return false;
        try
        {
            auto v = m.as<double>();   // YAML 没有 float，统一按 double 读
            if (!std::isfinite(v)) return false;
            out = static_cast<T>(v);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    // 统一的浮点数组读取模板（float/double 都能用）
    template<typename T>
    static bool loadFloatArray(const YAML::Node &m, std::vector<T> &out, bool nonEmpty)
    {
        if (!m.IsSequence()) return false;
        out.clear();
        out.reserve(m.size());
        for (auto it: m)
        {
            T tmp{};
            if (!loadFloatScalar<T>(it, tmp)) return false;
            out.push_back(tmp);
        }
        if (nonEmpty && out.empty()) return false;
        return true;
    }

    static void loadArrayString(const YAML::Node &n, FieldArray<std::string> &f, ErrorCollector &ec)
    {
        if (!n.IsSequence())
        {
            ec.add(typeErr(f.key, "array"));
            return;
        }
        f.value.clear();
        f.value.reserve(n.size());
        std::unordered_set<std::string> uniq;
        for (auto it: n)
        {
            try
            {
                auto s = it.as<std::string>();
                if (auto e = validateString(s, f.v)) ec.add(f.key + ": " + *e);
                f.value.push_back(s);
                if (f.v.uniqueItems) uniq.insert(s);
            }
            catch (...)
            {
                ec.add(typeErr(f.key, "array<string>"));
            }
        }
        if (f.v.nonEmpty && f.value.empty())
        {
            ec.add(f.key + ": array empty");
        }
        if (f.v.uniqueItems && uniq.size() != f.value.size())
        {
            ec.add(f.key + ": array not unique");
        }
    }

    // 由于不使用 Visitor，这里通过 dynamic_cast 分派 Field* 的具体类型
    static void loadItem(const YAML::Node &n, ConfigItem *it, ErrorCollector &ec)
    {
        for (ConfigItem::VF::size_type fi = 0; fi < it->fieldCount(); ++fi)
        {
            FieldBase *f = it->fieldAt(fi);
            auto m = n[f->key];
            if (!m)
            {
                if (f->required) ec.add("missing required key: " + f->key);
                continue;
            }

                // 标量整型族
#define HANDLE_INT_SCALAR(T) \
            if (auto* fv = dynamic_cast<FieldValue<T>*>(f)) { \
                if (!loadIntegralScalar<T>(m, fv->value, /*errKey*/f->key)) ec.add(typeErr(f->key, "integer")); \
                continue; \
            }

            HANDLE_INT_SCALAR(signed char)
            HANDLE_INT_SCALAR(unsigned char)
            HANDLE_INT_SCALAR(short)
            HANDLE_INT_SCALAR(unsigned short)
            HANDLE_INT_SCALAR(int)
            HANDLE_INT_SCALAR(unsigned int)
            HANDLE_INT_SCALAR(long long)
            HANDLE_INT_SCALAR(unsigned long long)
#undef HANDLE_INT_SCALAR

            // 标量 float double
            if (auto *fv = dynamic_cast<FieldValue<float> *>(f))
            {
                if (!loadFloatScalar<float>(m, fv->value))
                {
                    ec.add(typeErr(f->key, "float"));
                }
                continue;
            }
            if (auto *fv = dynamic_cast<FieldValue<double> *>(f))
            {
                if (!loadFloatScalar<double>(m, fv->value))
                {
                    ec.add(typeErr(f->key, "double"));
                }
                continue;
            }
            if (auto *s = dynamic_cast<FieldValue<std::string> *>(f))
            {
                try
                {
                    auto v = m.as<std::string>();
                    if (auto e = validateString(v, f->v)) ec.add(f->key + ": " + *e);
                    s->value = v;
                }
                catch (...)
                {
                    ec.add(typeErr(f->key, "string"));
                }
                continue;
            }
            if (auto *dt = dynamic_cast<FieldValue<cx::DateTime> *>(f))
            {
                try
                {
                    if (!m.IsScalar())
                    {
                        ec.add(typeErr(f->key, "datetime string"));
                    }
                    else
                    {
                        auto s = m.as<std::string>();
                        cx::DateTime parsed;
                        if (s.empty())
                        {
                            dt->value = cx::DateTime::invalid();
                        }
                        else
                        {
                            if (cx::DateTime::tryParse(s, parsed, cx::DateTime::Kind::Unspecified))
                            {
                                dt->value = parsed;
                            }
                            else
                            {
                                ec.add(f->key + ": bad datetime format");
                            }
                        }
                    }
                }
                catch (...)
                {
                    ec.add(typeErr(f->key, "datetime string"));
                }
                continue;
            }

                // 数组整型族
#define HANDLE_INT_ARRAY(T) \
            if (auto* fa = dynamic_cast<FieldArray<T>*>(f)) { \
                if (!loadIntegralArray<T>(m, fa->value, f->v.nonEmpty, f->v.uniqueItems, /*errKey*/f->key)) \
                    ec.add(typeErr(f->key, "array(integer)")); \
                continue; \
            }

            HANDLE_INT_ARRAY(signed char)
            HANDLE_INT_ARRAY(unsigned char)
            HANDLE_INT_ARRAY(short)
            HANDLE_INT_ARRAY(unsigned short)
            HANDLE_INT_ARRAY(int)
            HANDLE_INT_ARRAY(unsigned int)
            HANDLE_INT_ARRAY(long long)
            HANDLE_INT_ARRAY(unsigned long long)
#undef HANDLE_INT_ARRAY

            // 数组 float double
            if (auto *fa = dynamic_cast<FieldArray<float> *>(f))
            {
                if (!loadFloatArray<float>(m, fa->value, f->v.nonEmpty)) {
                    ec.add(typeErr(f->key, "array(float)"));
                }
                continue;
            }
            if (auto *fa = dynamic_cast<FieldArray<double> *>(f))
            {
                if (!loadFloatArray<double>(m, fa->value, f->v.nonEmpty)) {
                    ec.add(typeErr(f->key, "array(double)"));
                }
                continue;
            }
            if (auto *as = dynamic_cast<FieldArray<std::string> *>(f))
            {
                loadArrayString(m, *as, ec);
                continue;
            }
            if (auto *adt = dynamic_cast<FieldArray<cx::DateTime> *>(f))
            {
                if (!m.IsSequence())
                {
                    ec.add(typeErr(f->key, "array(datetime string)"));
                    continue;
                }
                adt->value.clear();
                adt->value.reserve(m.size());
                std::unordered_set<std::string> uniq; // 用 ISO 文本判断唯一
                for (auto itV: m)
                {
                    try
                    {
                        auto s = itV.as<std::string>();
                        cx::DateTime parsed;
                        if (cx::DateTime::tryParse(s, parsed, cx::DateTime::Kind::Unspecified) && parsed.isValid())
                        {
                            adt->value.push_back(parsed);
                            if (f->v.uniqueItems) uniq.insert(parsed.toIso8601());
                        }
                        else
                        {
                            ec.add(f->key + ": bad datetime format in array");
                        }
                    }
                    catch (...)
                    {
                        ec.add(typeErr(f->key, "array<datetime string>"));
                    }
                }
                if (f->v.nonEmpty && adt->value.empty()) ec.add(f->key + ": array empty");
                if (f->v.uniqueItems && uniq.size() != adt->value.size()) ec.add(f->key + ": array not unique");
                continue;
            }

            // 对象
            if (auto *fo = dynamic_cast<FieldObjectBase *>(f))
            {
                if (!m.IsMap())
                {
                    ec.add(typeErr(f->key, "object"));
                    continue;
                }
                loadItem(m, fo->getItem(), ec);
                continue;
            }
            if (auto *fao = dynamic_cast<FieldArrayObjectBase *>(f))
            {
                if (!m.IsSequence())
                {
                    ec.add(typeErr(f->key, "array(object)"));
                    continue;
                }
                fao->resize(m.size());
                for (size_t i = 0; i < m.size(); ++i)
                {
                    auto ni = m[i];
                    if (!ni.IsMap())
                    {
                        ec.add(typeErr(f->key + "[" + std::to_string(i) + "]", "object"));
                        continue;
                    }
                    loadItem(ni, fao->at(i), ec);
                }
                continue;
            }
        }
    }

    static void dumpItem(YAML::Node &out, const ConfigItem *it)
    {
        for (ConfigItem::VF::size_type fi = 0; fi < it->fieldCount(); ++fi)
        {
            const FieldBase *f = it->fieldAt(fi);
            switch (f->kind)
            {
                case ValueKind::kBool:
                {
                    auto *b = (const FieldValue<bool> *) f;
                    out[f->key] = b->value;
                    break;
                }
                case ValueKind::kInt:
                {
                    if (auto *p = dynamic_cast<const FieldValue<signed char> *>(f))
                    {
                        out[f->key] = (long long) p->value;
                        break;
                    }
                    if (auto *p = dynamic_cast<const FieldValue<unsigned char> *>(f))
                    {
                        out[f->key] = (unsigned long long) p->value;
                        break;
                    }
                    if (auto *p = dynamic_cast<const FieldValue<short> *>(f))
                    {
                        out[f->key] = (long long) p->value;
                        break;
                    }
                    if (auto *p = dynamic_cast<const FieldValue<unsigned short> *>(f))
                    {
                        out[f->key] = (unsigned long long) p->value;
                        break;
                    }
                    if (auto *p = dynamic_cast<const FieldValue<int> *>(f))
                    {
                        out[f->key] = (long long) p->value;
                        break;
                    }
                    if (auto *p = dynamic_cast<const FieldValue<unsigned int> *>(f))
                    {
                        out[f->key] = (unsigned long long) p->value;
                        break;
                    }
                    break; // 未匹配就让 default 吞掉
                }
                case ValueKind::kInt64:
                {
                    if (auto *p = dynamic_cast<const FieldValue<long long> *>(f))
                    {
                        out[f->key] = (long long) p->value;
                        break;
                    }
                    if (auto *p = dynamic_cast<const FieldValue<unsigned long long> *>(f))
                    {
                        out[f->key] = (unsigned long long) p->value;
                        break;
                    }
                    break;
                }
                case ValueKind::kDouble:
                {
                    if (auto *p = dynamic_cast<const FieldValue<float> *>(f))
                    {
                        out[f->key] = (double) p->value;
                        break;
                    }
                    if (auto *p = dynamic_cast<const FieldValue<double> *>(f))
                    {
                        out[f->key] = p->value;
                        break;
                    }
                    break;
                }
                case ValueKind::kString:
                {
                    auto *s = (const FieldValue<std::string> *) f;
                    out[f->key] = s->value;
                    break;
                }
                case ValueKind::kDateTime:
                {
                    auto *dt = (const FieldValue<cx::DateTime> *) f;
                    std::string v = dt->value.isValid() ? dt->value.toIso8601() : std::string{};
                    out[f->key] = v;
                    break;
                }
                    // 数组
                case ValueKind::kIntArray:
                {
                    auto arr = out[f->key] = YAML::Node(YAML::NodeType::Sequence);
                    if (auto *p = dynamic_cast<const FieldArray<signed char> *>(f))
                    {
                        for (auto v: p->value) arr.push_back((long long) v);
                        break;
                    }
                    if (auto *p = dynamic_cast<const FieldArray<unsigned char> *>(f))
                    {
                        for (auto v: p->value) arr.push_back((unsigned long long) v);
                        break;
                    }
                    if (auto *p = dynamic_cast<const FieldArray<short> *>(f))
                    {
                        for (auto v: p->value) arr.push_back((long long) v);
                        break;
                    }
                    if (auto *p = dynamic_cast<const FieldArray<unsigned short> *>(f))
                    {
                        for (auto v: p->value) arr.push_back((unsigned long long) v);
                        break;
                    }
                    if (auto *p = dynamic_cast<const FieldArray<int> *>(f))
                    {
                        for (auto v: p->value) arr.push_back((long long) v);
                        break;
                    }
                    if (auto *p = dynamic_cast<const FieldArray<unsigned int> *>(f))
                    {
                        for (auto v: p->value) arr.push_back((unsigned long long) v);
                        break;
                    }
                    break;
                }
                case ValueKind::kInt64Array:
                {
                    auto arr = out[f->key] = YAML::Node(YAML::NodeType::Sequence);
                    if (auto *p = dynamic_cast<const FieldArray<long long> *>(f))
                    {
                        for (auto v: p->value) arr.push_back((long long) v);
                        break;
                    }
                    if (auto *p = dynamic_cast<const FieldArray<unsigned long long> *>(f))
                    {
                        for (auto v: p->value) arr.push_back((unsigned long long) v);
                        break;
                    }
                    break;
                }
                case ValueKind::kDoubleArray:
                {
                    auto arr = out[f->key] = YAML::Node(YAML::NodeType::Sequence);
                    if (auto *p = dynamic_cast<const FieldArray<float> *>(f))
                    {
                        for (auto v: p->value) arr.push_back((double) v);
                        break;
                    }
                    if (auto *p = dynamic_cast<const FieldArray<double> *>(f))
                    {
                        for (auto v: p->value) arr.push_back(v);
                        break;
                    }
                    break;
                }
                case ValueKind::kStringArray:
                {
                    auto *as = (const FieldArray<std::string> *) f;
                    auto arr = out[f->key];
                    arr = YAML::Node(YAML::NodeType::Sequence);
                    for (auto &v: as->value) arr.push_back(v);
                    break;
                }
                case ValueKind::kDateTimeArray:
                {
                    auto *adt = (const FieldArray<cx::DateTime> *) f;
                    auto arr = out[f->key];
                    arr = YAML::Node(YAML::NodeType::Sequence);
                    for (auto const &v: adt->value)
                    {
                        arr.push_back(v.isValid() ? v.toIso8601() : std::string());
                    }
                    break;
                }
                    // 对象
                case ValueKind::kObject:
                {
                    auto *fo = (const FieldObjectBase *) f;
                    auto n = out[f->key];
                    n = YAML::Node(YAML::NodeType::Map);
                    dumpItem(n, fo->getItem());
                    break;
                }
                    // 对象数组
                case ValueKind::kObjectArray:
                {
                    auto *fao = (const FieldArrayObjectBase *) f;
                    auto n = out[f->key];
                    n = YAML::Node(YAML::NodeType::Sequence);
                    for (size_t i = 0; i < fao->size(); ++i)
                    {
                        n.push_back(YAML::Node(YAML::NodeType::Map));
                        auto elem = n[n.size() - 1];  // 取刚 push 的元素句柄
                        dumpItem(elem, fao->at(i));
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    bool ConfigIOYAML::loadFromFile(ConfigItem *root, const std::string &path, std::string *err)
    {
        try
        {
            YAML::Node doc = YAML::LoadFile(path);
            if (!doc || !doc.IsMap())
            {
                if (err) *err = "root is not a map";
                return false;
            }
            ErrorCollector ec;
            loadItem(doc, root, ec);
            if (!ec.empty())
            {
                if (err) *err = ec.join();
                return false;
            }
            return true;
        }
        catch (const std::exception &ex)
        {
            if (err) *err = std::string("yaml load error: ") + ex.what();
            return false;
        }
    }

    bool ConfigIOYAML::saveToFile(const ConfigItem *root, const std::string &path, std::string *err)
    {
        try
        {
            YAML::Node doc(YAML::NodeType::Map);
            dumpItem(doc, root);
            std::ofstream ofs(path);
            ofs << doc;
            return true;
        }
        catch (const std::exception &ex)
        {
            if (err) *err = std::string("yaml save error: ") + ex.what();
            return false;
        }
    }

} // namespace cfg

