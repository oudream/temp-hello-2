#ifndef CX_CT_X2_YAML_HELPER_H
#define CX_CT_X2_YAML_HELPER_H


#include <ccxx/configuration.h>

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <unordered_set>

namespace cfg {

// 由于不使用 Visitor，这里通过 dynamic_cast 分派 Field* 的具体类型
    class ConfigIOYAML : public ConfigIOBase {
    public:
        bool loadFromFile(ConfigItem *root, const std::string &path, std::string *err) override {
            try {
                YAML::Node doc = YAML::LoadFile(path);
                if (!doc || !doc.IsMap()) {
                    if (err) *err = "root is not a map";
                    return false;
                }
                ErrorCollector ec;
                loadItem(doc, root, ec);
                if (!ec.empty()) {
                    if (err) *err = ec.join();
                    return false;
                }
                return true;
            }
            catch (const std::exception &ex) {
                if (err) *err = std::string("yaml load error: ") + ex.what();
                return false;
            }
        }

        bool saveToFile(const ConfigItem *root, const std::string &path, std::string *err) override {
            try {
                YAML::Node doc(YAML::NodeType::Map);
                dumpItem(doc, root);
                std::ofstream ofs(path);
                ofs << doc;
                return true;
            }
            catch (const std::exception &ex) {
                if (err) *err = std::string("yaml save error: ") + ex.what();
                return false;
            }
        }

    private:
        static void loadItem(const YAML::Node &n, ConfigItem *it, ErrorCollector &ec) {
            for (ConfigItem::VF::size_type fi = 0; fi < it->fieldCount(); ++fi) {
                FieldBase *f = it->fieldAt(fi);
                auto m = n[f->key];
                if (!m) {
                    if (f->required) ec.add("missing required key: " + f->key);
                    return;
                }
                // 标量
                if (auto *b = dynamic_cast<FieldValue<bool> *>(f)) {
                    try { b->value = m.as<bool>(); }
                    catch (...) { ec.add(typeErr(f->key, "bool")); }
                    continue;
                }
                if (auto *x = dynamic_cast<FieldValue<int> *>(f)) {
                    try {
                        auto v = m.as<int>();
                        if (auto e = validateInt(v, f->v)) ec.add(f->key + ": " + *e);
                        x->value = v;
                    }
                    catch (...) { ec.add(typeErr(f->key, "int")); }
                    continue;
                }
                if (auto *d = dynamic_cast<FieldValue<double> *>(f)) {
                    try {
                        auto v = m.as<double>();
                        if (auto e = validateReal(v, f->v)) ec.add(f->key + ": " + *e);
                        d->value = v;
                    }
                    catch (...) { ec.add(typeErr(f->key, "double")); }
                    continue;
                }
                if (auto *s = dynamic_cast<FieldValue<std::string> *>(f)) {
                    try {
                        auto v = m.as<std::string>();
                        if (auto e = validateString(v, f->v)) ec.add(f->key + ": " + *e);
                        s->value = v;
                    }
                    catch (...) { ec.add(typeErr(f->key, "string")); }
                    continue;
                }
                if (auto *ll = dynamic_cast<FieldValue<long long> *>(f)) {
                    try {
                        if (m.IsScalar()) {
                            auto s = m.as<std::string>();
                            auto ms = parseDateToMillis(s, PrimarySemantic::DateTimeLong);
                            if (ms) ll->value = *ms; else ec.add(f->key + ": bad date format");
                        } else {
                            ec.add(typeErr(f->key, "long long, not is scalar"));
                        }
                    }
                    catch (...) { ec.add(typeErr(f->key, "long long")); }
                    continue;
                }

                // 数组
                if (auto *ai = dynamic_cast<FieldArray<int> *>(f)) {
                    loadArrayNumeric<int>(m, *ai, ec);
                    continue;
                }
                if (auto *ad = dynamic_cast<FieldArray<double> *>(f)) {
                    loadArrayNumeric<double>(m, *ad, ec);
                    continue;
                }
                if (auto *as = dynamic_cast<FieldArray<std::string> *>(f)) {
                    loadArrayString(m, *as, ec);
                    continue;
                }
                if (auto *al = dynamic_cast<FieldArray<long long> *>(f)) {
                    loadArrayNumeric<long long>(m, *al, ec);
                    continue;
                }

                // 对象
                if (auto *fo = dynamic_cast<FieldObjectBase *>(f)) {
                    if (!m.IsMap()) {
                        ec.add(typeErr(f->key, "object"));
                        continue;
                    }
                    loadItem(m, fo->getItem(), ec);
                    continue;
                }
                if (auto *fao = dynamic_cast<FieldArrayObjectBase *>(f)) {
                    if (!m.IsSequence()) {
                        ec.add(typeErr(f->key, "array(object)"));
                        continue;
                    }
                    fao->resize(m.size());
                    for (size_t i = 0; i < m.size(); ++i) {
                        auto ni = m[i];
                        if (!ni.IsMap()) {
                            ec.add(typeErr(f->key + "[" + std::to_string(i) + "]", "object"));
                            continue;
                        }
                        loadItem(ni, fao->at(i), ec);
                    }
                    continue;
                }
            }
        }

