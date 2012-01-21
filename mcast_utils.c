/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file mcast_utils.c
 * @author T. Ostaszewski
 * @date 04-Jan-2012
 * @brief 
 * @details 
 */

#include "pcc.h"
#include "mcast_utils.h"
#include "resolve.h"
#include "debug_helpers.h"

/**
 * @brief
 * This function joins the multicast socket on the specified multicast 
 * group.
 * @details The structures for IPv4 and IPv6 multicast joins are slightly
 * different which requires different handlers. For IPv6 the scope-ID 
 * (interface index) is specified for the local interface whereas for IPv4
 * the actual IPv4 address of the interface is given.
 * @param[in] s socket that shall join mulitcast group
 * @param[in] group multicast group address
 * @param[in] iface interface to be used for multicast communication.
 * @return returns 0 on success, SOCKET_ERROR otherwise
 */
int JoinMulticastGroup(SOCKET s, struct addrinfo *group, struct addrinfo *iface)
{
    struct ip_mreq   mreqv4;
    struct ipv6_mreq mreqv6;
    char            *optval=NULL;
    int              optlevel = 0,
                     option = 0,
                     optlen = 0,
                     rc;

    rc = NO_ERROR;
    if (group->ai_family == AF_INET)
    {
        // Setup the v4 option values and ip_mreq structure
        optlevel = IPPROTO_IP;
        option   = IP_ADD_MEMBERSHIP;
        optval   = (char *)& mreqv4;
        optlen   = sizeof(mreqv4);

        mreqv4.imr_multiaddr.s_addr = ((SOCKADDR_IN *)group->ai_addr)->sin_addr.s_addr;
        mreqv4.imr_interface.s_addr = ((SOCKADDR_IN *)iface->ai_addr)->sin_addr.s_addr;

    }
    else if (group->ai_family == AF_INET6)
    {
        // Setup the v6 option values and ipv6_mreq structure
        optlevel = IPPROTO_IPV6;
        option   = IPV6_ADD_MEMBERSHIP;
        optval   = (char *) &mreqv6;
        optlen   = sizeof(mreqv6);

        mreqv6.ipv6mr_multiaddr = ((SOCKADDR_IN6 *)group->ai_addr)->sin6_addr;
        mreqv6.ipv6mr_interface = ((SOCKADDR_IN6 *)iface->ai_addr)->sin6_scope_id;
    }
    else
    {
        debug_outputln("%s %4.4u : %d\n", group->ai_family);
        rc = SOCKET_ERROR;
    }
    if (rc != SOCKET_ERROR)
    {
        // Join the group
        rc = setsockopt(
                s, 
                optlevel, 
                option,
                optval,
                optlen
                );
        if (rc == SOCKET_ERROR)
        {
            debug_outputln("%s %4.4u : %d\n", WSAGetLastError());
        }
    }
    return rc;
}

/**
 * @brief This routine sets the send (outgoing) interface of the socket.
 * @details Again, for v4 the IP address is used to specify the interface while
 * for v6 its the scope-ID.
 * @param s socket to set the interface.
 * @param iface address of the interface.
 * @return returns 0 on success, SOCKET_ERROR otherwise
 */
