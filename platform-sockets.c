#include "platform-sockets.h"
#if defined WIN32

unsigned long get_last_socket_error(void)
{
	return WSAGetLastError();
}

#else
#	include <errno.h>

unsigned long get_last_socket_error(void)
{
	return errno;
}

int closesocket(SOCKET s) 
{
	return close(s);
}

#endif

