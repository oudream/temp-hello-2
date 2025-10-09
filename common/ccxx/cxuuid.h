#ifndef CXUUID_H
#define CXUUID_H


/**
 * Attention ...
 * Attention ...
 * Attention ...
 * CxUuid Is Dependent 3rd Library
 * CxUuid Is Dependent 3rd Library
 * CxUuid Is Dependent 3rd Library
 *
### mac
        self support

### Ubuntu
        sudo apt-get install uuid-dev

### CentOS
        sudo yum install libuuid-devel
 */


#include "cxglobal.h"

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct GUID {
  unsigned int Data1;
  unsigned short Data2;
  unsigned short Data3;
  unsigned char Data4[8];
} GUID;
#endif

class CCXX_EXPORT CxUuid
{
public:
    static std::string createGuidString();

    static std::string guidToString(const GUID &guid);

    static GUID guidFromString(const std::string &sGuidString);

#ifdef __ANDROID__
    static GUID createGuid(JNIEnv *env);
#else

    static GUID createGuid();

#endif
};

#endif // CXUUID_H
