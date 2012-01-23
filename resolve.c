/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file resolve.c
 * @brief Common routines for resolving addresses and hostnames
 * @details This file contains common name resolution and name printing functions.
 * @author T. Ostaszewski
 * @date 04-Jan-2012
 */

#include "pcc.h"
#include "resolve.h"
#include "debug_helpers.h"

int FormatAddress(struct sockaddr *sa, int salen, char *addrbuf, int addrbuflen)
{
    HRESULT hr;
    char    host[NI_MAXHOST],
            serv[NI_MAXSERV];
    int     hostlen = NI_MAXHOST,
            servlen = NI_MAXSERV,
            rc;

    /* Validate input */
    if ((sa == NULL) || (addrbuf == NULL))
        return WSAEFAULT;
    /* Format the name */
    rc = getnameinfo(sa, salen, host, hostlen, serv, servlen, NI_NUMERICHOST | NI_NUMERICSERV /* Convert to numeric representation */);
    if (rc != 0)
    {
        debug_outputln("%s %u : %d", __FILE__, __LINE__, rc);
        return rc;
    }
    if ( (strlen(host) + strlen(serv) + 1) > (unsigned)addrbuflen)
        return WSAEFAULT;
    if (strncmp(serv, "0", 1) != 0)
    {
        if (sa->sa_family == AF_INET)
        {
            hr = StringCchPrintf(addrbuf, addrbuflen, "%s:%s", host, serv);
        }
        else if (sa->sa_family == AF_INET6)
        {
            hr = StringCchPrintf(addrbuf, addrbuflen, "[%s]:%s", host, serv);
        }
        else
            addrbuf[0] = '\0';
    }
    else
    {
        hr = StringCchPrintf(addrbuf, addrbuflen, "%s", host);
    }
    return S_OK == hr ? NO_ERROR : -1;
}

static struct addrinfo *sResolveAddress(char *addr, char *port, int af, int type, int proto, int flags)
{
    struct addrinfo hints,
                    *res = NULL;
    int             rc;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags  = flags;
    hints.ai_family = af;
    hints.ai_socktype = type;
    hints.ai_protocol = proto;

    rc = getaddrinfo(addr, port, &hints, &res);
    if (rc != 0)
    {
        return NULL;
    }
    return res;
}

struct addrinfo *ResolveAddress(char *addr, char *port, int af, int type, int proto)
{
    return sResolveAddress(addr, port, af, type, port, (addr) ? 0 : AI_PASSIVE);
}

struct addrinfo *resolve_address_ipv4(struct sockaddr_in const * p_in_addr, int type, int proto, int flags)
{
    char    host[NI_MAXHOST],
            port[NI_MAXSERV];
    struct addrinfo hints;
    struct addrinfo *res = NULL;
    int rc;
    unsigned long ipv4addr = ntohl(p_in_addr->sin_addr.s_addr);
    ipv4addr = ntohl(p_in_addr->sin_addr.s_addr);
    sprintf(host, "%hhu.%hhu.%hhu.%hhu", 
            (unsigned char )((ipv4addr>>24) & 0xff),
            (unsigned char )((ipv4addr>>16) & 0xff),
            (unsigned char )((ipv4addr>>8) & 0xff),
            (unsigned char )((ipv4addr) & 0xff)); 
    sprintf(port, "%hu", ntohs(p_in_addr->sin_port));
    return sResolveAddress(host, port, AF_INET, type, proto, flags);
}

/**
 * @brief Common routines for resolving addresses and hostnames
 * @details This routine takes a SOCKADDR and does a reverse 
 * lookup for the name corresponding to that address.
 * @param[in] sa address of the SOCKADDR for which the lookup is to be made.
 * @param[in] salen length of the structure given as sa parameter
 * @param[in] buf buffer to which address found will be written.
 * @param[in] buflen lenght of the buffer given as buf parameter
 * @return returns 0 on success, <>0 otherwise.
 */
int ReverseLookup(SOCKADDR *sa, int salen, char *buf, int buflen)
{
    char    host[NI_MAXHOST];
    int     hostlen=NI_MAXHOST,
            rc;
    /* Validate parameters */
    if ((sa == NULL) || (buf == NULL))
        return WSAEFAULT;

    rc = getnameinfo(sa, salen, host, hostlen, NULL, 0, 0);
    if (rc != 0)
    {
        debug_outputln("%s %u : %d", __FILE__, __LINE__, rc);
        return rc;
    }
    StringCchPrintf(buf, buflen, "%s", host);
    return NO_ERROR;
}

