/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file perf-counter-itf.c
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
 * @brief 
 * @details 
 */

#include "pcc.h"
#include "perf-counter-itf.h"

struct perf_counter * pref_counter_create(unsigned short entries_count)
{
            //QueryPerformanceCounter((LARGE_INTEGER*)&overhead.before_);
            //QueryPerformanceCounter((LARGE_INTEGER*)&overhead.after_);
 }

void pref_counter_destroy(struct perf_counter * p_perf_data)
{
}

size_t pref_counter_mark_before(struct perf_counter * pref_data)
{
            QueryPerformanceCounter((LARGE_INTEGER*)&el_time[n_count].after_);
            if (++n_count >= EL_TIME_MAX)
            {
                size_t idx;
                _int64  sum = 0;
                _int64 overhead_calc = overhead.after_ - overhead.before_;
                for (idx = 0; idx <EL_TIME_MAX; ++idx)
                {
                    sum += el_time[idx].after_ - el_time[idx].before_ - overhead_calc;
                }
                debug_outputln("%s %5.5d : %I64d", __FILE__, __LINE__, sum/EL_TIME_MAX);
                n_count = 0;
            }
}

size_t pref_counter_mark_after(struct perf_counter * pref_data, size_t index)
{
}

