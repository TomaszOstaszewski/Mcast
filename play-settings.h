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
	/*!
	 * @brief Default timeout, in milliseconds, for multimedia play timer.
	 * @details Each time the timeout expires, the DirectSound fill-buffer-and-play function will be invoked.
	 */
 	UINT 	timer_delay_;
	/*!
	 * @brief Timer resolution for multimedia play timer.
     */
	UINT 	timer_resolution_;
    /*!
  	 * @brief Default size, in bytes, of the play buffer to be used in DirectSound. 
 	 */
	UINT	play_buffer_size_;
};

/*!
 * @brief Returns default play settings. 
 * @param[in,out] p_settings pointer to the settings structure. This structure will be written with the default settings.
 * @return returns non-zero on success, 0 otherwise.
 */
int play_settings_get_default(struct play_settings * p_settings);

/*!
 * @brief Validates, if settings are correct.
 * @details Invalid multicast settings are for example those, which has a non multicast IPv4 address.
 * @param[in] p_settings settings to be check for validity. 
 * @return returns non-zero on success, 0 otherwise.
 */
int play_settings_validate(struct play_settings const * p_settings);

/*!
 * @brief Copies the receiver settings from one structure into another.
 * @details After the copy, the p_dest will have settings of p_source. The p_source remains unaltered.
 * @param[in,out] p_dest target of the copy. to this sturcture settings will be written.
 * @param[in,out] p_source copy source. From this sturcture settings will be read.
 */
void play_settings_copy(struct play_settings * p_dest, struct play_settings const * p_source);

/*!
 * @brief Exchanges the settings between each other.
 * @details After the exchange, the p_left' will have settings of p_right, wherease p_right' will have settings of p_left.
 * @param[in,out] p_left first object to swap data with.
 * @param[in,out] p_right second object to swap data with.
 * @return returns non-zero on success, 0 otherwise.
 */
void play_settings_swap(struct play_settings * p_left, struct play_settings * p_right);

/*!
 * @brief Compares the settings with each other.
 * @param[in,out] p_left left hand side of the comparison.
 * @param[in,out] p_right right hand side of the comparison.
 * @return returns non zero if settings are equal, returns 0 otherwise.
 */
int play_settings_compare(struct play_settings const * p_left, struct play_settings const * p_right);

#if defined __cplusplus
}
#endif

#endif /* !defined PLAY_SETTINGS_F482A975_7C28_43C6_88FA_8A60EE598052 */

