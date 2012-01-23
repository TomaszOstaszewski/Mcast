/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @brief Describes the multicast settings.
 * @file mcast-settings.h
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
#if !defined MCAST_SETTINGS_4220923E_A4E4_4081_8291_4D05B9D799B1
#define MCAST_SETTINGS_4220923E_A4E4_4081_8291_4D05B9D799B1

#if defined __cplusplus
extern "C" {
#endif

#include <winsock2.h>

/*!
 * @brief Configuration of the multicast connection.
 */
struct mcast_settings {
	int bConnect_; /*!< Whether or not call connect() on the socket bound to the multicast group */
	int bReuseAddr_; /*!< Whether or not reuse address.  */
    char * bindAddr_; /*!< Name of the interface to bind to */
    char * interface_; /*!< Name of the interface to bind to */
	int nTTL_; /*!< The 'Time To Live' parameter to set on the socket. */
	struct sockaddr_in mcast_addr_; /*!< IPv4 address and port of the multicast group */
};

/*!
 * @brief Returns default multicast settings. 
 * @param[in,out] p_target pointer to the settings structure. This structure will be written with the default settings.
 * @return returns non-zero on success, 0 otherwise.
 */
int mcast_settings_get_default(struct mcast_settings * p_target);

/*!
 * @brief Validates, if settings are correct.
 * @details Invalid multicast settings are for example those, which has a non multicast IPv4 address.
 * @param[in] p_settings settings to be check for validity. 
 * @return returns non-zero on success, 0 otherwise.
 */
int mcast_settings_validate(struct mcast_settings const * p_settings);

/*!
 * @brief Copies the settings from one into another.
 * @details After the copy, the p_dest will have settings of p_source. The p_source remains unaltered.
 * @param[in,out] p_dest target of the copy. to this sturcture settings will be written.
 * @param[in,out] p_source copy source. From this sturcture settings will be read.
 */
void mcast_settings_copy(struct mcast_settings * p_dest, struct mcast_settings const * p_source);

/*!
 * @brief Exchanges the settings between each other.
 * @details After the exchange, the p_left' will have settings of p_right, wherease p_right' will have settings of p_left.
 * @param[in,out] p_left first object to swap data with.
 * @param[in,out] p_right second object to swap data with.
 * @return returns non-zero on success, 0 otherwise.
 */
void mcast_settings_swap(struct mcast_settings * p_left, struct mcast_settings * p_right);

/*!
 * @brief Compares if two settings are equal.
 * @details The compare and swap methods are so commonly used that they have been abstracted with this
 * @code
 * if (mcast_settings_compare(p_left, p_righ))
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
 * @attention The return value is negation of this you would get from calling memcpy().
 */
int mcast_settings_compare(struct mcast_settings const * p_left, struct mcast_settings const * p_right);

#if defined __cplusplus
}
#endif

#endif /* MCAST_SETTINGS_4220923E_A4E4_4081_8291_4D05B9D799B1 */

