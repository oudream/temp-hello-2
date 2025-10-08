
// ============================================================================
// cfg v6（精简版）：集中加载 + 统一拓扑/校验 + 有向环检测
// 要点：
//  - 保留“类层 / IO 层”分离；
//  - 移除 IFieldVisitor；
//  - 移除分散钩子（onValidate/onAfterLoad/onInit）；
//  - 统一在 Manager 中做加载、注册表构建、引用解析、环检测、基础校验；
//  - YAML 驱动在不使用 Visitor 的前提下，基于 RTTI dynamic_cast 处理各 Field*；
//  - 仍支持 long long 的日期/日期时间语义；支持 FieldRef<T> 做拓扑解析。
// 依赖：C++17；yaml-cpp >= 0.7
// ============================================================================

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <limits>
#include <regex>
#include <unordered_set>
#include <typeindex>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <functional>

namespace cfg {

// ============= 元信息与校验 =============

    enum class ValueKind {
        kBool, kInt, kInt64, kDouble, kString,
        kIntArray, kInt64Array, kDoubleArray, kStringArray,
        kObject, kObjectArray,
    };

    enum class PrimarySemantic {
        None = 0,
        DateShort,     // YYYY-MM-DD
        DateTimeLong,  // YYYY-MM-DD HH:mm:ss
        IPAddress,
        HostName,
        Port,
        FilePath,
        Email,
        RegionCode,
        URLPath
    };

    struct Validation {
        std::optional<long long> minInt, maxInt;
        std::optional<double> minReal, maxReal;
        std::optional<std::string> regex;    // String 正则
        std::vector<std::string> enumSet;  // String 枚举
        bool nonEmpty = false;            // String/Array 非空
        bool uniqueItems = false;            // Array 元素唯一
        PrimarySemantic primary = PrimarySemantic::None;

        static Validation IntRange(long long a, long long b, PrimarySemantic ps = PrimarySemantic::None) {
            Validation v;
            v.minInt = a;
            v.maxInt = b;
            v.primary = ps;
            return v;
        }

        static Validation RealRange(double a, double b, PrimarySemantic ps = PrimarySemantic::None) {
            Validation v;
            v.minReal = a;
            v.maxReal = b;
            v.primary = ps;
            return v;
        }

        static Validation
        StringRegex(const std::string &re, bool ne = true, PrimarySemantic ps = PrimarySemantic::None) {
            Validation v;
            v.regex = re;
            v.nonEmpty = ne;
            v.primary = ps;
            return v;
        }

        static Validation
        EnumSet(const std::vector<std::string> &es, bool ne = true, PrimarySemantic ps = PrimarySemantic::None) {
            Validation v;
            v.enumSet = es;
            v.nonEmpty = ne;
            v.primary = ps;
            return v;
        }

        static Validation Array(bool ne = false, bool uniq = false, PrimarySemantic ps = PrimarySemantic::None) {
            Validation v;
            v.nonEmpty = ne;
            v.uniqueItems = uniq;
            v.primary = ps;
            return v;
        }
    };

    struct ConfigMeta {
        std::string nodeName;
        std::string displayName;
        std::string comment;
    };

    struct ErrorCollector {
        std::vector<std::string> messages;

        void add(const std::string &m) { messages.push_back(m); }

        bool empty() const { return messages.empty(); }

        std::string join() const {
            std::ostringstream oss;
            for (auto &m: messages) oss << m << "";
            return oss.str();
        }
    };

// ============= Field 抽象层（无 Visitor） =============
    class FieldBase {
    public:
        std::string key;        // YAML/JSON key
        bool required = false;
        std::string displayName;
        std::string comment;
        Validation v;
        ValueKind kind = ValueKind::kString;

        FieldBase(const std::string &k, bool req, const std::string &dn, const std::string &cmt, Validation val,
                  ValueKind kd)
                : key(k), required(req), displayName(dn), comment(cmt),
                  v(val), kind(kd) {}

        virtual ~FieldBase() = default;
    };

    template<typename T>
    struct _KindMap;

    template<typename T>
    class FieldValue : public FieldBase {
    public:
        T value{};

        // 让它能像普通类型一样赋值
        FieldValue &operator=(const T &v) {
            value = v;
            return *this;
        }

        // 隐式转换成 T，方便直接用
        operator T() const { return value; }

