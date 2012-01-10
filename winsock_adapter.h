/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file winsock_adapter.h
 * @author T.Ostaszewski
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
