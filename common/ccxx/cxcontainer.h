#ifndef CXCONTAINER_H
#define CXCONTAINER_H


#include "cxglobal.h"
#include "cxstring.h"

class CxContainer
{
public:
    // ======================= vector ================================
    //vector - append
    template<typename TValue>
    inline static void append(std::vector<TValue> &vector1, const std::vector<TValue> &vector2)
    {
        vector1.insert(vector1.end(), vector2.begin(), vector2.end());
    }

    //vector - update
    template<typename TValue>
    inline static void update(std::vector<TValue> &vector1, const TValue &value1)
    {
        if (std::find(vector1.begin(), vector1.end(), value1) == vector1.end())
        {
            vector1.push_back(value1);
        }
    }

    template<typename TValue>
    inline static bool update(std::vector<TValue> &vector1, const TValue &value1, const TValue &value2)
    {
        typename std::vector<TValue>::iterator it = std::find(vector1.begin(), vector1.end(), value1);
        if (it != vector1.end())
        {
            *it = value2;
            return true;
        }
        return false;
    }

    //vector - contain
    template<typename TValue>
    inline static bool contain(std::vector<TValue> &vector1, const TValue &value1)
    {
        return std::find(vector1.begin(), vector1.end(), value1) != vector1.end();
    }

    template<typename TValue>
    inline static bool contain(const std::vector<TValue> &vector1, const TValue &value1)
    {
        return std::find(vector1.begin(), vector1.end(), value1) != vector1.end();
    }

    template<typename TValue>
    inline static bool contain(const std::vector<TValue> *oVector1, const TValue &value1)
    {
        return std::find(oVector1->begin(), oVector1->end(), value1) != oVector1->end();
    }

    template<typename TValue>
    inline static bool contain(std::vector<TValue> *oVector1, const TValue &value1)
    {
        return std::find(oVector1->begin(), oVector1->end(), value1) != oVector1->end();
    }

    template<typename TKey, typename TValue>
    inline static bool contain(const std::vector<std::pair<TKey, TValue> > &vec, const TKey &key)
    {
        for (int i = 0; i < vec.size(); ++i)
        {
            if (vec[i]->first == key)
            {
                return true;
            }
        }
        return false;
    }

    template<typename TKey, typename TValue>
    inline static typename std::vector<std::pair<TKey, TValue> >::const_iterator contain(const std::vector<std::pair<TKey, TValue> > &vec, const TKey &key)
    {
        for (int i = 0; i < vec.size(); ++i)
        {
            if (vec[i]->first == key)
            {
                return vec.begin() + i;
            }
        }
        return vec.end();
    }


    //vector - remove
    template<typename TValue>
    inline static size_t remove(std::vector<TValue> &vector1, const std::vector<TValue> &vector2)
    {
        size_t r = vector1.size();
        for (typename std::vector<TValue>::iterator it = vector1.begin(); it != vector1.end();)
        {
            if (std::find(vector2.begin(), vector2.end(), *it) != vector2.end())
            {
                it = vector1.erase(it);
            }
            else
            {
                ++it;
            }
        }
        return r - vector1.size();
    }

    template<typename TValue>
    inline static size_t remove2(std::vector<TValue> &vector1, const std::vector<TValue> &vector2)
    {
        size_t r = vector1.size();
        std::vector<TValue> rv(r);
        size_t k = 0;
        for (typename std::vector<TValue>::iterator it = vector1.begin(); it != vector1.end();)
        {
            if (std::find(vector2.begin(), vector2.end(), *it) == vector2.end())
            {
                rv[k++] = *it;
            }
            ++it;
        }
        rv.resize(k);
        vector1 = rv;
        return r - k;
    }

    template<typename TValue>
    inline static void remove(std::vector<TValue> &vector1, const TValue &value)
    {
        vector1.erase(std::remove(vector1.begin(), vector1.end(), value), vector1.end());
    }