        FieldValue(const std::string &k, T def, bool req, const std::string &dn, const std::string &cmt, Validation val)
                : FieldBase(k, req, dn, cmt, val, _KindMap<T>::kind),
                  value(def) {}
    };

    template<>
    struct _KindMap<bool> {
        static constexpr ValueKind kind = ValueKind::kBool;
    };
    template<>
    struct _KindMap<int> {
        static constexpr ValueKind kind = ValueKind::kInt;
    };
    template<>
    struct _KindMap<double> {
        static constexpr ValueKind kind = ValueKind::kDouble;
    };
    template<>
    struct _KindMap<std::string> {
        static constexpr ValueKind kind = ValueKind::kString;
    };
    template<>
    struct _KindMap<long long> {
        static constexpr ValueKind kind = ValueKind::kInt64;
    };

    template<typename T>
    struct _ArrKindMap;

    template<typename T>
    class FieldArray : public FieldBase {
    public:
        std::vector<T> value{};

        FieldArray(const std::string &k, std::vector<T> def, bool req, const std::string &dn, const std::string &cmt,
                   Validation val)
                : FieldBase(k, req, dn, cmt, val, _ArrKindMap<T>::kind),
                  value(def) {}
    };

    template<>
    struct _ArrKindMap<int> {
        static constexpr ValueKind kind = ValueKind::kIntArray;
    };
    template<>
    struct _ArrKindMap<double> {
        static constexpr ValueKind kind = ValueKind::kDoubleArray;
    };
    template<>
    struct _ArrKindMap<std::string> {
        static constexpr ValueKind kind = ValueKind::kStringArray;
    };
    template<>
    struct _ArrKindMap<long long> {
        static constexpr ValueKind kind = ValueKind::kInt64Array;
    };

    class ConfigItem; // 前置

    class FieldObjectBase : public FieldBase {
    public:
        using FieldBase::FieldBase;

        virtual ConfigItem *getItem() = 0;

        virtual const ConfigItem *getItem() const = 0;
    };

    template<typename T>
    class FieldObject : public FieldObjectBase {
    public:
        static_assert(std::is_base_of<ConfigItem, T>::value, "T must derive from ConfigItem");
        T obj{};

        FieldObject(const std::string &k, bool req, const std::string &dn, const std::string &cmt, Validation val)
                : FieldObjectBase(k, req, dn, cmt, val, ValueKind::kObject) {}

        ConfigItem *getItem() override { return &obj; }

        const ConfigItem *getItem() const override { return &obj; }
    };

    class FieldArrayObjectBase : public FieldBase {
    public:
        using FieldBase::FieldBase;

        virtual size_t size() const = 0;

        virtual void resize(size_t n) = 0;

        virtual ConfigItem *at(size_t i) = 0;

        virtual const ConfigItem *at(size_t i) const = 0;
    };

    template<typename T>
    class FieldArrayObject : public FieldArrayObjectBase {
    protected:
        static_assert(std::is_base_of<ConfigItem, T>::value, "T must derive from ConfigItem");
        std::vector<T> arr{};

        using Self = FieldArrayObject<T>;
    public:
        FieldArrayObject(const std::string &k, bool req, const std::string &dn, const std::string &cmt, Validation val)
                : FieldArrayObjectBase(k, req, dn, cmt, val, ValueKind::kObjectArray) {}

        // --- 基本接口 ---
        size_t size() const override { return arr.size(); }

        void resize(size_t n) override { arr.resize(n); }

        ConfigItem *at(size_t i) override { return &arr.at(i); }

        const ConfigItem *at(size_t i) const override { return &arr.at(i); }

        // --- 便捷单次添加 ---
        template<typename Fn>
        Self &emplace(Fn &&fn) {
            arr.emplace_back();
            fn(arr.back());
            return *this;   // 注意这里返回 Self&，以便链式调用
        }

        // --- 一次性初始化 ---
        Self &init(std::initializer_list<T> initList) {
            arr.assign(initList);
            return *this;
        }

        // --- 支持链式 push ---
        Self &push(const T &obj) {
            arr.push_back(obj);
            return *this;
        }
    };

// ============= 工具函数 =============
    inline bool _regexMatch(const std::string &s, const std::string &re) {
        try { return std::regex_match(s, std::regex(re)); }
        catch (...) { return true; }
    }

