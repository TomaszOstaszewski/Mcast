#include "pcc.h"

#if !defined WIN32

int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData)
{
	return 0;
}

int	WSACleanup(void)
{
}

#endif