    //vector - remove
    template<typename TKey, typename TValue>
    inline static size_t remove(const std::vector<std::pair<TKey, TValue> > &vec, const TKey &key)
    {
        size_t r = vec.size();
        for (int i = vec.size() - 1; i >= 0; --i)
        {
            if (vec[i]->first == key)
            {
                vec.erase(vec.begin() + i);
            }
        }
        return r - vec.size();
    }

    template<typename TKey, typename TValue>
    inline static size_t removeByValue(const std::vector<std::pair<TKey, TValue> > &vec, const TValue &value)
    {
        size_t r = vec.size();
        for (int i = vec.size() - 1; i >= 0; --i)
        {
            if (vec[i]->second == value)
            {
                vec.erase(vec.begin() + i);
            }
        }
        return r - vec.size();
    }

    //vector - delete clear
    template<typename TValue>
    inline static void deleteAndClear(std::vector<TValue> &vector1)
    {
        for (typename std::vector<TValue>::iterator it = vector1.begin(); it != vector1.end(); ++it)
        {
            TValue d = *it;
            delete d;
        }
        vector1.clear();
    }

    // ---------- std::vector<T*> std::list<T*> ----------
    template<class PtrContainer>
    inline void deleteAll(PtrContainer &c)
    {
        for (auto *p: c) delete p;
        c.clear();
    }

    // ---------- std::vector<T*>* std::list<T*>* ----------
    template<class PtrContainerPtr>
    inline void deleteAll(PtrContainerPtr cptr)
    {
        if (!cptr) return;
        for (auto *p: *cptr) delete p;
        cptr->clear();
    }

    // CxContainer::findByMember(v, &MyClass::id, 123)
    template<class PtrContainer, class T, class M>
    inline T *findByMember(const PtrContainer &c, M T::*member, const M &value)
    {
        for (auto *p: c)
        {
            if (p && (p->*member) == value) return p;
        }
        return nullptr;
    }

    // CxContainer::findByMember(&v, &MyClass::id, 123)
    template<class PtrContainerPtr, class T, class M>
    inline T *findByMember(PtrContainerPtr cptr, M T::*member, const M &value)
    {
        if (!cptr) return nullptr;
        for (auto *p: *cptr)
        {
            if (p && (p->*member) == value) return p;
        }
        return nullptr;
    }

    // MyClass* r2 = CxContainer::findIfPtr(v, [](const MyClass& o){ return o.code == "ABC"; });
    template<class PtrContainer, class Pred>
    inline typename std::remove_pointer<typename PtrContainer::value_type>::type *findIfPtr(const PtrContainer &c, Pred pred)
    {
        using T = typename std::remove_pointer<typename PtrContainer::value_type>::type;
        for (auto *p: c) if (p && pred(*p)) return p;
        return nullptr;
    }

    // for_each_ptr(v, [](MyClass& o){ o.done(); });
    template<class PtrContainer, class Fn>
    inline void forEachPtr(PtrContainer &c, Fn fn)
    {
        for (auto *p: c) if (p) fn(*p);
    }

    // const PtrContainer &c
    template<class PtrContainer, class Fn>
    inline void forEachPtr(const PtrContainer &c, Fn fn)
    {
        for (auto *p: c) if (p) fn(*p);
    }

    //vector - pointer begin end
    template<typename TValue>
    inline static const TValue *beginPointer(const std::vector<TValue> &vector1)
    {
        return (!vector1.empty()) ? (&vector1.front()) : (nullptr);
    }

    template<typename TValue>
    inline static TValue *beginPointer(std::vector<TValue> &vector1)
    {
        return (!vector1.empty()) ? (&vector1.front()) : (nullptr);
    }

    template<typename TValue>
    inline static const TValue *endPointer(const std::vector<TValue> &vector1)
    {
        return (!vector1.empty()) ? (&vector1.back()) : (nullptr);
    }

    template<typename TValue>
    inline static TValue *endPointer(std::vector<TValue> &vector1)
    {
        return (!vector1.empty()) ? (&vector1.back()) : (nullptr);
    }

