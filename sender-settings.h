/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @brief Declares data structure that define the multicast sender settings. Also declares operations on those.
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

/*!
 * @brief Forward declaration.
 */
struct master_riff_chunk;

/*!
 * @brief Describes the sender settings.
 */
struct sender_settings {
	struct mcast_settings mcast_settings_; /*!< Describes the multicast connection settings. */
    /*!
     * @brief Describes how much data to send in one chunk. 
     * @details This member contains how many milliseconds of WAV file will be send in a single packet. 
     */
	uint16_t chunk_size_ms_;
	struct master_riff_chunk * chunk_; /*!< Describes which WAV file to send. */
};

/*!
 * @brief Returns the default settings.
 * @param[in] p_settings structure referenced by this pointer will be written with default settings.
 * @return returns 0 on success, <>0 otherwise.
 */
int get_default_settings(struct sender_settings * p_settings);

/*!
 * @brief Validates if the sender settings are correct.
 * @param[in] p_settings describes settings to be validated against their logical correctness.
 * @return returns non zero if settings are valid, returns 0 otherwise.
 */
int sender_settings_validate(struct sender_settings const * p_settings);

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
int sender_settings_compare(struct sender_settings const * p_left, struct sender_settings const * p_right);

/*!
 * @brief Validates if the sender settings are correct.
 * @param[in,out] p_dest target of the copy. to this sturcture settings will be written.
 * @param[in,out] p_source copy source. From this sturcture settings will be read.
 */
void sender_settings_copy(struct sender_settings * p_dest, struct sender_settings const * p_source);

/*!
 * @brief Exchanges the settings between each other.
 * @details After the exchange, the p_left' will have settings of p_right, wherease p_right' will have settings of p_left.
 * Can safely be called on the same object pointed by both parameters.
 * @param[in,out] p_left first object to swap data with.
 * @param[in,out] p_right second object to swap data with.
 * @return returns non-zero on success, 0 otherwise.
 * @attention The return value is negation of this you would get from calling memcpy().
 */
void sender_settings_swap(struct sender_settings * p_left, struct sender_settings * p_right);

/*!
 * @brief Returns how many bytes it would take to fit the sample that length is described inside the settings object.
 * @param[in] p_settings pointer to the settings object.
 * @return returns number of bytes it takes to fit the sample that length is described inside the settings object.
 */
uint32_t sender_settings_get_chunk_size_bytes(struct sender_settings const * p_settings);

/*!
 * @brief Converts bytes taken by the sample buffers to the time it takes to play this buffer.
 * @param[in] p_settings pointer to the settings object.
 * @param[in] bytes size of the samples buffer, in  bytes.
 * @return returns number of milliseconds it takes to play the buffer of bytes long, given the settings present in p_settings. 
 */
uint32_t sender_settings_convert_bytes_to_ms(struct sender_settings const * p_settings, uint16_t bytes);

#if defined __cplusplus
}
#endif

#endif /* MCAST_SENDER_SETTINGS_C03B81E9_5988_44C7_B40B_CCEE3B74E961 */

