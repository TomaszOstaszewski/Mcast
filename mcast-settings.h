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

#include "winsock_adapter.h"

/*!
 * @brief Configuration of the multicast connection.
 */
struct mcast_settings {
	int bConnect_; /*!< Whether or not call connect() on the socket bound to the multicast group */
	int bReuseAddr_; /*!< Whether or not reuse address.  */
    char * bindAddr_; /*!< Name of the interface to bind to */
    char * interface_; /*!< Name of the interface to bind to */
	int nTTL_; /*!< The 'Time To Live' parameter to set on the socket. */
	struct sockaddr_in mcast_addr_; /*!< Internet address and port of the multicast group */
};

struct mcast_settings const * get_default_mcast_settings(void);

#if defined __cplusplus
}
#endif

#endif /* MCAST_SETTINGS_4220923E_A4E4_4081_8291_4D05B9D799B1 */

