#ifndef CX_CT_X2_CONFIG_H
#define CX_CT_X2_CONFIG_H

#include "cxglobal.h"

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


#endif //CX_CT_X2_CONFIG_H