    //vector - index
    template<typename TValue>
    inline static size_t index(const std::vector<TValue> &vector1, const TValue &value1)
    {
        typename std::vector<TValue>::const_iterator it = std::find(vector1.begin(), vector1.end(), value1);
        if (it != vector1.end())
        {
            return it - vector1.begin();
        }
        return std::string::npos;
    }

    //vector - merge
    template<typename T>
    inline static std::vector<T> merge(const std::vector<T> &a, const std::vector<T> &b)
    {
        std::vector<T> result;
        result.reserve(a.size() + b.size());
        result.insert(result.end(), a.begin(), a.end());
        result.insert(result.end(), b.begin(), b.end());
        return result;
    }

    template<typename T>
    inline static std::vector<T> merge(const std::vector<std::vector<T>> &vs)
    {
        std::vector<T> result;

        size_t total = 0;
        for (const auto &v: vs) total += v.size();
        result.reserve(total);

        for (const auto &v: vs)
        {
            result.insert(result.end(), v.begin(), v.end());
        }

        return result;
    }

    template<typename T>
    inline static std::vector<T> mergeByMove(std::vector<std::vector<T>> &&vs)
    {
        std::vector<T> result;

        size_t total = 0;
        for (const auto &v: vs) total += v.size();
        result.reserve(total);

        for (auto &v: vs)
        {
            result.insert(result.end(), std::make_move_iterator(v.begin()), std::make_move_iterator(v.end()));
        }

        return result;
    }

    //vector - next
    template<typename TValue>
    inline static TValue next(const std::vector<TValue> &vector1, const TValue &value)
    {
        typename std::vector<TValue>::const_iterator it = std::find(vector1.begin(), vector1.end(), value);
        if (it != vector1.end())
        {
            ++it;
            if (it != vector1.end())
            {
                return (*it);
            }
        }
        return CxValueType::originalValue<TValue>();
    }

    template<typename TValue>
    inline static std::string joinToString(const std::vector<TValue> &ss, char cJoin = cx::CHAR_NEWLINE)
    {
        std::string r;
        if (ss.empty())
            return r;
        size_t iSize_1 = ss.size() - 1;
        for (size_t i = 0; i < iSize_1; ++i)
        {
            r.append(CxString::toString(ss.at(i)));
            r.push_back(cJoin);
        }
        if (!ss.empty())
        {
            r.append(CxString::toString(ss.at(iSize_1)));
        }
        return r;
    }

    template<typename TValue>
    inline static std::string joinToString(const std::vector<TValue> &ss, const std::string &sJoin)
    {
        std::string r;
        if (ss.empty())
            return r;
        size_t iSize_1 = ss.size() - 1;
        for (size_t i = 0; i < iSize_1; ++i)
        {
            r.append(CxString::toString(ss.at(i)));
            r.append(sJoin);
        }
        if (!ss.empty())
        {
            r.append(CxString::toString(ss.at(iSize_1)));
        }
        return r;
    }

    template<typename TValue>
    inline static std::string joinToHexString(const std::vector<TValue> &ss, char cJoin = cx::CHAR_NEWLINE)
    {
        std::string r;
        if (ss.empty())
            return r;
        size_t iSize_1 = ss.size() - 1;
        for (size_t i = 0; i < iSize_1; ++i)
        {
            r.append(CxString::toHexString(ss.at(i)));
            r.push_back(cJoin);
        }
        if (!ss.empty())
        {
            r.append(CxString::toHexString(ss.at(iSize_1)));
        }
        return r;
    }

    template<typename TValue>
    inline static std::string joinToHexString(const std::vector<TValue> &ss, const std::string &sJoin)
    {
        std::string r;
        if (ss.empty())
            return r;
        size_t iSize_1 = ss.size() - 1;
        for (size_t i = 0; i < iSize_1; ++i)
        {
            r.append(CxString::toHexString(ss.at(i)));
            r.append(sJoin);
        }
        if (!ss.empty())
        {
            r.append(CxString::toHexString(ss.at(iSize_1)));
        }
        return r;
    }

