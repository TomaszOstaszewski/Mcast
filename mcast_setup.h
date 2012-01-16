/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file mcast_setup.h
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
 * @date 04-Jan-2012
 * @brief
 * @details
 */
#if !defined MCAST_SETUP_H_870702C0_B65B_4828_949C_490704388A28
#define MCAST_SETUP_H_870702C0_B65B_4828_949C_490704388A28

#include <stddef.h>
#include <Windows.h>
#include <Winsock2.h>
#include <ws2tcpip.h>
#include "mcast-settings.h"

struct addrinfo;
struct mcast_settings;

/*!
 * @brief Describes the MCAST connection.
 */
struct mcast_connection {
	struct addrinfo * bindAddr_; /*!< */
	struct addrinfo * resolveAddr_; /*!< */
	struct addrinfo * multiAddr_; /*!< */
	SOCKET socket_; /*!< */
};

/*!
 * @brief Setup the multicast connection with given parameters.
 * @param[in] bConnect
 * @param[in] bReuseAddr
 * @param[in] bindAddr
 * @param[in] interfaceAddr
 * @param[in] nTTL
 * @param[in] p_multicast_addr IPv4 of the multicast group to connect, i.e. "224.5.6.7"
 * @param[in] p_port  port number on which data will be send/received.
 * @param[out] p_mcast_conn this memory location will be written with active multicast connection upon successful exit.
 * @return returns 0 on success, <>0 otherwise.
 */
int setup_multicast(BOOL bConnect, BOOL bReuseAddr, char * bindAddr, char * interfaceAddr, uint8_t nTTL, char * p_multicast_addr, char * p_port, struct mcast_connection * p_mcast_conn);

/*!
 * @brief Setup the multicast connection with given parameters.
 * @param[in] bConnect
 * @param[in] bReuseAddr
 * @param[in] bindAddr
 * @param[in] interfaceAddr
 * @param[in] nTTL
 * @param[in] p_in_addr
 * @param[out] p_mcast_conn this memory location will be written with active multicast connection upon successful exit.
 * @return returns 0 on success, <>0 otherwise.
 */
int setup_multicast_addr(BOOL bConnect, BOOL bReuseAddr, char * bindAddr, char * interfaceAddr, uint8_t nTTL, struct sockaddr_in const * p_in_addr, struct mcast_connection * p_mcast_conn);

/*!
 * @brief Wrapper for the setup_multicast function with most of the parameters set to so called "reasonable defaults".
 * @details The so called "reasonable defaults" are:
 * \li bConnect set to FALSE - don't call connect() after joining a multicast group;
 * \li bReuseAddr set to TRUE so the address can be reused immediatelly without relaying on Windows to free that after some period of time;
 * \li bindAddr set to NULL to bind to any address
 * \li interfaceAddr set to NULL to bind to any interface
 * \li nTTL set to value 8
 * @param[in] p_multicast_addr IPv4 of the multicast group to connect, i.e. "224.5.6.7"
 * @param[in] p_port  port number on which data will be send/received.
 * @param[out] p_mcast_conn this memory location will be written with active multicast connection upon successful exit.
 * @return returns 0 on success, <>0 otherwise.
 * @sa setup_multicast
 */
int setup_multicast_default(char * p_multicast_addr, char * p_port, struct mcast_connection * p_mcast_conn);

/*!
 * @brief Setup the multicast connection with given parameters.
 * @param[in] p_settings contains all the multicast connection related settings.
 * @param[out] p_conn this memory location will be written with active multicast connection upon successful exit.
 * @return returns 0 on success, <>0 otherwise.
 */
int setup_multicast_indirect(struct mcast_settings const * p_settings, struct mcast_connection * p_conn);

/*!
 * 
 */
size_t mcast_sendto_flags(struct mcast_connection * p_conn, void const * p_data, size_t data_size, int flags);

/*!
 * 
 */
size_t mcast_sendto(struct mcast_connection * p_conn, void const * p_data, size_t data_size);

/*!
 * @brief Leaves the multicast group, closes socket.
 * @param p_mcast_conn descriptor of the multicast connection obtained via setup_multicast_indirect, setup_mutlicast_default or setup_multicast call.
 * @return returns 0 on success, <>0 otherwise
 */
int close_multicast(struct mcast_connection * p_mcast_conn);

#endif /*MCAST_SETUP_H_870702C0_B65B_4828_949C_490704388A28 */