int SetSendInterface(SOCKET s, struct addrinfo *iface)
{
    char *optval=NULL;
    int   optlevel = 0,
          option = 0,
          optlen = 0,
          rc;

    rc = NO_ERROR;
    if (iface->ai_family == AF_INET)
    {
        // Setup the v4 option values
        optlevel = IPPROTO_IP;
        option   = IP_MULTICAST_IF;
        optval   = (char *) &((SOCKADDR_IN *)iface->ai_addr)->sin_addr.s_addr;
        optlen   = sizeof(((SOCKADDR_IN *)iface->ai_addr)->sin_addr.s_addr);
    }
    else if (iface->ai_family == AF_INET6)
    {
        // Setup the v6 option values
        optlevel = IPPROTO_IPV6;
        option   = IPV6_MULTICAST_IF;
        optval   = (char *) &((SOCKADDR_IN6 *)iface->ai_addr)->sin6_scope_id;
        optlen   = sizeof(((SOCKADDR_IN6 *)iface->ai_addr)->sin6_scope_id);
    }
    else
    {
        debug_outputln("%s %4.4u : %d\n", iface->ai_family);
        rc = SOCKET_ERROR;
    }

    // Set send IF
    if (rc != SOCKET_ERROR)
    {
        // Set the send interface
        rc = setsockopt(
                s, 
                optlevel, 
                option,
                optval,
                optlen
                       );
        if (rc == SOCKET_ERROR)
        {
            debug_outputln("%s %4.4u : %d\n", WSAGetLastError());
        }
    }
    return rc;
}

/**
 * @brief This routine sets the multicast TTL value for the socket.
 * @param[in] s - socket for which TTL is to be set.
 * @param[in] af - Address family.
 * @param[in] ttl - TTL value to be set.
 * @return returns 0 on success, SOCKET_ERROR otherwise
 */
int SetMulticastTtl(SOCKET s, int af, int ttl)
{
    char *optval=NULL;
    int   optlevel = 0,
          option = 0,
          optlen = 0,
          rc;

    rc = NO_ERROR;
    if (af == AF_INET)
    {
        // Set the options for V4
        optlevel = IPPROTO_IP;
        option   = IP_MULTICAST_TTL;
        optval   = (char *) &ttl;
        optlen   = sizeof(ttl);
    }
    else if (af == AF_INET6)
    {
        // Set the options for V6
        optlevel = IPPROTO_IPV6;
        option   = IPV6_MULTICAST_HOPS;
        optval   = (char *) &ttl;
        optlen   = sizeof(ttl);
    }
    else
    {
        debug_outputln("%s %4.4u : %d\n", af);
        rc = SOCKET_ERROR;
    }
    if (rc != SOCKET_ERROR)
    {
        // Set the TTL value
        rc = setsockopt(
                s, 
                optlevel, 
                option,
                optval, 
                optlen
                );
        if (rc == SOCKET_ERROR)
        {
            debug_outputln("%s %4.4u : %d\n", WSAGetLastError());
        }
    }
    return rc;
}

/**
 * @brief This function enabled or disables multicast loopback. 
 * @details If loopback is enabled (and the socket is a member of the destination multicast group) then the
 * data will be placed in the receive queue for the socket such that if a
 * receive is posted on the socket its own data will be read. For this sample
 * it doesn't really matter as if invoked as the sender, no data is read.
 * @return returns 0 on success, SOCKET_ERROR otherwise
 */
int SetMulticastLoopBack(SOCKET s, int af, int loopval)
{
    char *optval=NULL;
    int   optlevel = 0,
          option = 0,
          optlen = 0,
          rc;

    rc = NO_ERROR;
    if (af == AF_INET)
    {
        // Set the v4 options
        optlevel = IPPROTO_IP;
        option   = IP_MULTICAST_LOOP;
        optval   = (char *) &loopval;
        optlen   = sizeof(loopval);
    }
    else if (af == AF_INET6)
    {
        // Set the v6 options
        optlevel = IPPROTO_IPV6;
        option   = IPV6_MULTICAST_LOOP;
        optval   = (char *) &loopval;
        optlen   = sizeof(loopval);
    }
    else
    {
        debug_outputln("%s %4.4u : %d\n", af);
        rc = SOCKET_ERROR;
    }
    if (rc != SOCKET_ERROR)
    {
        // Set the multpoint loopback
        rc = setsockopt(
                s, 
                optlevel, 
                option,
                optval, 
                optlen
                );
        if (rc == SOCKET_ERROR)
        {
            debug_outputln("%s %4.4u : %d\n", WSAGetLastError());
        }
    }
    return rc;
}

