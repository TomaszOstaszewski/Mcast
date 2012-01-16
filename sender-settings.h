/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @brief
 * @file sender-settings.h
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
#if !defined MCAST_SENDER_SETTINGS_C03B81E9_5988_44C7_B40B_CCEE3B74E961
#define MCAST_SENDER_SETTINGS_C03B81E9_5988_44C7_B40B_CCEE3B74E961

#if defined __cplusplus
extern "C" {
#endif
#include <windows.h>
#include "std-int.h"
#include "mcast-settings.h"

struct master_riff_chunk;

/*!
 * @brief
 */
struct sender_settings {
	uint16_t chunk_size_;
	uint16_t send_delay_;
	struct master_riff_chunk * chunk_;
	struct mcast_settings mcast_settings_;
};

/*!
 * @brief
 */
int get_default_settings(HINSTANCE hInst, struct sender_settings * p_settings);

/*!
 * @brief
 */
int sender_settings_validate(struct sender_settings const * p_settings);

int sender_settings_compare(struct sender_settings const * p_left, struct sender_settings const * p_right);

void sender_settings_copy(struct sender_settings * p_dest, struct sender_settings const * p_source);

#if defined __cplusplus
}
#endif

#endif /* MCAST_SENDER_SETTINGS_C03B81E9_5988_44C7_B40B_CCEE3B74E961 */

