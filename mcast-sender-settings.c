/*!
 * @brief
 * @file mcast-sender-settings.c
 * @author T.Ostaszewski
 * @date Jan-2012
 */
#include "pcc.h"
#include "resource.h"
#include "mcast-sender-settings.h"
#include "wave_utils.h"

/*!
 * @brief 
 */
#define DEFAULT_CHUNK_SEND_TIMEOUT (85)

/*!
 * @brief 
 */
#define DEFAULT_WAV_CHUNK_SIZE    (1024+256+128)

int get_default_settings(HINSTANCE hInst, struct sender_settings * p_settings)
{
	int result;
	result = init_master_riff(&p_settings->chunk_, hInst, MAKEINTRESOURCE(IDR_0_1));
	assert(0 == result);
	if (0 == result) 
	{
		struct mcast_settings const * p_default_mcast_settings;
		p_settings->send_delay_ = DEFAULT_CHUNK_SEND_TIMEOUT;
		p_settings->chunk_size_ = DEFAULT_WAV_CHUNK_SIZE;
		p_default_mcast_settings = get_default_mcast_settings();
		memcpy(&p_settings->mcast_settings_, p_default_mcast_settings, sizeof(struct mcast_settings));
	}
	return result;
}

