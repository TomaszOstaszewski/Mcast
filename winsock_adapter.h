/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file winsock_adapter.h
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
 * @brief Winsock header suited for non-Windows platform.
 * @details Having this file allows one to write common code for both Windows and POSIX 
 * platforms.
 */
#if !defined WINSOCK_ADAPTER_3EC3154C_246B_4F9B_9BC1_008BB70BC774
#define WINSOCK_ADAPTER_3EC3154C_246B_4F9B_9BC1_008BB70BC774

#if !defined WIN32
/* POSIX world */
#	include <stdint.h>
#	if !defined SOCKET_TYPE
		typedef int SOCKET;
#		define SOCKET_TYPE
#	endif
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netdb.h>

/** @brief needed for Winsock stubs */
#define WSADESCRIPTION_LEN     (256)
/** @brief needed for Winsock stubs */
#define WSASYS_STATUS_LEN      (128)

typedef uint16_t WORD; /*!< needed for Winsock stubs */

typedef struct WSAData {
  WORD			 wVersion;
  WORD			 wHighVersion;
  char           szDescription[WSADESCRIPTION_LEN+1];
  char           szSystemStatus[WSASYS_STATUS_LEN+1];
  unsigned short iMaxSockets;
  unsigned short iMaxUdpDg;
  char FAR       *lpVendorInfo;
} WSADATA, *LPWSADATA;

/*!
 * @brief Winsock init stub.
 * @return returns 0.
 */
int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData);

/*!
 * @brief Winsock cleanup stub.
 * @return returns 0.
 */
int	WSACleanup(void);

#else
/* WINDOWS world */
#	include "std-int.h"
#	include <Winsock2.h>
#	include <ws2tcpip.h>
#	include <wspiapi.h>
#	include "timeofday.h"
#	if !defined timeradd
#		define timeradd(left,right,sum) do { (sum)->tv_sec = (left)->tv_sec + (right)->tv_sec; (sum)->tv_usec = (left)->tv_usec + (right)->tv_usec; if ((sum)->tv_usec >= 1000000) { ++(sum)->tv_sec; (sum)->tv_usec -= 1000000; } } while (0)
#	endif 
#endif

#endif /* WINSOCK_ADAPTER_3EC3154C_246B_4F9B_9BC1_008BB70BC774 */
