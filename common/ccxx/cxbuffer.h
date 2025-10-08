#ifndef CXBUFFER_H
#define CXBUFFER_H

#include "cxglobal.h"

class CCXX_EXPORT CxBuffer
{
public:
    ///
    /// \brief setBitTrue
    /// \param data
    /// \param n : index of data ; size_of( * data ) <= 32
    ///
    static void setBitTrue(cx::uchar *data, cx::uchar n);

    static void setBitFalse(cx::uchar *data, cx::uchar n);

    static cx::uchar getBit(const cx::uchar &data, cx::uchar n);

    static void set2Bit(cx::uchar *data, cx::uchar n, cx::uchar value);

    static cx::uchar get2Bit(const cx::uchar &data, cx::uchar n);

    static int divBig(int iQuantity, int iMod);

    static int index(cx::uchar mod, int address, cx::uchar *indexMod = nullptr, int beginAddress = 0);

    static int count(int iQuantity, int iMod);

    static std::vector<cx::uchar> toBuffer(const std::vector<bool> &values);

    static std::vector<cx::uchar> toBuffer(const std::map<int, int> &addressValues, int iMinAddress);

    static std::vector<cx::uchar> toBuffer(const std::map<int, int> &addressValues, int iMinAddress, int iMaxAddress);

    static std::vector<cx::uchar> toBuffer(const std::map<int, double> &addressValues, int iMinAddress);

    template<typename T>
    static bool
    isContinueAddress(const std::map<int, T> &addressValues, int *pMinAddress = nullptr, int *pMaxAddress = nullptr)
    {
        int iMaxAddress = (std::numeric_limits<int>::max)();
        int iMinAddress = (std::numeric_limits<int>::min)();
        for (typename std::map<int, T>::const_iterator it = addressValues.begin(); it != addressValues.end(); ++it)
        {
            int iAddress = it->first;
            if (iAddress > iMaxAddress)
            {
                iMaxAddress = iAddress;
            }
            if (iAddress < iMinAddress)
            {
                iMinAddress = iAddress;
            }
        }
        if (pMinAddress) *pMinAddress = iMinAddress;
        if (pMaxAddress) *pMaxAddress = iMaxAddress;
        return (iMaxAddress - iMinAddress == addressValues.size() - 1);
    }

    static int toBuf(cx::uint8 *pBuf, cx::uint16 val, int mode = 1);

    static int toBuf(cx::uint8 *pBuf, cx::int16 val, int mode = 1);

    static int toBuf(cx::uint8 *pBuf, cx::uint32 val, int mode = 1);

    static int toBuf(cx::uint8 *pBuf, cx::int32 val, int mode = 1);

    static int toBuf(cx::uint8 *pBuf, cx::uint64 val, int mode = 1);

    static int toBuf(cx::uint8 *pBuf, cx::int64 val, int mode = 1);

    static int toBuf(cx::uint8 *pBuf, float val, int mode = 1);

    static int toBuf(cx::uint8 *pBuf, double val, int mode = 1);


    static bool fromBuf(cx::uint8 *pBuf, int len, cx::uint16 &val, int mode = 1);

    static bool fromBuf(cx::uint8 *pBuf, int len, cx::int16 &val, int mode = 1);

    static bool fromBuf(cx::uint8 *pBuf, int len, cx::uint32 &val, int mode = 1);

    static bool fromBuf(cx::uint8 *pBuf, int len, cx::int32 &val, int mode = 1);

    static bool fromBuf(cx::uint8 *pBuf, int len, cx::uint64 &val, int mode = 1);

    static bool fromBuf(cx::uint8 *pBuf, int len, cx::int64 &val, int mode = 1);

    static bool fromBuf(cx::uint8 *pBuf, int len, float &val, int mode = 0);

    static bool fromBuf(cx::uint8 *pBuf, int len, double &val, int mode = 0);

    static int toBuf(cx::uint8 *pBuf, cx::int64 val, std::string fmt, int mode = 0);

    static int toBuf(cx::uint8 *pBuf, double val, std::string fmt, int mode = 0);

    static int toBuf(cx::uint8 *pBuf, std::string val, std::string fmt, int mode = 0);

};

#endif // CXBUFFER_H
