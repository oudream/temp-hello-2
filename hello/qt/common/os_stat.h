#ifndef OS_STAT_H
#define OS_STAT_H

#include <ccxx/cxglobal.h>

class OSStat
{
public:
    static void readCPU(msepoch_t dtNow);

    /**
     * getStat
     * @param c : CPU %59 return 59
     * @param m_t : MemTotal:        8003672 kB
     * @param m_f : MemFree:          182756 kB
     * @param m_a : MemAvailable:    6129696 kB
     * @param d : DISK %59 return 59
     * @return
     */
    static bool getStat(int *c, int *m_t, int *m_f, int *m_a, int *d);

};


#endif //OS_STAT_H

