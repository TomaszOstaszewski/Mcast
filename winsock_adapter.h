#if !defined WINSOCK_ADAPTER_3EC3154C_246B_4F9B_9BC1_008BB70BC774
#define WINSOCK_ADAPTER_3EC3154C_246B_4F9B_9BC1_008BB70BC774

#if !defined WIN32
#include <stdint.h>
#	if !defined SOCKET_TYPE
typedef int SOCKET;
#	define SOCKET_TYPE
#	endif
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netdb.h>

#define WSADESCRIPTION_LEN      256
#define WSASYS_STATUS_LEN       128

typedef uint16_t WORD;

typedef struct WSAData {
  WORD			 wVersion;
  WORD			 wHighVersion;
  char           szDescription[WSADESCRIPTION_LEN+1];
  char           szSystemStatus[WSASYS_STATUS_LEN+1];
  unsigned short iMaxSockets;
  unsigned short iMaxUdpDg;
  char FAR       *lpVendorInfo;
} WSADATA, *LPWSADATA;

int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData);
int	WSACleanup(void);

#else
#if defined _MSC_VER && _MSC_VER < 1600
#	include "vc2003stdint.h"
#else
#	include <stdint.h>
#endif
#	include <Winsock2.h>
#	include <ws2tcpip.h>
#	include <wspiapi.h>
#	include "timeofday.h"
#	if !defined timeradd
#		define timeradd(left,right,sum) do { (sum)->tv_sec = (left)->tv_sec + (right)->tv_sec; (sum)->tv_usec = (left)->tv_usec + (right)->tv_usec; if ((sum)->tv_usec >= 1000000) { ++(sum)->tv_sec; (sum)->tv_usec -= 1000000; } } while (0)
#	endif 
#endif

#endif /* WINSOCK_ADAPTER_3EC3154C_246B_4F9B_9BC1_008BB70BC774 */
