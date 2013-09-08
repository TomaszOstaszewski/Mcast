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

int join_mcast_group_set_ttl(SOCKET s, struct addrinfo const * group, struct addrinfo const * iface, int ttl)
{
    struct ip_mreq   mreqv4;
    struct ipv6_mreq mreqv6;
    char            *optval=NULL;
    int              optlevel = 0,
                     option = 0,
                     optlen = 0,
                     rc;
    rc = NO_ERROR;
    switch (group->ai_family)
    {
        case AF_INET: /* IPv4 */
            debug_outputln("%4.4u %10.10s", __LINE__, __FILE__);
            /* Join Multicast group */
            optlevel = IPPROTO_IP;
            option   = IP_ADD_MEMBERSHIP;
            optval   = (char *)& mreqv4;
            optlen   = sizeof(mreqv4);
            mreqv4.imr_multiaddr.s_addr = ((SOCKADDR_IN *)group->ai_addr)->sin_addr.s_addr;
            mreqv4.imr_interface.s_addr = ((SOCKADDR_IN *)iface->ai_addr)->sin_addr.s_addr;
            rc = setsockopt(s, optlevel, option, (const char *)&mreqv4, optlen);
            if (SOCKET_ERROR == rc)
            {
                goto error;
            }
            /* Set send interface */
            option   = IP_MULTICAST_IF;
            optval   = (char *) &((SOCKADDR_IN *)iface->ai_addr)->sin_addr.s_addr;
            optlen   = sizeof(((SOCKADDR_IN *)iface->ai_addr)->sin_addr.s_addr);
            rc = setsockopt(s, optlevel, option, optval, optlen);
            if (SOCKET_ERROR == rc)
            {
                goto error;
            }
            /* Set outgoing TTL */
            option   = IP_MULTICAST_TTL;
            optval   = (char *) &ttl;
            optlen   = sizeof(ttl);
            rc = setsockopt(s, optlevel, option, optval, optlen);
            if (SOCKET_ERROR == rc)
            {
                goto error;
            }
            break;
        case AF_INET6: /* IPv6 */
            optlevel = IPPROTO_IPV6;
            // Setup the v6 option values and ipv6_mreq structure
            option   = IPV6_ADD_MEMBERSHIP;
            optval   = (char *) &mreqv6;
            optlen   = sizeof(mreqv6);

            mreqv6.ipv6mr_multiaddr = ((SOCKADDR_IN6 *)group->ai_addr)->sin6_addr;
            mreqv6.ipv6mr_interface = ((SOCKADDR_IN6 *)iface->ai_addr)->sin6_scope_id;
            rc = setsockopt(s, optlevel, option, (const char *)&mreqv6, optlen);
            if (SOCKET_ERROR == rc)
            {
                goto error;
            }
            // Setup the v6 option values
            option   = IPV6_MULTICAST_IF;
            optval   = (char *) &((SOCKADDR_IN6 *)iface->ai_addr)->sin6_scope_id;
            optlen   = sizeof(((SOCKADDR_IN6 *)iface->ai_addr)->sin6_scope_id);
            rc = setsockopt(s, optlevel, option, optval, optlen);
            if (SOCKET_ERROR == rc)
            {
                goto error;
            }
            // Set the options for V6
            option   = IPV6_MULTICAST_HOPS;
            optval   = (char *) &ttl;
            optlen   = sizeof(ttl);
            rc = setsockopt(s, optlevel, option, optval, optlen);
            if (SOCKET_ERROR == rc)
            {
                goto error;
            }
            break;
        default:
            rc = SOCKET_ERROR;
            break;
    }
    return rc;
error:
    debug_outputln("%4.4u %10.10s", __LINE__, __FILE__);
    return rc;
}

int JoinMulticastGroup(SOCKET s, struct addrinfo const *group, struct addrinfo const *iface)
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
        rc = setsockopt(s, optlevel, option, optval, optlen);
        if (rc == SOCKET_ERROR)
        {
            debug_outputln("%s %4.4u : %d\n", WSAGetLastError());
        }
    }
    return rc;
}

int SetSendInterface(SOCKET s, struct addrinfo const *iface)
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
        rc = setsockopt(s, optlevel, option, optval, optlen);
        if (rc == SOCKET_ERROR)
        {
            debug_outputln("%s %4.4u : %d\n", WSAGetLastError());
        }
    }
    return rc;
}

int SetMulticastTtl(SOCKET s, int af, uint8_t ub_ttl)
{
    char *optval=NULL;
    int   optlevel = 0,
          option = 0,
          optlen = 0,
          rc;
    unsigned int ttl = ub_ttl;

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
        rc = setsockopt(s, optlevel, option, optval, optlen);
        if (rc == SOCKET_ERROR)
        {
            debug_outputln("%s %4.4u : %d\n", WSAGetLastError());
        }
    }
    return rc;
}

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
        rc = setsockopt(s, optlevel, option, optval, optlen);
        if (rc == SOCKET_ERROR)
        {
            debug_outputln("%s %4.4u : %d\n", WSAGetLastError());
        }
    }
    return rc;
}

