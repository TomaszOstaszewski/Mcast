/*!
 * @brief
 * @file receiver-settings.c
 * @author T.Ostaszewski
 * @date Jan-2012
 */
#include "pcc.h"
#include "resource.h"
#include "receiver-settings.h"
#include "wave_utils.h"

/*!
 * @brief 
 */
#define DEFAULT_CHUNK_SEND_TIMEOUT (85)

/*!
 * @brief 
 */
#define DEFAULT_WAV_CHUNK_SIZE    (1024+256+128)

int receiver_get_default_settings(HINSTANCE hInst, struct receiver_settings * p_settings)
{
	int result;
	result = init_master_riff(&p_settings->chunk_, hInst, MAKEINTRESOURCE(IDR_0_1));
	assert(0 == result);
	if (0 == result) 
	{
		struct mcast_settings const * p_default_mcast_settings;
		p_settings->play_buffer_size_ = 1024;
		p_settings->poll_sleep_time_ = 50;
		p_settings->timer_delay_ = 10 ;
		p_default_mcast_settings = get_default_mcast_settings();
		memcpy(&p_settings->mcast_settings_, p_default_mcast_settings, sizeof(struct mcast_settings));
	}
	return result;
}

int receiver_validate_settings(struct receiver_settings const * p_settings)
{
	if (p_settings->timer_delay_ < 1 || p_settings->timer_delay_ > 1000)
		return 0;	
	if (p_settings->poll_sleep_time_ < 1 || p_settings->poll_sleep_time_ > 10000)
		return 0;	
	if (p_settings->play_buffer_size_ < 128 || p_settings->play_buffer_size_ > 8192)
		return 0;	
	return 1;
}