    //vector - value
    template<typename TValue, typename TNewValue>
    inline static std::vector<TNewValue> valuesTo(const std::vector<TValue> &vec1, const TNewValue &defaultValue)
    {
        std::vector<TNewValue> r;
        for (size_t i = 0; i < vec1.size(); ++i)
        {
            r.push_back(CxValueType::valueTo(vec1.at(i), defaultValue));
        }
        return r;
    }

    template<typename TValue>
    inline static void sort(std::vector<TValue> &vec1, bool bOrder = true)
    {
        if (bOrder)
            std::sort(vec1.begin(), vec1.end(), CxContainer::compareOrder<TValue>);
        else
            std::sort(vec1.begin(), vec1.end(), CxContainer::compareReverse<TValue>);
    }

    template<typename TValue>
    inline static int compareOrder(const TValue &v1, const TValue &v2)
    {
        return v2 > v1;
    }

    template<typename TValue>
    inline static int compareReverse(const TValue &v1, const TValue &v2)
    {
        return v2 < v1;
    }

    template<typename TKey, typename TValue>
    inline static typename std::vector<std::pair<TKey, TValue> >::const_iterator find(const std::vector<std::pair<TKey, TValue> > &vec, const TKey &key)
    {
        for (int i = 0; i < vec.size(); ++i)
        {
            if (vec[i]->first == key)
            {
                return vec.begin() + i;
            }
        }
        return vec.end();
    }

    // ======================= map ================================
    template<typename TMap>
    using CxMapKey_t   = typename TMap::key_type;

    template<typename TMap>
    using CxMapValue_t = typename TMap::mapped_type;

    template<typename TMap>
    using CxMapPair_t  = typename TMap::value_type;

    // ---------- keys ----------
    template<typename TMap>
    inline static std::vector<CxMapKey_t<TMap>> keys(const TMap &map1)
    {
        std::vector<CxMapKey_t<TMap>> r;
        r.reserve(map1.size());
        for (const auto &kv : map1) r.push_back(kv.first);
        return r;
    }

    template<typename TMap>
    inline static std::vector<CxMapKey_t<TMap>> keys(const TMap &map1, const CxMapValue_t<TMap> &value1)
    {
        std::vector<CxMapKey_t<TMap>> r;
        for (const auto &kv : map1) if (kv.second == value1) r.push_back(kv.first);
        return r;
    }

// ---------- contain (by key) ----------
    template<typename TMap>
    inline static bool contain(TMap &map1, const CxMapKey_t<TMap> &key1)
    {
        return map1.find(key1) != map1.end();
    }

    template<typename TMap>
    inline static bool contain(const TMap &map1, const CxMapKey_t<TMap> &key1)
    {
        return map1.find(key1) != map1.end();
    }

    template<typename TMap>
    inline static bool contain(const TMap *map1, const CxMapKey_t<TMap> &key1)
    {
        if (!map1) return false;
        return map1->find(key1) != map1->end();
    }

// ---------- containValue ----------
    template<typename TMap>
    inline static bool containValue(const TMap *map1, const CxMapValue_t<TMap> &value1)
    {
        if (!map1) return false;
        for (const auto &kv : *map1) if (kv.second == value1) return true;
        return false;
    }

    template<typename TMap>
    inline static bool containValue(const TMap &map1, const CxMapValue_t<TMap> &value1)
    {
        for (const auto &kv : map1) if (kv.second == value1) return true;
        return false;
    }

// ---------- key (find key by value) ----------
    template<typename TMap>
    inline static CxMapKey_t<TMap> key(const TMap &map1, const CxMapValue_t<TMap> &value1)
    {
        for (const auto &kv : map1) if (kv.second == value1) return kv.first;
        return CxValueType::originalValue<CxMapKey_t<TMap>>();
    }

