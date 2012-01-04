/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file resolve.h
 * @author T.Ostaszewski
 * @date 04-Jan-2011
 * @brief This file contains function prototypes for resolve.cpp
 * @details These are common routines for resolving and printing IPv4 and IPv6   addresses.
 */

#ifndef _RESOLVE_H_
#define _RESOLVE_H_

#ifdef _cplusplus
extern "C" {
#endif

int              PrintAddress(SOCKADDR const *sa, int salen);
int              FormatAddress(SOCKADDR *sa, int salen, char *addrbuf, int addrbuflen);
int              ReverseLookup(SOCKADDR *sa, int salen, char *namebuf, int namebuflen);
struct addrinfo *ResolveAddress(char *addr, char *port, int af, int type, int proto);

#ifdef _cplusplus
}
#endif

#endif
