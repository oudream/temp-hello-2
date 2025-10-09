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
    inline static typename std::vector<std::pair<TKey, TValue> >::const_iterator
    contain(const std::vector<std::pair<TKey, TValue> > &vec, const TKey &key)
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

    //vector - pointer begin end
    template<typename TValue>
    inline static const TValue *begin_pointer(const std::vector<TValue> &vector1)
    {
        return (!vector1.empty()) ? (&vector1.front()) : (nullptr);
    }

    template<typename TValue>
    inline static TValue *begin_pointer(std::vector<TValue> &vector1)
    {
        return (!vector1.empty()) ? (&vector1.front()) : (nullptr);
    }

    template<typename TValue>
    inline static const TValue *end_pointer(const std::vector<TValue> &vector1)
    {
        return (!vector1.empty()) ? (&vector1.back()) : (nullptr);
    }

    template<typename TValue>
    inline static TValue *end_pointer(std::vector<TValue> &vector1)
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
    template<typename TValue>
    inline static std::vector<TValue> merge(const std::vector<TValue> &vector1, const std::vector<TValue> &vector2)
    {
        std::vector<TValue> r = vector1;
        vector1.insert(r.end(), vector2.begin(), vector2.end());
        return r;
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
            std::sort(vec1.begin(), vec1.end(), CxContainer::compare_order<TValue>);
        else
            std::sort(vec1.begin(), vec1.end(), CxContainer::compare_reverse<TValue>);
    }

    template<typename TValue>
    inline static int compare_order(const TValue &v1, const TValue &v2)
    {
        return v2 > v1;
    }

    template<typename TValue>
    inline static int compare_reverse(const TValue &v1, const TValue &v2)
    {
        return v2 < v1;
    }

    template<typename TKey, typename TValue>
    inline static typename std::vector<std::pair<TKey, TValue> >::const_iterator
    find(const std::vector<std::pair<TKey, TValue> > &vec, const TKey &key)
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
    //map - keys
    template<typename TKey, typename TValue>
    inline static std::vector<TKey> keys(const std::map<TKey, TValue> &map1)
    {
        std::vector<TKey> r;
        for (typename std::map<TKey, TValue>::const_iterator it = map1.begin(); it != map1.end(); ++it)
        {
            r.push_back(it->first);
        }
        return r;
    }

    template<typename TKey, typename TValue>
    inline static std::vector<TKey> keys(const std::map<TKey, TValue> &map1, const TValue &value1)
    {
        std::vector<TKey> r;
        for (typename std::map<TKey, TValue>::const_iterator it = map1.begin(); it != map1.end(); ++it)
        {
            if (it->second == value1)
                r.push_back(it->first);
        }
        return r;
    }

    //map - contain
    template<typename TKey, typename TValue>
    inline static bool contain(std::map<TKey, TValue> &map1, const TKey &key1)
    {
        typename std::map<TKey, TValue>::const_iterator it = map1.find(key1);
        if (it != map1.end())
            return true;
        else
            return false;
    }

    template<typename TKey, typename TValue>
    inline static bool contain(const std::map<TKey, TValue> &map1, const TKey &key1)
    {
        typename std::map<TKey, TValue>::const_iterator it = map1.find(key1);
        if (it != map1.end())
            return true;
        else
            return false;
    }

    template<typename TKey, typename TValue>
    inline static bool contain(const std::map<TKey, TValue> *map1, const TKey &key1)
    {
        typename std::map<TKey, TValue>::const_iterator it = map1->find(key1);
        if (it != map1->end())
            return true;
        else
            return false;
    }

    template<typename TKey, typename TValue>
    inline static bool containValue(const std::map<TKey, TValue> *map1, const TValue &value1)
    {
        for (typename std::map<TKey, TValue>::const_iterator it = map1->begin(); it != map1->end(); ++it)
        {
            if (it->second == value1)
                return true;
        }
        return false;
    }

    template<typename TKey, typename TValue>
    inline static bool containValue(const std::map<TKey, TValue> &map1, const TValue &value1)
    {
        for (typename std::map<TKey, TValue>::const_iterator it = map1.begin(); it != map1.end(); ++it)
        {
            if (it->second == value1)
                return true;
        }
        return false;
    }

    //map - key
    template<typename TKey, typename TValue>
    inline static const TKey &key(const std::map<TKey, TValue> &map1, const TValue &value1, const TValue &defaultKey)
    {
        for (typename std::map<TKey, TValue>::const_iterator it = map1.begin(); it != map1.end(); ++it)
        {
            if (it->second == value1)
                return it->first;
        }
        return defaultKey;
    }

    template<typename TKey, typename TValue>
    inline static TKey key(const std::map<TKey, TValue> &map1, const TValue &value1)
    {
        for (typename std::map<TKey, TValue>::const_iterator it = map1.begin(); it != map1.end(); ++it)
        {
            if (it->second == value1)
                return it->first;
        }
        return CxValueType::originalValue<TKey>();
    }

    //map - value
    template<typename TKey, typename TValue>
    inline static const TValue &value(const std::map<TKey, TValue> &map1, const TKey &key1, const TValue &defaultValue)
    {
        typename std::map<TKey, TValue>::const_iterator it = map1.find(key1);
        if (it != map1.end())
            return it->second;
        else
            return defaultValue;
    }

    template<typename TKey, typename TValue>
    inline static const TValue &value(const std::map<TKey, TValue> *map1, const TKey &key1, const TValue &defaultValue)
    {
        if (map1)
        {
            typename std::map<TKey, TValue>::const_iterator it = map1->find(key1);
            if (it != map1->end())
                return it->second;
            else
                return defaultValue;
        }
        else
        {
            return defaultValue;
        }
    }

    template<typename TKey, typename TValue>
    inline static TValue value(const std::map<TKey, TValue> &map1, const TKey &key1)
    {
        typename std::map<TKey, TValue>::const_iterator it = map1.find(key1);
        if (it != map1.end())
            return it->second;
        else
            return CxValueType::originalValue<TValue>();
    }

    template<typename TKey, typename TValue>
    inline static TValue value(const std::map<TKey, TValue> *map1, const TKey &key1)
    {
        if (map1)
        {
            typename std::map<TKey, TValue>::const_iterator it = map1->find(key1);
            if (it != map1->end())
                return it->second;
            else
                return CxValueType::originalValue<TValue>();
        }
        else
        {
            return CxValueType::originalValue<TValue>();
        }
    }

    //map - value
    template<typename TKey, typename TValue, typename TNewValue>
    inline static std::map<TKey, TNewValue> valuesTo(const std::map<TKey, TValue> &map1, const TNewValue &defaultValue)
    {
        std::map<TKey, TNewValue> r;
        for (typename std::map<TKey, TValue>::const_iterator it = map1.begin(); it != map1.end(); ++it)
        {
            TNewValue value = CxValueType::valueTo(it->second, defaultValue);
            r[it->first] = value;
        }
        return r;
    }

    //map - value
    template<typename TKey, typename TValue, typename TNewValue>
    inline static TNewValue valueTo(const std::map<TKey, TValue> &map1, const TKey &key1, const TNewValue &defaultValue)
    {
        typename std::map<TKey, TValue>::const_iterator it = map1.find(key1);
        if (it != map1.end())
        {
            return CxValueType::valueTo(it->second, defaultValue);
        }
        else
        {
            return defaultValue;
        }
    }

    //map - value
    template<typename TValue>
    inline static TValue valueCase(const std::map<std::string, TValue> &map1, const std::string &key1)
    {
        for (typename std::map<std::string, TValue>::const_iterator it = map1.begin(); it != map1.end(); ++it)
        {
            if (equalCase(it->first, key1))
                return it->second;
        }
        return CxValueType::originalValue<TValue>();
    }

    //map - value
    template<typename TValue>
    inline static TValue valueCase(const std::map<std::string, TValue> &map1, const std::string &key1, const TValue &defaultValue)
    {
        for (typename std::map<std::string, TValue>::const_iterator it = map1.begin(); it != map1.end(); ++it)
        {
            if (CxString::equalCase(it->first, key1))
                return it->second;
        }
        return defaultValue;
    }

    //map - merge
    template<typename TKey, typename TValue>
    inline static std::map<TKey, TValue> merge(const std::map<TKey, TValue> &map1, const std::map<TKey, TValue> &map2)
    {
        std::map<TKey, TValue> r = map1;
        r.insert(map2.begin(), map2.end());
        return r;
    }

    //map - update
    template<typename TKey, typename TValue>
    inline static void update(std::map<TKey, TValue> &map1, const std::map<TKey, TValue> &map2)
    {
        for (typename std::map<TKey, TValue>::const_iterator it = map2.begin(); it != map2.end(); ++it)
        {
            map1[it->first] = it->second;
        }
    }

    template<typename TKey, typename TValue>
    inline static void remove(std::map<TKey, TValue> &map1, const TKey &key1)
    {
        typename std::map<TKey, TValue>::iterator it = map1.find(key1);
        if (it != map1.end()) map1.erase(it);
    }

    template<typename TKey, typename TValue>
    inline static void remove(std::map<TKey, TValue> &map1, const std::vector<TKey> &keys)
    {
        for (size_t i = 0; i < keys.size(); ++i)
        {
            const TKey &key1 = keys.at(i);
            typename std::map<TKey, TValue>::iterator it = map1.find(key1);
            if (it != map1.end()) map1.erase(it);
        }
    }

    //joinToString
    template<typename TKey, typename TValue>
    inline static std::string joinToString(const std::map<TKey, TValue> &pairs, const std::string &sMid, const std::string &sJoin)
    {
        std::string r;
        for (typename std::map<TKey, TValue>::const_iterator it = pairs.begin(); it != pairs.end(); ++it)
        {
            r.append(CxString::toString(it->first) + sMid + CxString::toString(it->second) + sJoin);
        }
        if (!r.empty()) r.resize(r.size() - sJoin.size());
        return r;
    }

    template<typename TKey, typename TValue>
    inline static std::string
    joinToString(const std::map<TKey, std::vector<TValue> > &pairs, const std::string &sMid, const std::string &sJoin1, const std::string &sJoin2)
    {
        std::string r;
        for (typename std::map<TKey, std::vector<TValue> >::const_iterator it = pairs.begin(); it != pairs.end(); ++it)
        {
            r.append(CxString::toString(it->first) + sMid + CxContainer::joinToString(it->second, sJoin1) + sJoin2);
        }
        if (!r.empty()) r.resize(r.size() - sJoin2.size());
        return r;
    }

    //joinToString
    //iMode : 1 key to hex ; 2 value to hex ; 3 key value to hex
    template<typename TKey, typename TValue>
    inline static std::string joinToHexString(const std::map<TKey, TValue> &pairs, const std::string &sMid, const std::string &sJoin, int iMode = 3)
    {
        if (iMode == 1)
        {
            std::string r;
            for (typename std::map<TKey, TValue>::const_iterator it = pairs.begin(); it != pairs.end(); ++it)
            {
                r.append(CxString::toHexString(it->first) + sMid + CxString::toString(it->second) + sJoin);
            }
            if (!r.empty()) r.resize(r.size() - sJoin.size());
            return r;
        }
        else if (iMode == 2)
        {
            std::string r;
            for (typename std::map<TKey, TValue>::const_iterator it = pairs.begin(); it != pairs.end(); ++it)
            {
                r.append(CxString::toString(it->first) + sMid + CxString::toHexString(it->second) + sJoin);
            }
            if (!r.empty()) r.resize(r.size() - sJoin.size());
            return r;
        }
        else
        {
            std::string r;
            for (typename std::map<TKey, TValue>::const_iterator it = pairs.begin(); it != pairs.end(); ++it)
            {
                r.append(CxString::toHexString(it->first) + sMid + CxString::toHexString(it->second) + sJoin);
            }
            if (!r.empty()) r.resize(r.size() - sJoin.size());
            return r;
        }
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


#define GM_PLISTO_DELETEALL(oList, ClassName) \
    for (size_t i = 0; i < oList->size(); ++i) \
    { \
        ClassName* o = oList->at(i); \
        delete o; \
    } \
    oList->clear();

#define GM_PLISTO_FIND(oList, ClassName, field, value) \
    for (size_t i = 0; i < oList->size(); ++i) \
    { \
        ClassName* o = oList->at(i); \
        if ( o->field == value ) \
            return o; \
    } \
    return nullptr;

#define GM_LISTO_DONE(list, ClassName, done) \
    for (size_t i = 0; i < list.size(); ++i) \
    { \
        ClassName * o = list.at(i); \
        o->done; \
    }

#define GM_LISTO_FIND(list, ClassName, field, value) \
    for (size_t i = 0; i < list.size(); ++i) \
    { \
        ClassName * o = list.at(i); \
        if ( o->field == value ) \
            return o; \
    } \
    return nullptr;


#endif // CXCONTAINER_H





/***
 *
//init map
const std::map<int, int>::value_type init_value[] =
{
    std::map<int, int>::value_type( 1, 1),
    std::map<int, int>::value_type( 2, 2),
};

const static std::map<int, int> m_mapPn(init_value, init_value+sizeof(init_value)/sizeof(init_value[0]));

*/

