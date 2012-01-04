/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file timeofday.c
 * @author Unknown - result of the internet search.
 * @date 04-Jan-2012
 * @brief Windows implementation of POSIX gettimeofday() routine.
 * @details It is not a big deal - Windows returns time in multiples of 100ns,
 * and the whole gettimeofday() comes down to calling GetSystemTimeAsFileTime()
 * and then doing some math.
 */

#include "pcc.h"
#include "timeofday.h" //I've ommited this line.
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    FILETIME ft;
    unsigned __int64 tmpres = 0;
    static int tzflag;

    if (NULL != tv)
    {
        GetSystemTimeAsFileTime(&ft);

        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;

        /*converting file time to unix epoch*/
        tmpres /= 10;  /*convert into microseconds*/
        tmpres -= DELTA_EPOCH_IN_MICROSECS; 
        tv->tv_sec = (long)(tmpres / 1000000UL);
        tv->tv_usec = (long)(tmpres % 1000000UL);
    }

    if (NULL != tz)
    {
        if (!tzflag)
        {
            _tzset();
            tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }

    return 0;
}

