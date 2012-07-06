// Original source template was written by shimitei
// http://www.asahi-net.or.jp/~kh4s-smz/spi/make_spi.html
// 
// Normally, you don't need to look at this file.

#ifndef critsect_h
#define critsect_h

#include <windows.h>

class CriticalSection
{
public:
    CriticalSection()
    {
        InitializeCriticalSection(&crit_sect);
    };
    ~CriticalSection()
    {
        DeleteCriticalSection(&crit_sect);
    };
    void Enter()
    {
        EnterCriticalSection(&crit_sect);
    };
    void Leave()
    {
        LeaveCriticalSection(&crit_sect);
    };
private:
    CRITICAL_SECTION crit_sect;
};

#endif
