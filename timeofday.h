/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file timeofday.h
 * @author T. Ostaszewski
 * @par License
 * @code Copyright 2012 Tomasz Ostaszewski. All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * 	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation 
 * 	and/or other materials provided with the distribution.
  * THIS SOFTWARE IS PROVIDED BY Tomasz Ostaszewski AS IS AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL Tomasz Ostaszewski OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE.
  * The views and conclusions contained in the software and documentation are those of the 
 * authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of Tomasz Ostaszewski.
 * @endcode
 * @date 04-Jan-2012
 * @brief Windows wrappers for POSIX time routines header.
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

struct timeval;
 
/*! 
 * @brief Windows counterpart for POSIX gettimeofday() 
 * @param[out] tv - reference to timeval structure, will be filled with current time upon return.
 * @param[out] tz - can be NULL. If not NULL, written with timezone information.
 * @return returns 0 on success, <>0 otherwise
 */
int gettimeofday(struct timeval *tv, struct timezone *tz);

#endif /* define TIME_OF_DAY_H */
