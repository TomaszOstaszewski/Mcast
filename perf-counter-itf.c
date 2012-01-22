/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file perf-counter-itf.c
 * @brief Performance counter utilities implementation.
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
#include "perf-counter-itf.h"

/*!
 * @brief The exponent of the maximum number of items to be stored in the counter.
 */
#define MAX_PERF_ITEMS_LOG (6)

/*!
 * @brief The maximum number of items of items to be stored in the counter. This needs to be a power of 2.
 */
#define MAX_PERF_ITEMS (2<<MAX_PERF_ITEMS_LOG)

/*!
 * @brief Defines a single performance meausrement. 
 * @details Holds the performance counter measured before and after the profiled code segment. To get the duration it took to execute
 * the profiled code segment, subtract before_ from after_.
 */
struct perf_data_item {
    /*!
     * @brief Timestamp taken <b>before</b> measured code segment.
     */
    _int64  before_; 
    /*!
     * @brief Timestamp taken <b>after</b> measured code segment.
     */
    _int64  after_;
};

/*!
 * @brief Defines a complete performance meausrement. 
 * @details Holds the performance counter values measured by all the calls that marked the beginning and the end of the measurement. 
 * It holds the values of the overhead it take to perform the performance measurement itself. Finally, it has the frequence of the performance counter
 * which is needed to calculate wall time it takes to execute the profiled code segment. 
 */
struct perf_counter {
    size_t items_count_; /*!< Number of performence measurements. */
    _int64 overhead_; /*!< Overhead of the performance measuerment. */
    _int64 freq_; /*!< Frequence of the performance measurement timer. */
    struct perf_data_item perf_table_[MAX_PERF_ITEMS]; /*!< Holds the performance measurements. */ 
};

struct perf_counter * pref_counter_create(void)
{
    struct perf_counter * p_counter = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct perf_counter));
    if (NULL != p_counter)
    {
        _int64 before, after;
        QueryPerformanceCounter((LARGE_INTEGER*)&before);
        QueryPerformanceCounter((LARGE_INTEGER*)&after);
        p_counter->overhead_ = after - before;
        QueryPerformanceFrequency((LARGE_INTEGER*)&p_counter->freq_);
    }
    return p_counter;
}

void pref_counter_destroy(struct perf_counter * p_perf_data)
{
    HeapFree(GetProcessHeap(), 0, p_perf_data);
}

_int64 pref_counter_get_freq(struct perf_counter * p_counter)
{
    return p_counter->freq_;
}

void pref_counter_mark_before(struct perf_counter * p_counter)
{
    size_t item_idx = p_counter->items_count_ & (MAX_PERF_ITEMS-1);
    assert(item_idx < MAX_PERF_ITEMS);
    QueryPerformanceCounter((LARGE_INTEGER*)&p_counter->perf_table_[item_idx].before_);
}

void pref_counter_mark_after(struct perf_counter * p_counter)
{
    size_t item_idx = p_counter->items_count_ & (MAX_PERF_ITEMS-1);
    assert(item_idx < MAX_PERF_ITEMS);
    QueryPerformanceCounter((LARGE_INTEGER*)&p_counter->perf_table_[item_idx].after_);
    ++p_counter->items_count_;
}

int pref_counter_get_duration(struct perf_counter * p_counter, _int64 * p_total, _int64 * p_avg)
{
    size_t idx;
    size_t items_count = p_counter->items_count_;
    if (0 == items_count)
        return 0;
    if (p_counter->items_count_ > MAX_PERF_ITEMS) 
        items_count = MAX_PERF_ITEMS;        
    *p_total = 0;
    *p_avg = 0;
    for (idx = 0; idx < items_count; ++idx)
    {
        *p_total += p_counter->perf_table_[idx].after_ - p_counter->perf_table_[idx].before_ - p_counter->overhead_; 
    }
    *p_avg = *p_total/items_count;
    return 1;
} 

