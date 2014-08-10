/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @brief
 * @file play-settings.c
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
#include "play-settings.h"

/*!
 * @brief Maximum acceptable multimedia timer timespan.
 */
#define MAX_TIMER_DELAY (500)

/*!
 * @brief Mimimum acceptable multimedia timer timespan.
 */
#define MIN_TIMER_DELAY (5)

/*!
 * @brief Default size, in bytes, of a single play chunk.
 */
#define MAX_PLAY_CHUNK_SIZE (16384)

/*!
 * @brief Default size, in bytes, of a single play chunk.
 */
#define MIN_PLAY_CHUNK_SIZE (512)

/*!
 * @brief Default size, in bytes, of a single play chunk.
 */
#define DEFAULT_PLAY_CHUNK_SIZE (1024)

/*!
 * @brief Default number of play chunk.s
 * @details Total buffer occupied by PCM data used for replaying it will be : number_of_chunks * size_of_single_chunk
 */
#define MAX_NUMBER_OF_CHUNKS (16)

/*!
 * @brief Default number of play chunk.s
 * @details Total buffer occupied by PCM data used for replaying it will be : number_of_chunks * size_of_single_chunk
 */
#define MIN_NUMBER_OF_CHUNKS (2)

/*!
 * @brief Default number of play chunk.s
 * @details Total buffer occupied by PCM data used for replaying it will be : number_of_chunks * size_of_single_chunk
 */
#define DEFAULT_NUMBER_OF_CHUNKS (2)

/*!
 * @brief Default timeout, in milliseconds, for multimedia play timer.
 * @details Each time the timeout expires, the DirectSound fill-buffer-and-play function will be invoked.
 */
#define DEFAULT_TIMER_DELAY (10)

/*!
 * @brief Timer resolution for multimedia play timer.
 */
#define DEFAULT_TIMER_RESOLUTION (1)

int play_settings_get_default(struct play_settings * p_settings)
{
    p_settings->timer_delay_ = DEFAULT_TIMER_DELAY;
    p_settings->timer_resolution_ = DEFAULT_TIMER_RESOLUTION;
    p_settings->play_buffer_size_ = DEFAULT_PLAY_CHUNK_SIZE;
    p_settings->play_chunks_count_ = DEFAULT_NUMBER_OF_CHUNKS;
	return 1;
}

int play_settings_validate(struct play_settings const * p_settings)
{
	if (p_settings->timer_delay_ < MIN_TIMER_DELAY || p_settings->timer_delay_ > MAX_TIMER_DELAY)
		return 0;	
	if (p_settings->play_buffer_size_ < MIN_PLAY_CHUNK_SIZE || p_settings->play_buffer_size_ > MAX_PLAY_CHUNK_SIZE)
		return 0;	
	if (p_settings->play_chunks_count_ < MIN_NUMBER_OF_CHUNKS || p_settings->play_chunks_count_ > MAX_NUMBER_OF_CHUNKS)
		return 0;	
    return 1;
}

void play_settings_copy(struct play_settings * p_dest, struct play_settings const * p_source)
{
    memcpy(p_dest, p_source, sizeof(struct play_settings));
}

void play_settings_swap(struct play_settings * p_left, struct play_settings * p_right)
{
    struct play_settings tmp;
    memcpy(&tmp, p_left, sizeof(struct play_settings));
    memcpy(p_left, p_right, sizeof(struct play_settings));
    memcpy(p_right, &tmp, sizeof(struct play_settings));
}

int play_settings_compare(struct play_settings const * p_left, struct play_settings const * p_right)
{
    return !memcmp(p_left, p_right, sizeof(struct play_settings));
}

