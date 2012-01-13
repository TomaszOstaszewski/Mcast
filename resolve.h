/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file resolve.h
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
struct addrinfo *ResolveAddress_2(struct sockaddr_in const * p_addr, int af, int type, int proto);

#ifdef _cplusplus
}
#endif

#endif
