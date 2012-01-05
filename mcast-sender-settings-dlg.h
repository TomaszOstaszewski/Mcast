#if !defined MCAST_SENDER_SETTINGS_DLG_BDDF0787_F748_479B_BB79_F5C26F739FB4
#define MCAST_SENDER_SETTINGS_DLG_BDDF0787_F748_479B_BB79_F5C26F739FB4

#if defined __cplusplus
extern "C" {
#endif
#include <windows.h>
#include "std-int.h"

struct platform_specific_data {
	HINSTANCE hInst_;
	HWND hParent_;
};

struct sender_settings {
	unsigned int send_delay_;
	unsigned long	ipv4_mcast_group_addr_;	
	uint16_t	mcast_port_;
};

void do_dialog(struct platform_specific_data const * p_platform, struct sender_settings * p_settings);

#if defined __cplusplus
}
#endif

#endif /* !defined MCAST_SENDER_SETTINGS_DLG_BDDF0787_F748_479B_BB79_F5C26F739FB4 */

