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
#define DEFAULT_MCASTPORT "25000"

struct mcast_settings const * get_default_mcast_settings(void)
{
	static struct mcast_settings default_settings = {
		FALSE,
		TRUE,
		NULL,
		NULL,
		8,
		DEFAULT_MCASTADDRV4,
		DEFAULT_MCASTPORT
	};
	return &default_settings;
}

