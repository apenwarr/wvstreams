// utils.cpp : Defines the entry point for the DLL application.
//

#include "wvwin32-sanitize.h"

#define EPOCHFILETIME (116444736000000000LL)

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME        ft;
    LARGE_INTEGER   li;
    __int64         t;
    static int      tzflag;

    if (tv)
    {
        GetSystemTimeAsFileTime(&ft);
        li.LowPart  = ft.dwLowDateTime;
        li.HighPart = ft.dwHighDateTime;
        t  = li.QuadPart;       /* In 100-nanosecond intervals */
	t -= EPOCHFILETIME;     /* Offset to the Epoch time */
        t /= 10;                /* In microseconds */
        tv->tv_sec  = (long)(t / 1000000);
        tv->tv_usec = (long)(t % 1000000);
    }

#if 0
    if (tz)
    {
        if (!tzflag)
        {
            _tzset();
            tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }
#endif
    return 0;
}


pid_t getpid()
{
    return GetCurrentThreadId();
}


unsigned int sleep(unsigned int secs)
{
    Sleep(secs*1000);
    return 0;
}


// FIXME: this makes alarms silently fail.  They should probably fail more
// nicely, or (better still) actually work...
unsigned int alarm(unsigned int t)
{
    return 0;
}


// This is the same as what Python uses, apparently
int fsync(int fd)
{
    return _commit(fd);
}
