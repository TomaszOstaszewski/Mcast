/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @brief
 * @file receiver-settings.h
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
 *  The views and conclusions contained in the software and documentation are those of the 
 * authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of Tomasz Ostaszewski.
 * @endcode
 */
#if !defined RECEIVER_SETTINGS_5847D978_FD86_4CA6_96A8_D3AF2F5DD975
#define RECEIVER_SETTINGS_5847D978_FD86_4CA6_96A8_D3AF2F5DD975

#if defined __cplusplus
extern "C" {
#endif
#include <windows.h>
#include "std-int.h"
#include "mcast-settings.h"

struct master_riff_chunk;

/*!
 * @brief A structure that describes receiver parameters.
 */
struct receiver_settings {
	struct master_riff_chunk * chunk_;
	uint16_t	play_buffer_size_;
	uint16_t	poll_sleep_time_;
	uint16_t 	timer_delay_;
	uint16_t	pad_;
	struct mcast_settings mcast_settings_;
};

/*!
 * @brief
 */
int receiver_get_default_settings(HINSTANCE hInst, struct receiver_settings * p_settings);

/*!
 * @brief
 */
int receiver_validate_settings(struct receiver_settings const * p_settings);

#if defined __cplusplus
}
#endif

#endif /* !defined RECEIVER_SETTINGS_5847D978_FD86_4CA6_96A8_D3AF2F5DD975 */

