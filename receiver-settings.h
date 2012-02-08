/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file receiver-settings.h
 * @brief Declares data structure that define the multicast receiver settings. Also declares operations on those.
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
#include "play-settings.h"

/*!
 * @brief A structure that describes receiver parameters.
 */
struct receiver_settings {
	WAVEFORMATEX wfex_;
	UINT   poll_sleep_time_;
    UINT   circular_buffer_level_; /*!< A parameter that determines the size of the circular buffer */
    struct play_settings play_settings_;
	struct mcast_settings mcast_settings_;
};

/*!
 * @brief Returns default receiver settings. 
 * @param[in,out] p_settings pointer to the settings structure. This structure will be written with the default settings.
 * @return returns non-zero on success, 0 otherwise.
 */
int receiver_settings_get_default(struct receiver_settings * p_settings);

/*!
 * @brief Validates, if settings are correct.
 * @details Invalid settings are for example those, which has a non multicast IPv4 address.
 * @param[in] p_settings settings to be check for validity. 
 * @return returns non-zero on success, 0 otherwise.
 */
int receiver_settings_validate(struct receiver_settings const * p_settings);

/*!
 * @brief Copies the receiver settings from one structure to another.
 * @details After the copy, the p_dest will have settings of p_source. The p_source remains unaltered.
 * @param[in,out] p_dest target of the copy. to this sturcture settings will be written.
 * @param[in,out] p_source copy source. From this sturcture settings will be read.
 */
void receiver_settings_copy(struct receiver_settings * p_dest, struct receiver_settings const * p_source);

/*!
 * @brief Exchanges the settings between each other.
 * @details After the exchange, the p_left' will have settings of p_right, wherease p_right' will have settings of p_left.
 * @param[in,out] p_left first object to swap data with.
 * @param[in,out] p_right second object to swap data with.
 * @return returns non-zero on success, 0 otherwise.
 */
void receiver_settings_swap(struct receiver_settings * p_left, struct receiver_settings * p_right);

/*!
 * @brief Copies the receiver settings from one structure to another.
 * @details The compare and swap methods are so commonly used that they have been abstracted with this
 * function, rather to restoring to call memcmp(). Usage:
 * @code
 * if (receiver_settings_compare(p_left, p_righ))
 * {
 *      printf("Objects %p and %p are equal.\n", p_left, p_right);
 * }
 * else
 * {
 *      printf("Objects %p and %p are different.\n", p_left, p_right);
 * }
 * @endcode
 * @param[in] p_left first element of the comparison
 * @param[in] p_right second element of the comparison
 * @return returns a non-zero value if both elements are equal, returns 0 otherwise, i.e. returns 0 in case they <b>are not</b> equal.
 * @attention The return value is negation of this you would get from calling memcpy().
 */
int receiver_settings_compare(struct receiver_settings const * p_left, struct receiver_settings const * p_right);

#if defined __cplusplus
}
#endif

#endif /* !defined RECEIVER_SETTINGS_5847D978_FD86_4CA6_96A8_D3AF2F5DD975 */