        static void dumpItem(YAML::Node &out, const ConfigItem *it) {
            for (ConfigItem::VF::size_type fi = 0; fi < it->fieldCount(); ++fi) {
                const FieldBase *f = it->fieldAt(fi);
                switch (f->kind) {
                    case ValueKind::kBool: {
                        auto *b = (const FieldValue<bool> *) f;
                        out[f->key] = b->value;
                        break;
                    }
                    case ValueKind::kInt: {
                        auto *x = (const FieldValue<int> *) f;
                        out[f->key] = x->value;
                        break;
                    }
                    case ValueKind::kInt64: {
                        auto *ll = (const FieldValue<long long> *) f;
                        out[f->key] = formatMillisAsString(ll->value, PrimarySemantic::DateTimeLong);
                        break;
                    }
                    case ValueKind::kDouble: {
                        auto *d = (const FieldValue<double> *) f;
                        out[f->key] = d->value;
                        break;
                    }
                    case ValueKind::kString: {
                        auto *s = (const FieldValue<std::string> *) f;
                        out[f->key] = s->value;
                        break;
                    }

                    case ValueKind::kIntArray: {
                        auto *ai = (const FieldArray<int> *) f;
                        auto arr = out[f->key];
                        arr = YAML::Node(YAML::NodeType::Sequence);
                        for (auto v: ai->value) arr.push_back(v);
                        break;
                    }
                    case ValueKind::kInt64Array: {
                        auto *al = (const FieldArray<long long> *) f;
                        auto arr = out[f->key];
                        arr = YAML::Node(YAML::NodeType::Sequence);
                        for (auto v: al->value) {
                            arr.push_back(formatMillisAsString(v, PrimarySemantic::DateTimeLong));
//                            if (f->v.primary == PrimarySemantic::DateShort ||
//                                f->v.primary == PrimarySemantic::DateTimeLong) {
//                                arr.push_back(formatMillisAsString(v, f->v.primary));
//                            } else {
//                                arr.push_back(v);
//                            }
                        }
                        break;
                    }
                    case ValueKind::kDoubleArray: {
                        auto *ad = (const FieldArray<double> *) f;
                        auto arr = out[f->key];
                        arr = YAML::Node(YAML::NodeType::Sequence);
                        for (auto v: ad->value) arr.push_back(v);
                        break;
                    }
                    case ValueKind::kStringArray: {
                        auto *as = (const FieldArray<std::string> *) f;
                        auto arr = out[f->key];
                        arr = YAML::Node(YAML::NodeType::Sequence);
                        for (auto &v: as->value) arr.push_back(v);
                        break;
                    }

                    case ValueKind::kObject: {
                        auto *fo = (const FieldObjectBase *) f;
                        auto n = out[f->key];
                        n = YAML::Node(YAML::NodeType::Map);
                        dumpItem(n, fo->getItem());
                        break;
                    }
                    case ValueKind::kObjectArray: {
                        auto *fao = (const FieldArrayObjectBase *) f;
                        auto n = out[f->key];
                        n = YAML::Node(YAML::NodeType::Sequence);
                        for (size_t i = 0; i < fao->size(); ++i) {
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

        template<typename T>
        static void loadArrayNumeric(const YAML::Node &n, FieldArray<T> &f, ErrorCollector &ec) {
            if (!n.IsSequence()) {
                ec.add(typeErr(f.key, "array"));
                return;
            }
            f.value.clear();
            f.value.reserve(n.size());
            std::unordered_set<long long> uniq;
            for (auto it: n) {
                try {
                    if constexpr (std::is_same_v<T, long long>) {
                        if ((
                                    f.v.primary == PrimarySemantic::DateShort ||
                                    f.v.primary == PrimarySemantic::DateTimeLong) && it.IsScalar()) {
                            auto s = it.as<std::string>();
                            auto ms = parseDateToMillis(s, f.v.primary);
                            if (ms) {
                                f.value.push_back(*ms);
                                if (f.v.uniqueItems) uniq.insert(*ms);
                                continue;
                            }
                        }
                    }
                    T x = it.as<T>();
                    f.value.push_back(x);
                    if (f.v.uniqueItems) uniq.insert((long long) x);
                }
                catch (...) { ec.add(typeErr(f.key, "array<scalar>")); }
            }
            if (f.v.nonEmpty && f.value.empty()) ec.add(f.key + ": array empty");
            if (f.v.uniqueItems && uniq.size() != f.value.size()) ec.add(f.key + ": array not unique");
        }

        static void loadArrayString(const YAML::Node &n, FieldArray<std::string> &f, ErrorCollector &ec) {
            if (!n.IsSequence()) {
                ec.add(typeErr(f.key, "array"));
                return;
            }
            f.value.clear();
            f.value.reserve(n.size());
            std::unordered_set<std::string> uniq;
            for (auto it: n) {
                try {
                    auto s = it.as<std::string>();
                    if (auto e = validateString(s, f.v)) ec.add(f.key + ": " + *e);
                    f.value.push_back(s);
                    if (f.v.uniqueItems) uniq.insert(s);
                }
                catch (...) { ec.add(typeErr(f.key, "array<string>")); }
            }
            if (f.v.nonEmpty && f.value.empty()) ec.add(f.key + ": array empty");
            if (f.v.uniqueItems && uniq.size() != f.value.size()) ec.add(f.key + ": array not unique");
        }

        template<typename T>
        static void dumpArray(YAML::Node &out, const std::string &key, const std::vector<T> &v) {
            auto arr = out[key];
            arr = YAML::Node(YAML::NodeType::Sequence);
            for (auto &x: v) arr.push_back(x);
        }

        static std::string typeErr(const std::string &k, const char *expect) {
            return "type error at key: " + k + ", expect " + expect;
        }
    };

} // namespace cfg

#endif //CX_CT_X2_YAML_HELPER_H
