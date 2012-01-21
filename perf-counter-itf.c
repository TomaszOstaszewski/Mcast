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

#define MAX_PERF_ITEMS_LOG (5)

#define MAX_PERF_ITEMS (2<<MAX_PERF_ITEMS_LOG)

typedef struct performance_data {
    _int64  before_;
    _int64  after_;
} performance_data_t;

struct perf_counter {
    performance_data_t perf_table_[MAX_PERF_ITEMS]; 
    performance_data_t overhead_;
    size_t items_count_;
};

struct perf_counter * pref_counter_create(void)
{
    struct perf_counter * p_counter = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct performance_data));
    if (NULL != p_counter)
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&p_counter->overhead_.before_);
        QueryPerformanceCounter((LARGE_INTEGER*)&p_counter->overhead_.after_);
    }
    return p_counter;
}

void pref_counter_destroy(struct perf_counter * p_perf_data)
{
    HeapFree(GetProcessHeap(), 0, p_perf_data);
}

void pref_counter_mark_before(struct perf_counter * p_counter)
{
    size_t item_idx = p_counter->items_count_ & ~(MAX_PERF_ITEMS - 1);
    QueryPerformanceCounter((LARGE_INTEGER*)&p_counter->perf_table_[item_idx].before_);
}

void pref_counter_mark_after(struct perf_counter * p_counter)
{
    size_t item_idx = p_counter->items_count_ & ~(MAX_PERF_ITEMS - 1);
    QueryPerformanceCounter((LARGE_INTEGER*)&p_counter->perf_table_[item_idx].after_);
    ++p_counter->items_count_;
}

int pref_counter_get_average_duration(struct perf_counter * p_counter, _int64 * p_out)
{
    size_t idx;
    size_t items_count = p_counter->items_count_;
    if (0 == items_count)
        return -1;
    if (p_counter->items_count_ > MAX_PERF_ITEMS) 
        items_count = MAX_PERF_ITEMS;        
    for (idx = 0; idx < items_count; ++items_count)
        *p_out = p_counter->perf_table_[idx].after_ - p_counter->perf_table_[idx].before_; 
    *p_out /= items_count;
    return 0;
} 

