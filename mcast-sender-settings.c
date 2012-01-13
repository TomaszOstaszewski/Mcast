/*!
 * @brief
 * @file mcast-sender-settings.c
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
#include "resource.h"
#include "mcast-sender-settings.h"
#include "wave_utils.h"

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

int get_default_settings(HINSTANCE hInst, struct sender_settings * p_settings)
{
	int result;
	result = init_master_riff(&p_settings->chunk_, hInst, MAKEINTRESOURCE(IDR_0_1));
	assert(0 == result);
	if (0 == result) 
	{
		p_settings->send_delay_ = DEFAULT_CHUNK_SEND_TIMEOUT;
		p_settings->ipv4_mcast_group_addr_ = inet_addr(DEFAULT_MCASTADDRV4);
		p_settings->mcast_port_ = DEFAULT_MCASTPORT;
		p_settings->ttl_ = DEFAULT_TTL;
		p_settings->chunk_size_ = DEFAULT_WAV_CHUNK_SIZE;
	}
	return result;
}