    inline bool _inEnumSet(const std::string &s, const std::vector<std::string> &es) {
        for (auto &e: es) if (s == e) return true;
        return es.empty();
    }

    inline std::optional<std::string> validateInt(long long v, const Validation &val) {
        if (val.minInt && v < *val.minInt) return "int < min";
        if (val.maxInt && v > *val.maxInt) return "int > max";
        if (val.primary == PrimarySemantic::Port && (v < 1 || v > 65535)) return "port out of range";
        return std::nullopt;
    }

    inline std::optional<std::string> validateReal(double v, const Validation &val) {
        if (val.minReal && v < *val.minReal) return "real < min";
        if (val.maxReal && v > *val.maxReal) return "real > max";
        return std::nullopt;
    }

    inline std::optional<std::string> validateString(const std::string &s, const Validation &val) {
        if (val.nonEmpty && s.empty()) return "string empty";
        if (val.regex && !_regexMatch(s, *val.regex)) return "string regex not match";
        if (!val.enumSet.empty() && !_inEnumSet(s, val.enumSet)) return "string not in enum";
        if (val.primary == PrimarySemantic::URLPath) {
            if (s.empty() || s[0] != '/')
                return "url path must start with '/'";
        }
        return std::nullopt;
    }

    inline std::optional<long long> parseDateToMillis(const std::string &text, PrimarySemantic ps) {
        std::tm tm{};
        std::istringstream iss(text);
        if (ps == PrimarySemantic::DateShort) {
            iss >> std::get_time(&tm, "%Y-%m-%d");
            tm.tm_hour = 0;
            tm.tm_min = 0;
            tm.tm_sec = 0;
        } else { iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S"); }
        if (iss.fail()) return std::nullopt;
        auto t = std::mktime(&tm);
        if (t == (std::time_t) -1) return std::nullopt;
        return (long long) t * 1000LL;
    }

    inline std::string formatMillisAsString(long long ms, PrimarySemantic ps) {
        std::time_t t = (std::time_t) (ms / 1000);
        std::tm *tm = std::localtime(&t);
        char buf[32] = {0};
        if (ps == PrimarySemantic::DateShort) std::strftime(buf, sizeof(buf), "%Y-%m-%d", tm);
        else std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", tm);
        return buf;
    }

    // ============= ConfigItem、遍历与注册表 =============
    class ConfigItem {
    public:
        using MT = FieldBase ConfigItem::*;
        using VF = std::vector<MT>;
    private:
        // 保存“成员指针”
        VF _members;

    public:
        template<class Owner, class Member>
        void registerField(Member Owner::* pm) {
            static_assert(std::is_base_of_v<ConfigItem, Owner>);
            _members.push_back(reinterpret_cast<MT>(pm));
        }

        // 统一遍历 【[&](const cfg::FieldBase* f){}】
        template<class F>
        void forEachField(F &&f) const {
            for (auto m: _members) {
                auto *fb = &(this->*m);   // 用当前 this 取真实地址
                f(fb);
            }
        }

        VF::size_type fieldCount() const { return _members.size(); }

        FieldBase *fieldAt(VF::size_type index) {
            auto m = _members[index];
            auto *fb = &(this->*m);
            return fb;
        }

        const FieldBase *fieldAt(VF::size_type index) const {
            auto m = _members[index];
            auto *fb = &(this->*m);
            return fb;
        }

        virtual const cfg::ConfigMeta &meta() const = 0;

    };

    template<class Fn>
    inline void walkItems(const cfg::ConfigItem *root, Fn &&fn) {
        fn(root);
        root->forEachField([&](const cfg::FieldBase *f) {
            using VK = cfg::ValueKind;
            switch (f->kind) {
                case VK::kObject: {
                    auto const *fo = dynamic_cast<cfg::FieldObjectBase const *>(f);
                    if (auto const *child = fo->getItem())
                        walkItems(child, std::forward<Fn>(fn));
                    break;
                }
                case VK::kObjectArray: {
                    auto const *fao = dynamic_cast<cfg::FieldArrayObjectBase const *>(f);
                    const std::size_t n = fao->size();
                    for (std::size_t i = 0; i < n; ++i) {
                        if (auto const *child = fao->at(i))
                            walkItems(child, std::forward<Fn>(fn));
                    }
                    break;
                }
                default:
                    break;
            }
        });
    }

    // 辅助：FieldBase 转字符串
    inline std::string fieldValueToString(const FieldBase *f) {
        switch (f->kind) {
            case ValueKind::kBool: {
                auto *b = (const FieldValue<bool> *) f;
                return b->value ? "true" : "false";
            }
            case ValueKind::kInt: {
                auto *x = (const FieldValue<int> *) f;
                return std::to_string(x->value);
            }
            case ValueKind::kInt64: {
                auto *ll = (const FieldValue<long long> *) f;
                return std::to_string(ll->value);
            }
            case ValueKind::kDouble: {
                auto *d = (const FieldValue<double> *) f;
                std::ostringstream oss;
                oss << d->value;
                return oss.str();
            }
            case ValueKind::kString: {
                auto *s = (const FieldValue<std::string> *) f;
                return s->value;
            }
            case ValueKind::kIntArray: {
                auto *a = (const FieldArray<int> *) f;
                std::ostringstream oss;
                for (size_t i = 0; i < a->value.size(); ++i) {
                    if (i) oss << ",";
                    oss << a->value[i];
                }
                return oss.str();
            }
            case ValueKind::kInt64Array: {
                auto *a = (const FieldArray<long long> *) f;
                std::ostringstream oss;
                for (size_t i = 0; i < a->value.size(); ++i) {
                    if (i) oss << ",";
                    oss << a->value[i];
                }
                return oss.str();
            }
            case ValueKind::kDoubleArray: {
                auto *a = (const FieldArray<double> *) f;
                std::ostringstream oss;
                for (size_t i = 0; i < a->value.size(); ++i) {
                    if (i) oss << ",";
                    oss << a->value[i];
                }
                return oss.str();
            }
            case ValueKind::kStringArray: {
                auto *a = (const FieldArray<std::string> *) f;
                std::ostringstream oss;
                for (size_t i = 0; i < a->value.size(); ++i) {
                    if (i) oss << ",";
                    oss << a->value[i];
                }
                return oss.str();
            }
            default:
                return "<unknown>";
        }
    }

    // 收集 path-key-value
    inline void collectAllConfigs(const ConfigItem *root,
                                  const std::string &prefix,
                                  std::vector<std::tuple<std::string, std::string, std::string>> &out) {
        root->forEachField([&](const FieldBase *f) {
            std::string fullPath = prefix.empty() ? f->key : prefix + "." + f->key;

            if (f->kind == ValueKind::kObject) {
                auto *fo = dynamic_cast<const FieldObjectBase *>(f);
                if (fo && fo->getItem())
                    collectAllConfigs(fo->getItem(), fullPath, out);
            } else if (f->kind == ValueKind::kObjectArray) {
                auto *fao = dynamic_cast<const FieldArrayObjectBase *>(f);
                if (fao) {
                    for (size_t i = 0; i < fao->size(); ++i) {
                        collectAllConfigs(fao->at(i), fullPath + "[" + std::to_string(i) + "]", out);
                    }
                }
            } else {
                out.emplace_back(prefix, f->key, fieldValueToString(f));
            }
        });
    }

    // 打印所有配置
    inline void printAllConfigs(const ConfigItem *root) {
        std::vector<std::tuple<std::string, std::string, std::string>> items;
        collectAllConfigs(root, "", items);
        for (auto &[path, key, value]: items) {
            std::cout << (path.empty() ? key : path + "." + key)
                      << " = " << value << std::endl;
        }
    }

    // ============= IO 抽象 + 管理器（集中流程） =============
    struct ConfigIOBase {
        virtual ~ConfigIOBase() = default;

        virtual bool loadFromFile(ConfigItem *, const std::string &, std::string *err) = 0;

        virtual bool saveToFile(const ConfigItem *, const std::string &, std::string *err) = 0;
    };

    class ConfigManager {
    public:
        explicit ConfigManager(ConfigIOBase *drv) {
            _driver.reset(drv);
        }

        const ConfigIOBase *driver() const { return _driver.get(); }

        template<typename RootT>
        bool loadAll(const std::string &path, RootT *root, std::string *err) {
            static_assert(std::is_base_of<ConfigItem, RootT>::value, "root must derive from ConfigItem");
            if (!_driver) {
                if (err) *err = "no driver";
                return false;
            }
            std::string e;
            if (!_driver->loadFromFile(root, path, &e)) {
                if (err) *err = e;
                return false;
            }
            return true;
        }

        template<typename RootT>
        bool save(const std::string &path, const RootT *root, std::string *err) {
            static_assert(std::is_base_of<ConfigItem, RootT>::value, "root must derive from ConfigItem");
            if (!_driver) {
                if (err) *err = "no driver";
                return false;
            }
            return _driver->saveToFile(root, path, err);
        }

    private:
        std::unique_ptr<ConfigIOBase> _driver;

    };

} // namespace cfg

#include <yaml-cpp/yaml.h>
#include <fstream>

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

// -----------------------------
// FILE: include/example_app_config.hpp
// -----------------------------

struct RotationPolicy : public cfg::ConfigItem {
    cfg::FieldValue<int> maxSizeMB{"maxSizeMB", 100, false, "单文件最大(MB)", "1~4096",
                                   cfg::Validation::IntRange(1, 4096)};

