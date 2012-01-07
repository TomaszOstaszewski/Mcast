#include "pcc.h"
#include "mcast-sender-settings.h"

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
 *
 */
#define DEFAULT_MCASTPORT (25000)

/*!
 * @brief 
 */
#define DEFAULT_CHUNK_SEND_TIMEOUT (85)

/*!
 * @brief
 */
#define DEFAULT_TTL (8)

/*!
 * @brief 
 */
#define DEFAULT_WAV_CHUNK_SIZE    (1024+256+128)

void get_default_settings(struct sender_settings * p_settings)
{
	p_settings->send_delay_ = DEFAULT_CHUNK_SEND_TIMEOUT;
	p_settings->ipv4_mcast_group_addr_ = inet_addr(DEFAULT_MCASTADDRV4);
	p_settings->mcast_port_ = DEFAULT_MCASTPORT;
	p_settings->ttl_ = DEFAULT_TTL;
	p_settings->chunk_size_ = DEFAULT_WAV_CHUNK_SIZE;
	p_settings->chunk_ = NULL;
}

