#ifndef CXPROCMUTEX_H
#define CXPROCMUTEX_H

#include "cxstring.h"
#include "cxthread.h"

#if defined(sun) || defined(__APPLE__) || defined(__osf__) || defined(__QNX__) || defined(_AIX)
#include <semaphore.h>
#endif

//multi process scenario
class CCXX_EXPORT CxProcMutex
{
public:
    CxProcMutex(const std::string& name);

    ~CxProcMutex();

    void lock();

    bool tryLock();

    void unlock();

private:
    CxProcMutex();
    CxProcMutex(const CxProcMutex&);
    CxProcMutex& operator = (const CxProcMutex&);

    std::string _name;

#ifdef _WIN32
private:
    HANDLE      _mutex;

#else
private:
    std::string getFileName();

#if defined(sun) || defined(__APPLE__) || defined(__osf__) || defined(__QNX__) || defined(_AIX)
    sem_t* _sem;
#else
    int _lockfd; // lock file descriptor
    int _semfd;  // file used to identify semaphore
    int _semid;  // semaphore id
#endif
#endif

};

class CxProcMutexScope
{
public:
    CxProcMutexScope(CxProcMutex & mutex);

    CxProcMutexScope(CxProcMutex * mutex);

    CxProcMutexScope(CxProcMutex& mutex, long milliseconds);

    CxProcMutexScope(CxProcMutex * mutex, long milliseconds);

    ~CxProcMutexScope();

    CxProcMutexScope() = delete;
    CxProcMutexScope(const CxProcMutexScope&) = delete;
    CxProcMutexScope& operator = (const CxProcMutexScope&) = delete;

private:
    CxProcMutex * _mutex;

};



#endif // CXPROCMUTEX_H
