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

#if defined WIN32

static int format_string_into_buffer(char * addrbuf, size_t addrbuflen, const char * format_string, ...)
{
    HRESULT hr; 
    int retval = -1;
    const char * p_end;
    va_list args;
    va_start(args, format_string);
	hr = StringCchVPrintfEx(addrbuf, addrbuflen, &p_end, NULL, 0, format_string, args);
    if (SUCCEEDED(hr))
    {
        retval = 0;
    }
    va_end(args);
    return retval;
}

#else

static int format_string_into_buffer(char * addrbuf, size_t addrbuflen, const char * format_string, ...)
{
    va_list args;
    va_start(args, format_string);
    vsnprintf(addrbuf, addrbuflen, format_string, args);
    va_end(args);
    return 0;     
}

#endif

int FormatAddress(struct sockaddr *sa, int salen, char *addrbuf, int addrbuflen)
{
    char    host[NI_MAXHOST],
            service[NI_MAXSERV];
    int     hostlen = NI_MAXHOST,
            servlen = NI_MAXSERV,
            rc;

    /* Validate input */
    if ((sa == NULL) || (addrbuf == NULL))
        return (-1);
    /* Format the name */
    rc = getnameinfo(sa, salen, host, hostlen, service, servlen, NI_NUMERICHOST | NI_NUMERICSERV /* Convert to numeric representation */);
    if (rc != 0)
    {
        debug_outputln("%s %u : %d", __FILE__, __LINE__, rc);
        return rc;
    }
    if ( (strlen(host) + strlen(service) + 1) > (unsigned)addrbuflen)
        return (-1);
    if (strncmp(service, "0", 1) != 0)
    {
        if (sa->sa_family == AF_INET)
        {
            rc = format_string_into_buffer(addrbuf, addrbuflen, "%s:%s", host, service);
        }
        else if (sa->sa_family == AF_INET6)
        {
            rc = format_string_into_buffer(addrbuf, addrbuflen, "%s:%s", host, service);
        }
        else
            addrbuf[0] = '\0';
    }
    else
    {
        rc = format_string_into_buffer(addrbuf, addrbuflen, "%s", host);
    }
    return rc;
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

#if 0
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
#endif

