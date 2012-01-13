/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file resolve.c
 * @author T. Ostaszewski
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
 * @brief Common routines for resolving addresses and hostnames
 * @details This file contains common name resolution and name printing functions.
 */

#include "pcc.h"
#include "resolve.h"

/**
 * @brief Length of the output buffer.
 */
#define OUTPUT_BUFFER_LEN (256)

/**
 * @brief Buffer for debug output to be send via OutputDebugString.
 * @details This buffer will be formatted via format string prior sending via OutputDebugString
 */
static __declspec(thread) TCHAR outputBuffer[OUTPUT_BUFFER_LEN];

/**
 * @brief Common routines for resolving addresses and hostnames
 * @details This routine takes a SOCKADDR structure and its lenght and prints converts it to a string representation. 
 * @date 04-Jan-2012
 */
static void debug_output(LPCTSTR formatString, ...)
{
    va_list args;
    HRESULT hr;
    va_start(args, formatString);
    hr = StringCchVPrintf(outputBuffer, OUTPUT_BUFFER_LEN, formatString, args);
    if (SUCCEEDED(hr))
        OutputDebugString(outputBuffer);
}

/**
 * @brief Common routines for resolving addresses and hostnames
 * @details This routine takes a SOCKADDR structure and its lenght and prints converts it to a string representation. 
 * @date 04-Jan-2012
 */
int PrintAddress(SOCKADDR const*sa, int salen)
{
    char    host[NI_MAXHOST],
            serv[NI_MAXSERV];
    int     hostlen = NI_MAXHOST,
            servlen = NI_MAXSERV,
            rc;
    // Validate argument
    if (sa == NULL)
        return WSAEFAULT;
    rc = getnameinfo(
            sa,
            salen,
            host,
            hostlen,
            serv,
            servlen,
            NI_NUMERICHOST | NI_NUMERICSERV
            );
    if (rc != 0)
    {
        //fprintf(stderr, "%s: getnameinfo failed: %d\n", __FILE__, rc);
        return rc;
    }

    // If the port is zero then don't print it
    if (strncmp(serv, "0", 1) != 0)
    {
        if (sa->sa_family == AF_INET6)
		{
            debug_output("[%s]:%s", host, serv);
		}
        else
		{
            debug_output("%s:%s", host, serv);
		}
    }
    else
        debug_output("%s", host);

    return NO_ERROR;
}

/**
 * @brief Common routines for resolving addresses and hostnames
 * @details This is similar to the PrintAddress function except that instead of printing the string address to the console, it is formatted into the supplied string buffer.
 */
int FormatAddress(SOCKADDR *sa, int salen, char *addrbuf, int addrbuflen)
{
    char    host[NI_MAXHOST],
            serv[NI_MAXSERV];
    int     hostlen = NI_MAXHOST,
            servlen = NI_MAXSERV,
            rc;

    // Validate input
    if ((sa == NULL) || (addrbuf == NULL))
        return WSAEFAULT;

    // Format the name
    rc = getnameinfo(
            sa,
            salen,
            host,
            hostlen,
            serv,
            servlen,
            NI_NUMERICHOST | NI_NUMERICSERV     // Convert to numeric representation
            );
    if (rc != 0)
    {
        //fprintf(stderr, "%s: getnameinfo failed: %d\n", __FILE__, rc);
        return rc;
    }
    if ( (strlen(host) + strlen(serv) + 1) > (unsigned)addrbuflen)
        return WSAEFAULT;
    if (strncmp(serv, "0", 1) != 0)
    {
        if (sa->sa_family == AF_INET)
		{
            //_snprintf_s(addrbuf,addrbuflen,addrbuflen-1,"%s:%s",host,serv);
		}
        else if (sa->sa_family == AF_INET6)
		{
            //_snprintf_s(addrbuf, addrbuflen,addrbuflen-1, "[%s]:%s", host, serv);
		}
        else
            addrbuf[0] = '\0';
    }
    else
    {
        //_snprintf_s(addrbuf, addrbuflen,addrbuflen-1, "%s", host);
    }

    return NO_ERROR;
}

/**
 * @brief Common routines for resolving addresses and hostnames
 * @details This routine resolves the specified address and returns a list of addrinfo
 * structure containing SOCKADDR structures representing the resolved addresses.
 * Note that if 'addr' is non-NULL, then getaddrinfo will resolve it whether
 * it is a string listeral address or a hostname.
 */
struct addrinfo *ResolveAddress(char *addr, char *port, int af, int type, int proto)
{
    struct addrinfo hints,
    *res = NULL;
    int             rc;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags  = ((addr) ? 0 : AI_PASSIVE);
    hints.ai_family = af; //((addr) ? AF_UNSPEC : af);
    hints.ai_socktype = type;
    hints.ai_protocol = proto;

    rc = getaddrinfo(
            addr,
            port,
           &hints,
           &res
            );
    if (rc != 0)
    {
        return NULL;
    }
    return res;
}

/**
 * @brief Common routines for resolving addresses and hostnames
 * @details This routine takes a SOCKADDR and does a reverse 
 * lookup for the name corresponding to that address.
 */
int ReverseLookup(SOCKADDR *sa, int salen, char *buf, int buflen)
{
    char    host[NI_MAXHOST];
    int     hostlen=NI_MAXHOST,
            rc;
    // Validate parameters
    if ((sa == NULL) || (buf == NULL))
        return WSAEFAULT;

    rc = getnameinfo(
            sa,
            salen,
            host,
            hostlen,
            NULL,
            0,
            0
            );
    if (rc != 0)
    {
        //fprintf(stderr, "getnameinfo failed: %d\n", rc);
        return rc;
    }

    //strncpy_s(buf,buflen,host,buflen-1);
    return NO_ERROR;
}

