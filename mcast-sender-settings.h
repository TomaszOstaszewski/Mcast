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

struct master_riff_chunk;

/*!
 * @brief
 */
struct sender_settings {
	struct master_riff_chunk * chunk_;
	uint16_t 	chunk_size_;
	unsigned int send_delay_;
	unsigned long	ipv4_mcast_group_addr_;	
	uint16_t	mcast_port_;
	uint16_t	ttl_;
};

/*!
 * @brief
 */
int get_default_settings(HINSTANCE hInst, struct sender_settings * p_settings);

#if defined __cplusplus
}
#endif

#endif /* MCAST_SENDER_SETTINGS_C03B81E9_5988_44C7_B40B_CCEE3B74E961 */