    template<typename TMap>
    inline static const CxMapKey_t<TMap> &key(const TMap &map1,
                                              const CxMapValue_t<TMap> &value1,
                                              const CxMapKey_t<TMap> &defaultKey)
    {
        for (const auto &kv : map1) if (kv.second == value1) return kv.first;
        return defaultKey;
    }

// ---------- value (with default) ----------
    template<typename TMap>
    inline static const CxMapValue_t<TMap> &value(const TMap &map1,
                                                  const CxMapKey_t<TMap> &key1,
                                                  const CxMapValue_t<TMap> &defaultValue)
    {
        auto it = map1.find(key1);
        return (it != map1.end()) ? it->second : defaultValue;
    }

    template<typename TMap>
    inline static const CxMapValue_t<TMap> &value(const TMap *map1,
                                                  const CxMapKey_t<TMap> &key1,
                                                  const CxMapValue_t<TMap> &defaultValue)
    {
        if (!map1) return defaultValue;
        auto it = map1->find(key1);
        return (it != map1->end()) ? it->second : defaultValue;
    }

    template<typename TMap>
    inline static CxMapValue_t<TMap> value(const TMap &map1, const CxMapKey_t<TMap> &key1)
    {
        auto it = map1.find(key1);
        return (it != map1.end()) ? it->second : CxValueType::originalValue<CxMapValue_t<TMap>>();
    }

    template<typename TMap>
    inline static CxMapValue_t<TMap> value(const TMap *map1, const CxMapKey_t<TMap> &key1)
    {
        if (!map1) return CxValueType::originalValue<CxMapValue_t<TMap>>();
        auto it = map1->find(key1);
        return (it != map1->end()) ? it->second : CxValueType::originalValue<CxMapValue_t<TMap>>();
    }

// ---------- valuesTo (改变 mapped_type 的版本：分别为 map 与 unordered_map 提供重载) ----------
    template<typename TKey, typename TValue, typename TNewValue>
    inline static std::map<TKey, TNewValue> valuesTo(const std::map<TKey, TValue> &map1, const TNewValue &defaultValue)
    {
        std::map<TKey, TNewValue> r;
        for (const auto &kv : map1) r[kv.first] = CxValueType::valueTo(kv.second, defaultValue);
        return r;
    }

    template<typename TKey, typename TValue, typename TNewValue>
    inline static std::unordered_map<TKey, TNewValue> valuesTo(const std::unordered_map<TKey, TValue> &map1, const TNewValue &defaultValue)
    {
        std::unordered_map<TKey, TNewValue> r;
        r.reserve(map1.size());
        for (const auto &kv : map1) r.emplace(kv.first, CxValueType::valueTo(kv.second, defaultValue));
        return r;
    }

// ---------- valueTo (读出后转型) ----------
    template<typename TMap, typename TNewValue>
    inline static TNewValue valueTo(const TMap &map1, const CxMapKey_t<TMap> &key1, const TNewValue &defaultValue)
    {
        auto it = map1.find(key1);
        if (it != map1.end()) return CxValueType::valueTo(it->second, defaultValue);
        return defaultValue;
    }

// ---------- valueCase（key 为 std::string 时大小写无关） ----------
    template<typename TValue>
    inline static TValue valueCase(const std::map<std::string, TValue> &map1, const std::string &key1)
    {
        for (const auto &kv : map1) if (CxString::equalCase(kv.first, key1)) return kv.second;
        return CxValueType::originalValue<TValue>();
    }

    template<typename TValue>
    inline static TValue valueCase(const std::map<std::string, TValue> &map1, const std::string &key1, const TValue &defaultValue)
    {
        for (const auto &kv : map1) if (CxString::equalCase(kv.first, key1)) return kv.second;
        return defaultValue;
    }

    template<typename TValue>
    inline static TValue valueCase(const std::unordered_map<std::string, TValue> &map1, const std::string &key1)
    {
        for (const auto &kv : map1) if (CxString::equalCase(kv.first, key1)) return kv.second;
        return CxValueType::originalValue<TValue>();
    }

