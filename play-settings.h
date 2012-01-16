/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @brief
 * @file play-settings.h
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
#if !defined PLAY_SETTINGS_F482A975_7C28_43C6_88FA_8A60EE598052
#define PLAY_SETTINGS_F482A975_7C28_43C6_88FA_8A60EE598052

#if defined __cplusplus
extern "C" {
#endif
#include <windows.h>

/*!
 * @brief A structure that describes receiver parameters.
 */
struct play_settings {
	UINT 	timer_delay_;
	UINT 	timer_resolution_;
	UINT	play_buffer_size_;
};

/*!
 * @brief
 */
int play_settings_get_default(struct play_settings * p_settings);

/*!
 * @brief
 */
int play_settings_validate(struct play_settings const * p_settings);

/*!
 * @brief Copies the receiver settings from one structure to another.
 */
void play_settings_copy(struct play_settings * p_dest, struct play_settings const * p_source);

/*!
 * @brief Copies the receiver settings from one structure to another.
 */
void play_settings_swap(struct play_settings * p_left, struct play_settings * p_right);

/*!
 * @brief Copies the receiver settings from one structure to another.
 */
int play_settings_compare(struct play_settings const * p_left, struct play_settings const * p_right);

#if defined __cplusplus
}
#endif

#endif /* !defined PLAY_SETTINGS_F482A975_7C28_43C6_88FA_8A60EE598052 */

