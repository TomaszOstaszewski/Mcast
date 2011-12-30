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