    template<typename TValue>
    inline static TValue valueCase(const std::unordered_map<std::string, TValue> &map1, const std::string &key1, const TValue &defaultValue)
    {
        for (const auto &kv : map1) if (CxString::equalCase(kv.first, key1)) return kv.second;
        return defaultValue;
    }

// ---------- merge / update ----------
    template<typename TMap>
    inline static TMap merge(const TMap &map1, const TMap &map2)
    {
        TMap r = map1;
        r.insert(map2.begin(), map2.end());
        return r;
    }

    template<typename TMap>
    inline static void update(TMap &map1, const TMap &map2)
    {
        for (const auto &kv : map2) map1[kv.first] = kv.second;
    }

// ---------- remove (by key / keys) ----------
    template<typename TMap>
    inline static void remove(TMap &map1, const CxMapKey_t<TMap> &key1)
    {
        auto it = map1.find(key1);
        if (it != map1.end()) map1.erase(it);
    }

    template<typename TMap>
    inline static void remove(TMap &map1, const std::vector<CxMapKey_t<TMap>> &keysVec)
    {
        for (const auto &k : keysVec)
        {
            auto it = map1.find(k);
            if (it != map1.end()) map1.erase(it);
        }
    }

// ---------- joinToString ----------
    template<typename TMap>
    inline static std::string joinToString(const TMap &pairs, const std::string &sMid, const std::string &sJoin)
    {
        std::string r;
        for (const auto &kv : pairs)
        {
            r.append(CxString::toString(kv.first));
            r.append(sMid);
            r.append(CxString::toString(kv.second));
            r.append(sJoin);
        }
        if (!r.empty()) r.resize(r.size() - sJoin.size());
        return r;
    }

    template<typename TKey, typename TValue>
    inline static std::string
    joinToString(const std::map<TKey, std::vector<TValue>> &pairs, const std::string &sMid, const std::string &sJoin1, const std::string &sJoin2)
    {
        std::string r;
        for (const auto &kv : pairs)
        {
            r.append(CxString::toString(kv.first));
            r.append(sMid);
            r.append(CxContainer::joinToString(kv.second, sJoin1));
            r.append(sJoin2);
        }
        if (!r.empty()) r.resize(r.size() - sJoin2.size());
        return r;
    }

    template<typename TKey, typename TValue>
    inline static std::string
    joinToString(const std::unordered_map<TKey, std::vector<TValue>> &pairs, const std::string &sMid, const std::string &sJoin1, const std::string &sJoin2)
    {
        std::string r;
        for (const auto &kv : pairs)
        {
            r.append(CxString::toString(kv.first));
            r.append(sMid);
            r.append(CxContainer::joinToString(kv.second, sJoin1));
            r.append(sJoin2);
        }
        if (!r.empty()) r.resize(r.size() - sJoin2.size());
        return r;
    }

// ---------- joinToHexString ----------
// iMode : 1 key to hex ; 2 value to hex ; 3 key & value to hex
    template<typename TMap>
    inline static std::string joinToHexString(const TMap &pairs, const std::string &sMid, const std::string &sJoin, int iMode = 3)
    {
        std::string r;
        for (const auto &kv : pairs)
        {
            if (iMode == 1)
            {
                r.append(CxString::toHexString(kv.first));
                r.append(sMid);
                r.append(CxString::toString(kv.second));
            }
            else if (iMode == 2)
            {
                r.append(CxString::toString(kv.first));
                r.append(sMid);
                r.append(CxString::toHexString(kv.second));
            }
            else
            {
                r.append(CxString::toHexString(kv.first));
                r.append(sMid);
                r.append(CxString::toHexString(kv.second));
            }
            r.append(sJoin);
        }
        if (!r.empty()) r.resize(r.size() - sJoin.size());
        return r;
    }

    // ======================= queue ================================
    template<typename TValue>
    inline static void clear(std::queue<TValue> &ss)
    {
        while (!ss.empty())
        {
            ss.pop();
        }
    }

    // ======================= stack ================================
    template<typename TValue>
    inline static void clear(std::stack<TValue> &ss)
    {
        while (!ss.empty())
        {
            ss.pop();
        }
    }

};

#endif // CXCONTAINER_H
