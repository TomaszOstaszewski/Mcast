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
#include "receiver-settings.h"
#include "debug_helpers.h"
#include "receiver-res.h"

/*!
 * @brief Default timeout for network data retrieval. 
 * @details The application will wait that time long for audio data to appear on the socket.
 */
#define DEFAULT_NETPOLL_SLEEP_TIME (1)

/*!
 * @brief Default level for the circular buffer.
 * @details The default level of N gives 2^N items to be stored in the circular buffer.
 */
#define DEFAULT_CIRCULAR_BUFFER_LEVEL (12)

/*!
 * @brief Maximum level for the circular buffer.
 */
#define MAXIMUM_CIRCULAR_BUFFER_LEVEL (16)

/*!
 * @brief Minimum level for the circular buffer.
 */
#define MINIMUM_CIRCULAR_BUFFER_LEVEL (5)


int receiver_settings_get_default(struct receiver_settings * p_settings)
{
    p_settings->poll_sleep_time_        = DEFAULT_NETPOLL_SLEEP_TIME;
    p_settings->circular_buffer_level_  = DEFAULT_CIRCULAR_BUFFER_LEVEL;
    p_settings->wfex_.wFormatTag        = WAVE_FORMAT_PCM;
    p_settings->wfex_.nChannels         = 1;
    p_settings->wfex_.nSamplesPerSec    = 8000;
    p_settings->wfex_.nAvgBytesPerSec   = 16000;
    p_settings->wfex_.nBlockAlign 	    = 2;
    p_settings->wfex_.wBitsPerSample	= 16;
    p_settings->wfex_.cbSize 			= sizeof(WAVEFORMATEX);
    play_settings_get_default(&p_settings->play_settings_);
    mcast_settings_get_default(&p_settings->mcast_settings_);
	return 1;
}

int receiver_settings_validate(struct receiver_settings const * p_settings)
{
	if (p_settings->poll_sleep_time_ > 1000)
		return 0;	
    if (p_settings->circular_buffer_level_ > MAXIMUM_CIRCULAR_BUFFER_LEVEL || p_settings->circular_buffer_level_  < MINIMUM_CIRCULAR_BUFFER_LEVEL)
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

