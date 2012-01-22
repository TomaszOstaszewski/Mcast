/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file perf-counter-itf.h
 * @brief Performance counter utilities interface.
 * @details Those wrappers are here to avoid writting the same boilerplate code again and again. Here's a short
 * instruction on how to use this functions:
 * \li Create a performance counter object.
 * \li In the piece of code you want to query performance of - call pref_counter_mark_before() before the timed piece of code execution and then pref_counter_mark_after() after the piece of code.
 * \li After this code has been executed enough number of times, call pref_counter_get_average_duration() to get average time (in clock ticks) it took to execute the code.
 * @author T.Ostaszewski
 * @date 04-Jan-2012
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
 */
#if !defined PERF_COUNTER_ITF_225F550A_0FE3_45A1_B7C6_EE2E2B97ECD8
#define PERF_COUNTER_ITF_225F550A_0FE3_45A1_B7C6_EE2E2B97ECD8

#if defined __cplusplus
extern "C" {
#endif
#include <windows.h>
#include <stdlib.h>
#include <stddef.h>

struct perf_counter;

/*!
 * @brief Creates a performance measurement object.
 * @return On success, returns a handle to the performance measurement object. On failure, returns NULL.
 * @sa pref_counter_destroy
 */
struct perf_counter * pref_counter_create(void);

/*!
 * @brief Destroys a performance measurement object.
 * @param[in] p_counter a handle to the performance measurement object, obtained via call to perf_counter_create.
 * @sa pref_counter_create
 */
void pref_counter_destroy(struct perf_counter * p_counter);

/*!
 * @brief Marks the beginning of the measurement of the profiled code segment.
 * @param[in] p_counter handle to the performence measurement object, obtained via call to perf_counter_create().
 */
void pref_counter_mark_before(struct perf_counter * p_counter);

/*!
 * @brief Marks the end beginning of the measurement of the profiled code segment.
 * @param[in] p_counter handle to the performence measurement object, obtained via call to perf_counter_create().
 */
void pref_counter_mark_after(struct perf_counter * p_counter);

/*!
 * @brief Gets the total and average duration it takes to execute a profiled code segment.
 * @param[in] p_counter handle to the performence measurement object, obtained via call to perf_counter_create().
 * @param[out] p_total this memory location will be written with <b>total</b> time it takes to execute profiled code segment. 
 * @param[out] p_avg this memory location will be written with <b>average</b> time it takes to execute profiled code segment. 
 * @return Returns a non-zero value on success, 0 otherwise.
 * @attention The units of used by p_total and p_avg are timer ticsk. To covert it to wall time, please divide it by value obtained
 * via call to perf_counter_get_freq().
 * @sa perf_counter_get_freq
 */
int pref_counter_get_duration(struct perf_counter * p_counter, _int64 * p_total, _int64 * p_avg);

/*!
 * @brief Returns the frequency of the timer used for performance measurement.
 * @param[in] p_counter handle to the performence measurement object, obtained via call to perf_counter_create().
 * @return returns the frequency of the timer used for performance measurement.
 * @sa pref_counter_get_duration
 */
_int64 pref_counter_get_freq(struct perf_counter * p_counter);

/*!
 * \example ex-perf-counter.c
 * Here's an example on how to get a timing of a piece of code using this functions.
 */

#if defined __cplusplus
}
#endif

#endif /* defined PERF_COUNTER_ITF_225F550A_0FE3_45A1_B7C6_EE2E2B97ECD8 */

