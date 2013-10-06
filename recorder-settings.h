/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @brief Declares data structure that define the multicast sender settings. Also declares operations on those.
 * @file recorder-settings.h
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
#if !defined RECORDER_SETTINGS_H_8D4D6871_B617_4BD2_AD4C_F40D03A213BF
#define RECORDER_SETTINGS_H_8D4D6871_B617_4BD2_AD4C_F40D03A213BF

#if defined __cplusplus
extern "C" {
#endif
#include <windows.h>
#include <MMSystem.h>
#include "std-int.h"

/*!
 * @brief Describes the sender settings.
 */
struct recorder_settings;

typedef struct recorder_settings * recorder_settings_t;

/*!
 * @brief Returns the default settings.
 * @param[in] p_settings structure referenced by this pointer will be written with default settings.
 * @return returns 0 on success, <>0 otherwise.
 */
recorder_settings_t recorder_settings_get_default(void);

void recorder_settings_destroy(recorder_settings_t handle);

GUID const * recorder_settings_get_guid(struct recorder_settings const * p_settings);

/*!
 * @brief Returns size of the samples buffer.
 * @param[in] p_settings structure referenced by this pointer will be written with default settings.
 * @return samples buffer size.
 */
uint16_t recorder_settings_get_samples_buffer_size(struct recorder_settings const * p_settings);

/*!
 * @brief Returns size of the samples buffer.
 * @param[in] p_settings structure referenced by this pointer will be written with default settings.
 * @return samples buffer size.
 */
WAVEFORMATEX const * recorder_settings_get_waveformatex(struct recorder_settings const * p_settings);

/*!
 * @brief Validates if the sender settings are correct.
 * @param[in] p_settings describes settings to be validated against their logical correctness.
 * @return returns non zero if settings are valid, returns 0 otherwise.
 */
int recorder_settings_validate(struct recorder_settings const * p_settings);

/*!
 * @brief Compares if two settings are equal.
 * @details The compare and swap methods are so commonly used that they have been abstracted with this
 * @code
 * if (sender_settings_compare(p_left, p_righ))
 * {
 *      printf("Objects %p and %p are equal.\n", p_left, p_right);
 * }
 * else
 * {
 *      printf("Objects %p and %p are different.\n", p_left, p_right);
 * }
 * @endcode
 * @param[in,out] p_left left hand element of the comparison.
 * @param[in,out] p_right right hand element of the comparison.
 * @return Returns non zero if settings are equal, returns 0 otherwise.
 */
int recorder_settings_compare(struct recorder_settings const * p_left, struct recorder_settings const * p_right);

/*!
 * @brief Validates if the sender settings are correct.
 * @param[in,out] p_dest target of the copy. to this sturcture settings will be written.
 * @param[in,out] p_source copy source. From this sturcture settings will be read.
 */
void recorder_settings_copy(struct recorder_settings * p_dest, struct recorder_settings const * p_source);

/*!
 * @brief Exchanges the settings between each other.
 * @details After the exchange, the p_left' will have settings of p_right, wherease p_right' will have settings of p_left.
 * Can safely be called on the same object pointed by both parameters.
 * @param[in,out] p_left first object to swap data with.
 * @param[in,out] p_right second object to swap data with.
 * @return returns non-zero on success, 0 otherwise.
 * @attention The return value is negation of this you would get from calling memcpy().
 */
void recorder_settings_swap(struct recorder_settings * p_left, struct recorder_settings * p_right);

#if defined __cplusplus
}
#endif

#endif /*define RECORDER_SETTINGS_H_8D4D6871_B617_4BD2_AD4C_F40D03A213BF */

