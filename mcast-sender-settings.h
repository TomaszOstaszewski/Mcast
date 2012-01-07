#if !defined MCAST_SENDER_SETTINGS_C03B81E9_5988_44C7_B40B_CCEE3B74E961
#define MCAST_SENDER_SETTINGS_C03B81E9_5988_44C7_B40B_CCEE3B74E961

#if defined __cplusplus
extern "C" {
#endif
#include "std-int.h"

struct master_riff_chunk;

struct sender_settings {
	unsigned int send_delay_;
	unsigned long	ipv4_mcast_group_addr_;	
	uint16_t	mcast_port_;
	struct master_riff_chunk * chunk_;
};

#if defined __cplusplus
}
#endif

#endif /* MCAST_SENDER_SETTINGS_C03B81E9_5988_44C7_B40B_CCEE3B74E961 */

