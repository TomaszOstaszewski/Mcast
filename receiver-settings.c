/* ex: set shiftwidth=4 tabstop=4 expandtab: */
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
#include "debug_helpers.h"

/*!
 * @brief 
 */
#define DEFAULT_NETPOLL_SLEEP_TIME (15)

int receiver_settings_get_default(HINSTANCE hInst, struct receiver_settings * p_settings)
{
	int result;
    master_riff_chunk_t * p_riff_chunk;
    /*! \todo Add function to unload WAV resource */
	result = init_master_riff(&p_riff_chunk, hInst, MAKEINTRESOURCE(IDR_0_1));
	assert(result);
    if (result) 
	{
		p_settings->poll_sleep_time_ = DEFAULT_NETPOLL_SLEEP_TIME;
        copy_waveformatex_2_WAVEFORMATEX(&p_settings->wfex_, &p_riff_chunk->format_chunk_.format_);
        play_settings_get_default(&p_settings->play_settings_);
		mcast_settings_get_default(&p_settings->mcast_settings_);
	}
	return result;
}

int receiver_settings_validate(struct receiver_settings const * p_settings)
{
	if (p_settings->poll_sleep_time_ > 1000)
		return 0;	
    if (!mcast_settings_validate(&p_settings->mcast_settings_))
        return 0;
    if (!play_settings_validate(&p_settings->play_settings_))
        return 0;
	return 1;
}

void receiver_settings_copy(struct receiver_settings * p_dest, struct receiver_settings const * p_source)
{
    memcpy(p_dest, p_source, sizeof(struct receiver_settings));
}

void receiver_settings_swap(struct receiver_settings * p_left, struct receiver_settings * p_right)
{
    struct receiver_settings tmp;
    memcpy(&tmp, p_left, sizeof(struct receiver_settings));
    memcpy(p_left, p_right, sizeof(struct receiver_settings));
    memcpy(p_right, &tmp, sizeof(struct receiver_settings));
}

int receiver_settings_compare(struct receiver_settings const * p_left, struct receiver_settings const * p_right)
{
    return !memcmp(p_left, p_right, sizeof(struct receiver_settings));
}

