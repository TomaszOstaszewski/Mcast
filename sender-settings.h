/*!
 * @brief
 * @file mcast-sender-settings.h
 * @author T.Ostaszewski
 * @date Jan-2012
 */
#if !defined MCAST_SENDER_SETTINGS_C03B81E9_5988_44C7_B40B_CCEE3B74E961
#define MCAST_SENDER_SETTINGS_C03B81E9_5988_44C7_B40B_CCEE3B74E961

#if defined __cplusplus
extern "C" {
#endif
#include <windows.h>
#include "std-int.h"
#include "mcast-settings.h"

struct master_riff_chunk;

/*!
 * @brief
 */
struct sender_settings {
	uint16_t chunk_size_;
	uint16_t send_delay_;
	struct master_riff_chunk * chunk_;
	struct mcast_settings mcast_settings_;
};

/*!
 * @brief
 */
int get_default_settings(HINSTANCE hInst, struct sender_settings * p_settings);

/*!
 * @brief
 */
int sender_settings_validate(struct sender_settings const * p_settings);

#if defined __cplusplus
}
#endif

#endif /* MCAST_SENDER_SETTINGS_C03B81E9_5988_44C7_B40B_CCEE3B74E961 */

