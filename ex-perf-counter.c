/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file ex-perf-counter.c
 * @brief Example usage of performance counter functions
 * @details 
 * @author T. Ostaszewski
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
#include "pcc.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "std-int.h"
#include "perf-counter-itf.h"

/*!
 * @brief A dummy container to hold function values.
 */
static struct x_y {
    double x_; /*!< Argument for the Bessel's functions.*/
    double y_j0; /*!< Value of the Bessel j0 function.*/
    double y_j1; /*!< Value or the Bessel j1 function.*/
} func_table[] = {
    { 0.0, 0, 0 },
    { 0.1, 0, 0 },
    { 0.2, 0, 0 },
    { 0.3, 0, 0 },
    { 0.4, 0, 0 },
    { 0.5, 0, 0 },
    { 0.6, 0, 0 },
    { 0.9, 0, 0 },
    { 0.8, 0, 0 },
    { 0.9, 0, 0 },
    { 1.0, 0, 0 },
    { 1.1, 0, 0 },
    { 1.2, 0, 0 },
    { 1.3, 0, 0 },
    { 1.4, 0, 0 },
    { 1.5, 0, 0 },
    { 1.6, 0, 0 },
    { 1.7, 0, 0 },
    { 1.8, 0, 0 },
    { 1.9, 0, 0 },
    { 2.0, 0, 0 },
    { 2.1, 0, 0 },
    { 2.2, 0, 0 },
    { 3.0, 0, 0 },
    { 3.1, 0, 0 },
    { 3.2, 0, 0 },
    { 3.3, 0, 0 },
    { 3.4, 0, 0 },
    { 3.5, 0, 0 },
    { 3.6, 0, 0 },
    { 3.7, 0, 0 },
    { 3.8, 0, 0 },
    { 3.9, 0, 0 },
    { 4.0, 0, 0 },
    { 4.1, 0, 0 },
    { 4.2, 0, 0 },
    { 4.3, 0, 0 },
    { 4.4, 0, 0 },
    { 4.5, 0, 0 },
    { 4.6, 0, 0 },
    { 4.7, 0, 0 },
    { 4.8, 0, 0 },
    { 4.9, 0, 0 },
    { 5.0, 0, 0 },
    { 5.1, 0, 0 },
    { 5.2, 0, 0 },
    { 5.3, 0, 0 },
    { 5.4, 0, 0 },
    { 5.5, 0, 0 },
    { 5.6, 0, 0 },
    { 5.7, 0, 0 },
    { 5.8, 0, 0 },
    { 5.9, 0, 0 },
    { 6.0, 0, 0 },
    { 6.1, 0, 0 },
    { 6.2, 0, 0 },
    { 6.3, 0, 0 },
    { 6.4, 0, 0 },
    { 6.5, 0, 0 },
    { 6.6, 0, 0 },
    { 6.7, 0, 0 },
    { 6.8, 0, 0 },
    { 6.9, 0, 0 },
    { 7.0, 0, 0 },
    { 7.1, 0, 0 },
    { 7.2, 0, 0 },
    { 7.3, 0, 0 },
    { 7.4, 0, 0 },
    { 7.5, 0, 0 },
    { 7.6, 0, 0 },
    { 7.7, 0, 0 },
    { 7.8, 0, 0 },
    { 7.9, 0, 0 },
    { 8.0, 0, 0 },
    { 8.1, 0, 0 },
    { 8.2, 0, 0 },
    { 8.3, 0, 0 },
    { 8.4, 0, 0 },
    { 8.5, 0, 0 },
    { 8.6, 0, 0 },
    { 8.7, 0, 0 },
    { 8.8, 0, 0 },
    { 8.9, 0, 0 },
 };

/*!
 * @brief Demonstration of the performance counter usage.
 */
int main(int argc, char ** argv)
{
	struct perf_counter *  counter;
	_int64 total, avg, freq;
	size_t idx;
	counter = pref_counter_create();
    assert(counter);
    freq = pref_counter_get_freq(counter);
	for (idx = 0; idx<sizeof(func_table)/sizeof(func_table[0]); ++idx)	
	{
        /* The performance counter will measuer from the beginning to an end */
        /* Mark the beginning. */
        pref_counter_mark_before(counter);
        func_table[idx].y_j0 = _j0(func_table[idx].x_);
        func_table[idx].y_j1 = _j1(func_table[idx].x_);
        printf("%u : _j0(%2.2f) = %+4.4f _j0(%2.2f) = %+4.4f\n", __LINE__, func_table[idx].x_, func_table[idx].y_j0, func_table[idx].x_, func_table[idx].y_j1);
        /* Mark the end. */
	    pref_counter_mark_after(counter);
	}
	pref_counter_get_duration(counter, &total, &avg);
	pref_counter_destroy(counter);
    printf("Time it took to execute: total = %I64d, average = %I64d\n", total, avg);
    printf("Freq = %I64d\n", freq);
    printf("Time [us]: total = %I64d, average = %I64d\n", (1000000*total)/freq, (1000000*avg)/freq);
	return 0;
}