    cfg::FieldValue<int> keepFiles{"keepFiles", 7, false, "最多保留", "1~1000",
                                   cfg::Validation::IntRange(1, 1000)};

    cfg::FieldValue<bool> dailyRotate{"dailyRotate", true, false, "是否按天", "true/false",
                                      cfg::Validation{}};

    const cfg::ConfigMeta &meta() const override {
        static const cfg::ConfigMeta m{"RotationPolicy", "日志滚动策略", "控制单文件大小、保留数量与按天滚动"};
        return m;
    }

    RotationPolicy() {
        registerField(&RotationPolicy::maxSizeMB);
        registerField(&RotationPolicy::keepFiles);
        registerField(&RotationPolicy::dailyRotate);
    }
};

struct SinkConfig : public cfg::ConfigItem {
    cfg::FieldValue<std::string> id{"id", "main", false, "Sink标识", "作为 Key（可用于引用）",
                                    cfg::Validation::StringRegex("^[A-Za-z0-9_\\-]{1,64}$")};

    cfg::FieldValue<std::string> type{"type", "file", false, "输出类型", "file/console/tcp",
                                      cfg::Validation::EnumSet({"file", "console", "tcp"})};

    cfg::FieldValue<std::string> level{"level", "info", false, "最低日志级别",
                                       "trace/debug/info/warn/error/critical/off",
                                       cfg::Validation::EnumSet(
                                               {"trace", "debug", "info", "warn", "error", "critical", "off"})};

