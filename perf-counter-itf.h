#if !defined PERF_COUNTER_ITF_225F550A_0FE3_45A1_B7C6_EE2E2B97ECD8
#define PERF_COUNTER_ITF_225F550A_0FE3_45A1_B7C6_EE2E2B97ECD8

#if defined __cplusplus
extern "C" {
#endif
struct perf_counter;

struct perf_counter * pref_counter_create(unsigned short entries_count);
void pref_counter_destroy(struct perf_counter * p_perf_data);

size_t pref_counter_mark_before(struct perf_counter * pref_data);
size_t pref_counter_mark_after(struct perf_counter * pref_data, size_t index);

#if defined __cplusplus
}
#endif

#endif /* defined PERF_COUNTER_ITF_225F550A_0FE3_45A1_B7C6_EE2E2B97ECD8 */

