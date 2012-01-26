/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @example ex-perf-counter.c
 * Here's an example on how to get a timing of a piece of code using interface presended in pref-counter-itf.h. 
 *
 * When executed, gives something like this on my machine:
 * <pre>
 *_j0(0.10) = +0.9975 _j0(0.10) = +0.0499
 *_j0(0.20) = +0.9900 _j0(0.20) = +0.0995
 * </pre>
 * .... a lot of other lines omitted for brevity...
 * <pre>
 *_j0(8.60) = +0.0146 _j0(8.60) = +0.2728
 *_j0(8.70) = -0.0125 _j0(8.70) = +0.2697
 * Time it took to execute: total = 884129, average = 8841
 * Freq = 14318180
 * Time [us]: total = 61748, average = 617
 * </pre>
 * So, on average, it takes 617 microseconds to calculate values for 2 Bessel's functions and to display them, along with the argument value. The whole loop took about 62 milliseconds to execute.
 * Your millage may vary, as those values are strictly bound to the CPU frequency. But the general idea of measurement shall be pretty much the same.
 * @file ex-perf-counter.c
 * @brief Example usage of performance counter functions
 * @author T. Ostaszewski
 * @date Jan-2012
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
    { 0.1, 0, 0 }, { 0.2, 0, 0 }, { 0.3, 0, 0 }, { 0.4, 0, 0 }, { 0.5, 0, 0 },
    { 0.6, 0, 0 }, { 0.9, 0, 0 }, { 0.8, 0, 0 }, { 0.9, 0, 0 }, { 1.0, 0, 0 },
    { 1.1, 0, 0 }, { 1.2, 0, 0 }, { 1.3, 0, 0 }, { 1.4, 0, 0 }, { 1.5, 0, 0 },
    { 1.6, 0, 0 }, { 1.7, 0, 0 }, { 1.8, 0, 0 }, { 1.9, 0, 0 }, { 2.0, 0, 0 },
    { 2.1, 0, 0 }, { 2.2, 0, 0 }, { 2.3, 0, 0 }, { 2.4, 0, 0 }, { 2.5, 0, 0 }, 
    { 2.6, 0, 0 }, { 2.7, 0, 0 }, { 2.8, 0, 0 }, { 2.9, 0, 0 }, { 3.0, 0, 0 },
    { 3.1, 0, 0 }, { 3.2, 0, 0 }, { 3.3, 0, 0 }, { 3.4, 0, 0 }, { 3.5, 0, 0 },
    { 3.6, 0, 0 }, { 3.7, 0, 0 }, { 3.8, 0, 0 }, { 3.9, 0, 0 }, { 4.0, 0, 0 },
    { 4.1, 0, 0 }, { 4.2, 0, 0 }, { 4.3, 0, 0 }, { 4.4, 0, 0 }, { 4.5, 0, 0 },
    { 5.6, 0, 0 }, { 5.7, 0, 0 }, { 5.8, 0, 0 }, { 5.9, 0, 0 }, { 6.0, 0, 0 },
    { 3.8, 0, 0 }, { 3.9, 0, 0 }, { 4.0, 0, 0 }, { 4.1, 0, 0 }, { 4.2, 0, 0 },
    { 4.3, 0, 0 }, { 4.4, 0, 0 }, { 4.5, 0, 0 }, { 4.6, 0, 0 }, { 4.7, 0, 0 },
    { 4.8, 0, 0 }, { 4.9, 0, 0 }, { 5.0, 0, 0 }, { 5.1, 0, 0 }, { 5.2, 0, 0 },
    { 5.3, 0, 0 }, { 5.4, 0, 0 }, { 5.5, 0, 0 }, { 5.6, 0, 0 }, { 5.7, 0, 0 },
    { 5.8, 0, 0 }, { 5.9, 0, 0 }, { 6.0, 0, 0 }, { 6.1, 0, 0 }, { 6.2, 0, 0 },
    { 6.3, 0, 0 }, { 6.4, 0, 0 }, { 6.5, 0, 0 }, { 6.6, 0, 0 }, { 6.7, 0, 0 },
    { 6.8, 0, 0 }, { 6.9, 0, 0 }, { 7.0, 0, 0 }, { 7.1, 0, 0 }, { 7.2, 0, 0 },
    { 7.3, 0, 0 }, { 7.4, 0, 0 }, { 7.5, 0, 0 }, { 7.6, 0, 0 }, { 7.7, 0, 0 },
    { 7.8, 0, 0 }, { 7.9, 0, 0 }, { 8.0, 0, 0 }, { 8.1, 0, 0 }, { 8.2, 0, 0 },
    { 8.3, 0, 0 }, { 8.4, 0, 0 }, { 8.5, 0, 0 }, { 8.6, 0, 0 }, { 8.7, 0, 0 },
 };

/*!
 * @brief Demonstration of the performance counter usage.
 */
int main(int argc, char ** argv)
{
	struct perf_counter *  counter;
	_int64 total, avg, freq;
	size_t idx;
    /* Create the performance counter object */
	counter = perf_counter_create();
    assert(counter);
    /* Get the timer frequency - it will be needed to calculate elapsed wall time of the profiled code segment. */
    freq = perf_counter_get_freq(counter);
	for (idx = 0; idx<sizeof(func_table)/sizeof(func_table[0]); ++idx)	
	{
        /* The performance counter will measure from the start to the end 
         * In this specific example, we measure how much time it takes to compute values of the Bessel's functions
         * and to display them.
         */
        /* Mark the start. */
        perf_counter_mark_before(counter);
        func_table[idx].y_j0 = _j0(func_table[idx].x_);
        func_table[idx].y_j1 = _j1(func_table[idx].x_);
        printf("_j0(%2.2f) = %+4.4f _j0(%2.2f) = %+4.4f\n", func_table[idx].x_, func_table[idx].y_j0, func_table[idx].x_, func_table[idx].y_j1);
        /* Mark the end. */
	    perf_counter_mark_after(counter);
	}
    /* Get total time and average it takes to execute the code segment between calls to perf_counter_mark_before and perf_counter_mark_end */
	perf_counter_get_duration(counter, &total, &avg);
    /* Destroy the performance counter object */
	perf_counter_destroy(counter);
    /* Display results */
    printf("Time it took to execute: total = %I64d, average = %I64d\n", total, avg);
    printf("Freq = %I64d\n", freq);
    printf("Time [us]: total = %I64d, average = %I64d\n", (1000000*total)/freq, (1000000*avg)/freq);
	return 0;
}

