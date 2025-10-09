#ifndef CXCRC_H
#define CXCRC_H

#include "cxglobal.h"

class CCXX_EXPORT CxCrc
{
public:
    static cx::uchar calcSum8(const cx::uchar *pData, int iLength);

    static cx::ushort calcSum16(const cx::uchar *pData, int iLength);

    // check table style
    static cx::uchar calcCRC8(const cx::uchar *pData, int iLength);

    // check table style
    static cx::ushort calcCRC16(const cx::uchar *pData, int iLength);

    static short calcCRC16(const char *pData, int iLength);

    //check table style
    static cx::ushort calcModbus(const cx::uchar *pData, int iLength);

    static std::string md5HexCode(const std::string &sData);

    static std::string md5HexCode(const std::vector<std::pair<const char *, int>> &sLines);

    static std::vector<char> md5(const std::vector<std::pair<const char *, int>> &sLines);

    static std::vector<char> md5(const char *pData, int iLength);

    /**
     * @brief md5
     * @param sFilePath
     * @param iCodeType : 0=data ; 1=hex
     * @return : empty=file is empty or error
     */
    static std::string file2md5(const std::string &sFilePath, int iCodeType = 1);

    static bool isSameMd5FileData(const std::string &sFilePath1, const std::string &sFilePath2);

    static cx::uchar calcXOR(const cx::uchar *pData, int iLength);

    // formula style
    static cx::uchar crc8(const cx::uchar *ptr, int len);

    static unsigned int calculate_crc16_2(unsigned char *ptr, unsigned char len);

    // simple encrypt
    static std::string encodeXorFix(const char *pData, int iDataLength, const char *pKey, int iKeyLength);

    static std::string decodeXorFix(const char *pData, int iDataLength, const char *pKey, int iKeyLength);

    static std::string encodeXorFix(const char *pData, int iLength);

    static std::string decodeXorFix(const char *pData, int iLength);

};

#endif // CXCRC_H
