/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @brief Defines operatoins on the multicast sender settings data structure.
 * @file sender-settings.c
 * @author T.Ostaszewski
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
 * @date Jan-2012
 */
#include "pcc.h"
#include "sender-settings.h"
#include "wave_utils.h"
#include "sender-res.h"

/*!
 * @brief Default timespan between moments the subsequent audio packets are being send.
 */
#define DEFAULT_CHUNK_SEND_TIMEOUT (90)

/*!
 * @brief Default number of bytes in the audio packet.
 */
#define DEFAULT_WAV_CHUNK_SIZE    (1024+256+128)

/*!
 * @brief Lower boundary on time it takes to send next audio packet after the previous one has been sent.
 * @details This value is arbitrary.
 */
#define MIN_PACKET_DELAY (1)

/*!
 * @brief Upper boundary on time it takes to send next audio packet after the previous one has been sent.
 * @details This value is arbitrary.
 */
#define MAX_PACKET_DELAY (1000)

/*!
 * @brief Defines the minimum payload length. 
 * @details The value is calculated by taking the minimum Ehternet payload length (46 bytes) and subtracting lenghts of both IP header (20 bytes) and UDP header (8 bytes).
 */
#define MIN_PACKET_LENGTH (46-20+8)

/*!
 * @brief Defines the maximum payload length. 
 * @details The value is calculated by taking the maximum Ehternet payload length (1500 bytes) and subtracting lenghts of both IP header (20 bytes) and UDP header (8 bytes).
 */
#define MAX_PACKET_LENGTH (1500-20-8)

int get_default_settings(struct sender_settings * p_settings)
{
	int result;
	result = init_master_riff(&p_settings->chunk_, NULL, MAKEINTRESOURCE(IDR_0_1));
	assert(result);
	if (result) 
	{
		p_settings->send_delay_ = DEFAULT_CHUNK_SEND_TIMEOUT;
		p_settings->chunk_size_ = DEFAULT_WAV_CHUNK_SIZE;
		result = mcast_settings_get_default(&p_settings->mcast_settings_);
        assert(result);
	}
	return result;
}

int sender_settings_validate(struct sender_settings const * p_settings)
{
	if (p_settings->send_delay_ < MIN_PACKET_DELAY || p_settings->send_delay_ > MAX_PACKET_DELAY)
		return 0;
	if (p_settings->chunk_size_ < MIN_PACKET_LENGTH || p_settings->chunk_size_ > MAX_PACKET_LENGTH)
		return 0;
	return 1;
}

int sender_settings_compare(struct sender_settings const * p_left, struct sender_settings const * p_right)
{
	return !memcmp(p_left, p_right, sizeof(struct sender_settings));
}

void sender_settings_copy(struct sender_settings * p_dest, struct sender_settings const * p_source)
{
    struct sender_settings tmp;
    CopyMemory(&tmp, p_source, sizeof(struct sender_settings));
	CopyMemory(p_dest, &tmp, sizeof(struct sender_settings));
}

void sender_settings_swap(struct sender_settings * p_left, struct sender_settings * p_right)
{
    struct sender_settings tmp;
	CopyMemory(&tmp, p_left, sizeof(struct sender_settings));
	CopyMemory(p_left, p_right, sizeof(struct sender_settings));
	CopyMemory(p_right, &tmp, sizeof(struct sender_settings));
}

