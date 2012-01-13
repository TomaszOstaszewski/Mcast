/*!
 * @brief
 * @file receiver-settings.c
 * @brief Implementation of the sender's settings representation.
 * @details Contains functions for:
 * \li returning a default settings.
 * \li validating settings for correctness.
 * @author T.Ostaszewski
 * @date Jan-2012
 * @par License
 * @code Copyright 2012 Tomasz Ostaszewski. All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * 	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation 
 * 	and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY Tomasz Ostaszewski AS IS AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL Tomasz Ostaszewski OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE.
 * The views and conclusions contained in the software and documentation are those of the 
 * authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of Tomasz Ostaszewski.
 * @endcode
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

