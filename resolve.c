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
    int flags = 0;
    if (NULL != addr)
        flags = AI_PASSIVE;
    return sResolveAddress(addr, port, af, type, proto, flags);
}

struct addrinfo *ResolveAddressWithFlags(char *addr, char *port, int af, int type, int proto, int flags)
{
    return sResolveAddress(addr, port, af, type, proto, flags);
}

struct addrinfo *resolve_address_ipv4(struct sockaddr_in const * p_in_addr, int type, int proto, int flags)
{
    HRESULT hr;
    TCHAR   host[NI_MAXHOST],
            port[NI_MAXSERV];
    unsigned long ipv4addr = ntohl(p_in_addr->sin_addr.s_addr);
    ipv4addr = ntohl(p_in_addr->sin_addr.s_addr);
    hr = StringCchPrintf(host, NI_MAXHOST, _T("%hhu.%hhu.%hhu.%hhu"), 
            (unsigned char )((ipv4addr>>24) & 0xff),
            (unsigned char )((ipv4addr>>16) & 0xff),
            (unsigned char )((ipv4addr>>8) & 0xff),
            (unsigned char )((ipv4addr) & 0xff)); 
    if (SUCCEEDED(hr))
    {
        hr = StringCchPrintf(port, NI_MAXSERV, _T("%hu"), ntohs(p_in_addr->sin_port));
        if (SUCCEEDED(hr))
        {
            return sResolveAddress(host, port, AF_INET, type, proto, flags);
        }
    }
    return NULL;
}

