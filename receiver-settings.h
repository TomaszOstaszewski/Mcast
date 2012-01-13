/*!
 * @brief
 * @file mcast-sender-settings.h
 * @author T.Ostaszewski
 * @date Jan-2012
 */
#if !defined RECEIVER_SETTINGS_5847D978_FD86_4CA6_96A8_D3AF2F5DD975
#define RECEIVER_SETTINGS_5847D978_FD86_4CA6_96A8_D3AF2F5DD975

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
struct receiver_settings {
	struct master_riff_chunk * chunk_;
	uint16_t	play_buffer_size_;
	uint16_t	poll_sleep_time_;
	uint16_t 	timer_delay_;
	uint16_t	pad_;
	struct mcast_settings mcast_settings_;
};

/*!
 * @brief
 */
int receiver_get_default_settings(HINSTANCE hInst, struct receiver_settings * p_settings);

/*!
 * @brief
 */
int receiver_validate_settings(struct receiver_settings const * p_settings);

#if defined __cplusplus
}
#endif

#endif /* !defined RECEIVER_SETTINGS_5847D978_FD86_4CA6_96A8_D3AF2F5DD975 */