    cfg::FieldObject<RotationPolicy> rotation{"rotation", false, "文件滚动策略", "type=file 时有效",
                                              cfg::Validation{}};

    cfg::FieldArray<std::string> tags{"tags", {"main"}, false, "标签", "非空且唯一",
                                      cfg::Validation::Array(true, true)};

    const cfg::ConfigMeta &meta() const override {
        static const cfg::ConfigMeta m{"SinkConfig", "日志输出(Sink)", "支持 file/console/tcp；file 可配置滚动策略"};
        return m;
    }

    SinkConfig() {
        registerField(&SinkConfig::id);
        registerField(&SinkConfig::type);
        registerField(&SinkConfig::level);
        registerField(&SinkConfig::rotation);
        registerField(&SinkConfig::tags);
    }
};

struct LoggerConfig : public cfg::ConfigItem {
    cfg::FieldValue<std::string> name{"name", "app", false, "日志器名称", "Key",
                                      cfg::Validation::StringRegex("^[A-Za-z0-9_\\-]{1,64}$")};

    cfg::FieldValue<std::string> format{"format", "%Y-%m-%dT%H:%M:%S [%l] %v", false, "格式模板", "",
                                        cfg::Validation::StringRegex(".+")};

    cfg::FieldValue<int> timeLong{"timeLong", 11, false, "时长", "",
                                  cfg::Validation::IntRange(0, 100)};

    cfg::FieldValue<int> timeCost{"timeCost", 99, false, "耗时", "",
                                  cfg::Validation::IntRange(0, 100)};

    cfg::FieldArrayObject<SinkConfig> sinks{"sinks", true, "输出列表", "至少一个",
                                            cfg::Validation::Array(true)};

    cfg::FieldArray<std::string> modules{"modules", {"core", "net"}, false, "模块过滤", "可为空；若非空需唯一",
                                         cfg::Validation::Array(false, true)};

    const cfg::ConfigMeta &meta() const override {
        static const cfg::ConfigMeta m{"LoggerConfig", "日志器(Logger)", "日志器包含若干输出(sinks)与模块过滤"};
        return m;
    }

