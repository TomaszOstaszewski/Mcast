/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file mcast_utils.h
 * @brief
 * @details
 * @date 04-Jan-2012
 */
#if !defined MCAST_UTILS_H_8DE850FE_74BA_4195_A769_0D9C94034663
#define MCAST_UTILS_H_8DE850FE_74BA_4195_A769_0D9C94034663

#include "platform-sockets.h"
#include "std-int.h"
#if defined __cplusplus
extern "C" {
#endif

/*!
 * @brief Multicast registration and Time-To-Live setup
 * @details This function joins the multicast socket on the specified multicast 
 * group. The structures for IPv4 and IPv6 multicast joins are slightly
 * different which requires different handlers. For IPv6 the scope-ID 
 * (interface index) is specified for the local interface whereas for IPv4
 * the actual IPv4 address of the interface is given.
 * @param[in] s socket for which the outgoing interface is to be set.
 * @param[in] group - multicast group address.
 * @param[in] iface - network interface address. This network interface wil be used for multicast group communication.
 * @param[in] ttl - TTL value to be set.
 */
int join_mcast_group_set_ttl(SOCKET s, struct addrinfo const * group, struct addrinfo const * iface, int ttl);

/*!
 * @details This function joins the multicast socket on the specified multicast 
 * group. The structures for IPv4 and IPv6 multicast joins are slightly
 * different which requires different handlers. For IPv6 the scope-ID 
 * (interface index) is specified for the local interface whereas for IPv4
 * the actual IPv4 address of the interface is given.
 * @param[in] s socket that shall join mulitcast group
 * @param[in] group multicast group address.
 * @param[in] iface network interface address. This network interface wil be used for multicast group communication.
 * @return
 */
int JoinMulticastGroup(SOCKET s, struct addrinfo const *group, struct addrinfo const *iface);

/*!
 * @brief
 * @details This routine sets the send (outgoing) interface of the socket.
 * Again, for v4 the IP address is used to specify the interface while
 * for v6 its the scope-ID.
 * @param[in] s socket for which the outgoing interface is to be set.
 * @param[in] iface address of the outgoing interface.
 * @return
 */
 int SetSendInterface(SOCKET s, struct addrinfo const *iface);

/*!
 * @brief This routine sets the multicast TTL value for the socket.
 * @param[in] s socket for which TTL is to be set.
 * @param[in] af address family, either <b>AF_INET</b> or <b>AF_INET6</b>
 * @param[in] ttl - TTL value to be set.
 * @return
 */
int SetMulticastTtl(SOCKET s, int af, uint8_t ttl);

/*!
 * @brief Enables or disables multicast loopback.
 * @details This function enabled or disables multicast loopback. If loopback is enabled
 * (and the socket is a member of the destination multicast group) then the data will be placed in the 
 * receive queue for the socket such that if a receive is posted on the socket its own data will be read. 
 * @param[in] s socket on which multicast communication is done.
 * @param[in] af address family, either <b>AF_INET</b> or <b>AF_INET6</b>
 * @param[in] loopval set to 0 to disable loopback, set to non-zero to enable.
 * @return
 */
int SetMulticastLoopBack(SOCKET s, int af, int loopval);

#if defined __cplusplus
}
#endif

#endif /* MCAST_UTILS_H_8DE850FE_74BA_4195_A769_0D9C94034663 */
