/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file timeofday.h
 * @author T. Ostaszewski
 * @date 04-Jan-2012
 * @brief Windows wrappers for POSIX time routines.
 * @details It looks like the mighty Windows lacks the gettimeofday() function.
 * A little bit of internet search fills that gap. The result is that very file with
 * forward declarations. See the actual implementation.
 * @sa timeofday.c
 */

#if !defined TIME_OF_DAY_H
#define TIME_OF_DAY_H
/*! 
 * @brief Windows counterpart of POSIX timezone structure.
 */
struct timezone 
{
  int  tz_minuteswest; /*!< minutes W of Greenwich */
  int  tz_dsttime;     /*!< type of dst correction */
};
 
/*! 
 * @brief Windows counterpart for POSIX gettimeofday() 
 * @param[out] tv - reference to timeval structure, will be filled with current time upon return.
 * @param[out] tz - can be NULL. If not NULL, written with timezone information.
 * @return returns 0 on success, <>0 otherwise
 */
int gettimeofday(struct timeval *tv, struct timezone *tz);

#endif /* define TIME_OF_DAY_H */
