/*!
 * @brief Functions for multicast group membership settings manipulation.
 * @file mcast-settings.c
 * @author T.Ostaszewski
 * @date Jan-2012
 */
#include "pcc.h"
#include "mcast-settings.h"

/*!
 * @brief 
 */
#define DEFAULT_MCASTADDRV4 "234.5.6.7"

/*!
 * @brief 
 */
#define DEFAULT_MCASTADDRV6 "ff12::1"

/*!
 * @brief 
 */
#define DEFAULT_MCASTPORT (25000)

static struct mcast_settings g_default_settings;

struct mcast_settings const * get_default_mcast_settings(void)
{
	unsigned long net_addr = inet_addr(DEFAULT_MCASTADDRV4);
	g_default_settings.bReuseAddr_ = TRUE;
	memcpy(&g_default_settings.mcast_addr_.sin_addr, &net_addr, sizeof(unsigned long));
	g_default_settings.mcast_addr_.sin_port = htons(DEFAULT_MCASTPORT);
	return &g_default_settings;
}