    LoggerConfig() {
        registerField(&LoggerConfig::name);
        registerField(&LoggerConfig::format);
        registerField(&LoggerConfig::timeLong);
        registerField(&LoggerConfig::timeCost);
        registerField(&LoggerConfig::sinks);
        registerField(&LoggerConfig::modules);
    }
};

struct ServiceEndpoint : public cfg::ConfigItem {
    cfg::FieldValue<std::string> host{"host", "127.0.0.1", false, "主机", "主机名或 IP",
                                      cfg::Validation::StringRegex(R"(^.+$)", true, cfg::PrimarySemantic::HostName)};

    cfg::FieldValue<int> port{"port", 8080, true, "端口", "1~65535",
                              cfg::Validation::IntRange(1, 65535, cfg::PrimarySemantic::Port)};

    cfg::FieldArray<std::string> paths{"paths", {"/api", "/health"}, false, "路径白名单", "每项形如 /xxx; 非空且唯一",
                                       [] {
                                           auto v = cfg::Validation::Array(true, true, cfg::PrimarySemantic::URLPath);
                                           v.regex = "^/.*$";
                                           return v;
                                       }()};

    const cfg::ConfigMeta &meta() const override {
        static const cfg::ConfigMeta m{"ServiceEndpoint", "服务端点", "主机/端口/路径白名单"};
        return m;
    }

    ServiceEndpoint() {
        registerField(&ServiceEndpoint::host);
        registerField(&ServiceEndpoint::port);
        registerField(&ServiceEndpoint::paths);
    }
};

struct AppConfig : public cfg::ConfigItem {
    cfg::FieldValue<bool> enable{"enable", true, false, "是否启用", "true=启用",
                                 cfg::Validation{}};

    cfg::FieldValue<double> timeout{"timeoutSec", 3.5, false, "全局超时(秒)", "0~600",
                                    cfg::Validation::RealRange(0.0, 600.0)};

    cfg::FieldValue<int> version{"configVersion", 6, false, "配置版本", ">=1",
                                 cfg::Validation::IntRange(1, std::numeric_limits<int>::max())};

    cfg::FieldValue<long long> buildDate{"buildDate", 0LL, false, "构建日期", "YYYY-MM-DD",
                                         cfg::Validation{}};
//                                         cfg::Validation{.primary=cfg::PrimarySemantic::DateShort}};

    cfg::FieldObject<ServiceEndpoint> endpoint{"endpoint", false, "服务端点", "主机/端口/路径白名单。",
                                               cfg::Validation{}};

    cfg::FieldArrayObject<LoggerConfig> loggers{"loggers", true, "日志器列表", "至少一个。",
                                                cfg::Validation::Array(true)};

    cfg::FieldArray<std::string> plugins{"plugins", {"metrics", "trace"}, false, "插件名", "非空且唯一",
                                         cfg::Validation::Array(true, true)
    };

    const cfg::ConfigMeta &meta() const override {
        static const cfg::ConfigMeta m{"AppConfig", "应用配置", "应用全局参数与子模块配置入口"};
        return m;
    }

    AppConfig() {
        loggers
                .emplace([](LoggerConfig &l) { l.name = "core"; })
                .emplace([](LoggerConfig &l) { l.name = "ui"; })
                .emplace([](LoggerConfig &l) { l.name = "net"; });

        registerField(&AppConfig::enable);
        registerField(&AppConfig::timeout);
        registerField(&AppConfig::version);
        registerField(&AppConfig::buildDate);
        registerField(&AppConfig::endpoint);
        registerField(&AppConfig::loggers);
        registerField(&AppConfig::plugins);
    }
};

int main() {
    AppConfig conf; // 默认值
    cfg::ConfigManager mgr(new cfg::ConfigIOYAML());

    cfg::printAllConfigs(&conf);

    std::string err;

    // 声明需要解析的 FieldRef<T> 类型集合：<LoggerConfig>
    bool ok = mgr.loadAll<AppConfig>("app.out.yaml", &conf, &err);
    if (!ok) {
        std::cerr << "Load failed:" << err;
        return 1;
    }

    cfg::printAllConfigs(&conf);

    if (!mgr.save("app.out.yaml", &conf, &err)) {
        std::cerr << "Save failed:" << err;
        return 2;
    }


    return 0;
}
