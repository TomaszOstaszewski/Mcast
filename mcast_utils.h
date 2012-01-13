/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file mcast_utils.h
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
#if !defined MCAST_UTILS_H_8DE850FE_74BA_4195_A769_0D9C94034663
#define MCAST_UTILS_H_8DE850FE_74BA_4195_A769_0D9C94034663

#include "winsock_adapter.h"

//
// Function: JoinMulticastGroup
// 
// Description:
//    This function joins the multicast socket on the specified multicast 
//    group. The structures for IPv4 and IPv6 multicast joins are slightly
//    different which requires different handlers. For IPv6 the scope-ID 
//    (interface index) is specified for the local interface whereas for IPv4
//    the actual IPv4 address of the interface is given.
//
int JoinMulticastGroup(SOCKET s, struct addrinfo *group, struct addrinfo *iface);

//
// Function: SetSendInterface
//
// Description:
//    This routine sets the send (outgoing) interface of the socket.
//    Again, for v4 the IP address is used to specify the interface while
//    for v6 its the scope-ID.
//
int SetSendInterface(SOCKET s, struct addrinfo *iface);

//
// Function: SetMulticastTtl
//
// Description:
//    This routine sets the multicast TTL value for the socket.
//
int SetMulticastTtl(SOCKET s, int af, int ttl);

//
// Function: SetMulticastLoopBack
//
// Description:
//    This function enabled or disables multicast loopback. If loopback is enabled
//    (and the socket is a member of the destination multicast group) then the
//    data will be placed in the receive queue for the socket such that if a
//    receive is posted on the socket its own data will be read. For this sample
//    it doesn't really matter as if invoked as the sender, no data is read.
//
int SetMulticastLoopBack(SOCKET s, int af, int loopval);

#endif /* MCAST_UTILS_H_8DE850FE_74BA_4195_A769_0D9C94034663 */
