#include "pcc.h"
#include "winsock_adapter.h"
#include "mcast_setup.h"
#include "conn_data.h"
#include "hello.h"
#include "mcast_utils.h"
#include "winsock_adapter.h"
#include "resolve.h"
#include "debug_helpers.h"

// Sample: IPv4/6 Multicasting using Setsockopt
//
// Purpose:
//    This sample illustrates IP multicasting using the Winsock 1
//    method of joining and leaving an multicast group.  This sample
//    may be invoked as either a sender or a receiver. This sample works
//    with both IPv4 and IPv6 multicasting but does not include support
//    for the IPv4 source multicasting.
//
//    One of the advantages of using the setsockopt over WSAJoinLeaf is
//    the ability to join multiple multicast groups on a single socket
//    which is not possible with WSAJoinLeaf.
//
//    Also note that because we include winsock2.h we must link with
//    ws2_32.lib and not with wsock32.lib.
//
//
// NOTE:
//   From Network Programming for Microsoft Windows, Second Edition
//   by Anthony Jones and James Ohlund.  Copyright 2002.
//   Reproduced by permission of Microsoft Press.  All rights reserved.

// This sample uses the new getaddrinfo/getnameinfo functions which are new to 
// Windows XP. To run this sample on older OSes, include the following header
// file which makes it work automagically.

typedef enum device_type { 
	BRICK_DEVICE,
	NGCH_DEVICE
} device_type_t;

#define MCASTADDRV4    "234.5.6.7"
#define MCASTADDRV6    "ff12::1"
#define MCASTPORT      "25000"
#define BUFSIZE        1024
#define DEFAULT_COUNT  500
#define DEFAULT_TTL    8

struct mcast_settings {
	BOOL bConnect_;
	BOOL bDontJoin_;
	BOOL bReuseAddr_;
	int nProtocol_;
	int nLoopback_;
	int nTTL_;
	char * bindAddr_;
	char * interface_;
	char * mcast_addr_;
	char * mcast_port_;
};

#define DEFAULT_MCAST_SETTINGS { FALSE, FALSE, TRUE, IPPROTO_UDP, 0, DEFAULT_TTL }

BOOL  
	bConnect=FALSE,           // Connect before sending?
	bDontJoin=FALSE,          // Specifies whether to join the multicast group
	bReuseAddr=TRUE;         // Set SO_REUSEADDR 
int   
	gProtocol=IPPROTO_UDP,    // UDP
	gLoopBack=0,              // Disable loopback?
	gTtl=DEFAULT_TTL;         // Multicast TTL value

device_type_t g_dev_type = BRICK_DEVICE;

char *gBindAddr=NULL,           // Address to bind socket to (default is 0.0.0.0 or ::)
	*gInterface=NULL,          // Interface to join the multicast group on
	*gMulticast=MCASTADDRV4,   // Multicast group to join
	*gPort=MCASTPORT;          // Port number to use

static const struct timeval hello_send_timer_value = { 0, 500000 };
static const struct timeval purge_entry_timer_value = { 2, 0 };
static const struct timeval new_data_timeout = { 0, 250000 };

int setup_multicast(struct mcast_connection * p_mcast_conn)
{
	SOCKET retval = (SOCKET)INVALID_HANDLE_VALUE;
	int rc;
	p_mcast_conn->multiAddr_ = ResolveAddress(gMulticast, gPort, AF_UNSPEC, SOCK_DGRAM, gProtocol);
	if (NULL == p_mcast_conn->multiAddr_)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
		goto cleanup;
	}
	// Resolve the binding address
	p_mcast_conn->bindAddr_ = ResolveAddress(gBindAddr, gPort, p_mcast_conn->multiAddr_->ai_family, p_mcast_conn->multiAddr_->ai_socktype, p_mcast_conn->multiAddr_->ai_protocol);
	if (NULL == p_mcast_conn->bindAddr_)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
		goto cleanup;
	}
	// Resolve the multicast interface
	p_mcast_conn->resolveAddr_	= ResolveAddress(gInterface, "0", p_mcast_conn->multiAddr_->ai_family, p_mcast_conn->multiAddr_->ai_socktype, p_mcast_conn->multiAddr_->ai_protocol);
	if (NULL == p_mcast_conn->multiAddr_)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
		goto cleanup;
	}
	// 
	// Create the socket. In Winsock 1 you don't need any special
	// flags to indicate multicasting.
	//
	retval = socket(p_mcast_conn->multiAddr_->ai_family, p_mcast_conn->multiAddr_->ai_socktype, p_mcast_conn->multiAddr_->ai_protocol);
	if (retval == INVALID_SOCKET)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
		goto cleanup;
	}
	//debug_outputln("%s %d : %8.8x", __FILE__, __LINE__, retval);
	if (bReuseAddr)
	{
		int optval;
		optval = 1;
		rc = setsockopt(retval, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));
		if (rc == SOCKET_ERROR)
		{
			debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
			goto cleanup;
		}
	}
	rc = bind(retval, p_mcast_conn->bindAddr_->ai_addr, (int) p_mcast_conn->bindAddr_->ai_addrlen);
	if (rc == SOCKET_ERROR)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
		goto cleanup;
	}
	{
		struct sockaddr_in local_bind = { 0 };
		socklen_t local_data_len = sizeof(local_bind);
		rc = getsockname(retval, (struct sockaddr*)&local_bind, &local_data_len);
		if (SOCKET_ERROR != rc)
		{
			debug_outputln("%s %d : %s:%u", __FILE__, __LINE__, inet_ntoa(local_bind.sin_addr), ntohs(local_bind.sin_port));
		}
		else
		{
			debug_outputln("%s %d : %u", __FILE__, __LINE__, rc);
		}
	}
	// Join the multicast group if specified
	rc = JoinMulticastGroup(retval, p_mcast_conn->multiAddr_, p_mcast_conn->resolveAddr_);
	if (rc == SOCKET_ERROR)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
		goto cleanup;
	}
	// Set the send (outgoing) interface 
	rc = SetSendInterface(retval, p_mcast_conn->resolveAddr_);
	if (rc == SOCKET_ERROR)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
		goto cleanup;
	}
	// Set the TTL to something else. The default TTL is one.
	rc = SetMulticastTtl(retval, p_mcast_conn->multiAddr_->ai_family, gTtl);
	if (rc == SOCKET_ERROR)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
		goto cleanup;
	}
	if (bConnect)
	{
		rc = connect(retval, p_mcast_conn->multiAddr_->ai_addr, (int) p_mcast_conn->multiAddr_->ai_addrlen);
		if (rc == SOCKET_ERROR)
		{
			debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
			goto cleanup;
		}
	}
	p_mcast_conn->socket_ = retval;
	return 0;
cleanup:
	return -1;
}

int setup_multicast_3(char * p_multicast_addr, char * p_port, struct mcast_connection * p_mcast_conn)
{
	int rc;
	p_mcast_conn->multiAddr_ 	= ResolveAddress(p_multicast_addr, p_port, AF_UNSPEC, SOCK_DGRAM, gProtocol);
	if (NULL == p_mcast_conn->multiAddr_)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
		goto cleanup;
	}
	// Resolve the binding address
	p_mcast_conn->bindAddr_ 	= ResolveAddress(gBindAddr, p_port, p_mcast_conn->multiAddr_->ai_family, p_mcast_conn->multiAddr_->ai_socktype, p_mcast_conn->multiAddr_->ai_protocol);
	if (NULL == p_mcast_conn->bindAddr_)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
		goto cleanup;
	}
	// Resolve the multicast interface
	p_mcast_conn->resolveAddr_	= ResolveAddress(gInterface, "0", p_mcast_conn->multiAddr_->ai_family, p_mcast_conn->multiAddr_->ai_socktype, p_mcast_conn->multiAddr_->ai_protocol);
	if (NULL == p_mcast_conn->multiAddr_)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
		goto cleanup;
	}
	// 
	// Create the socket. In Winsock 1 you don't need any special
	// flags to indicate multicasting.
	//
	p_mcast_conn->socket_ 		= socket(p_mcast_conn->multiAddr_->ai_family, p_mcast_conn->multiAddr_->ai_socktype, p_mcast_conn->multiAddr_->ai_protocol);
	if (p_mcast_conn->socket_ == INVALID_SOCKET)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
		goto cleanup;
	}
	//debug_outputln("%s %d : %8.8x", __FILE__, __LINE__, retval);
	if (bReuseAddr)
	{
		int optval;
		optval = 1;
		rc = setsockopt(p_mcast_conn->socket_, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));
		if (rc == SOCKET_ERROR)
		{
			debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
			goto cleanup;
		}
	}
	rc = bind(p_mcast_conn->socket_, p_mcast_conn->bindAddr_->ai_addr, (int) p_mcast_conn->bindAddr_->ai_addrlen);
	if (rc == SOCKET_ERROR)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
		goto cleanup;
	}
	{
		struct sockaddr_in local_bind = { 0 };
		socklen_t local_data_len = sizeof(local_bind);
		rc = getsockname(p_mcast_conn->socket_, (struct sockaddr*)&local_bind, &local_data_len);
		if (SOCKET_ERROR != rc)
		{
			debug_outputln("%s %d : %s:%u", __FILE__, __LINE__, inet_ntoa(local_bind.sin_addr), ntohs(local_bind.sin_port));
		}
		else
		{
			debug_outputln("%s %d : %u", __FILE__, __LINE__, rc);
		}
	}
	// Join the multicast group if specified
	rc = JoinMulticastGroup(p_mcast_conn->socket_, p_mcast_conn->multiAddr_, p_mcast_conn->resolveAddr_);
	if (rc == SOCKET_ERROR)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
		goto cleanup;
	}
	// Set the send (outgoing) interface 
	rc = SetSendInterface(p_mcast_conn->socket_, p_mcast_conn->resolveAddr_);
	if (rc == SOCKET_ERROR)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
		goto cleanup;
	}
	// Set the TTL to something else. The default TTL is one.
	rc = SetMulticastTtl(p_mcast_conn->socket_, p_mcast_conn->multiAddr_->ai_family, gTtl);
	if (rc == SOCKET_ERROR)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
		goto cleanup;
	}
	if (bConnect)
	{
		rc = connect(p_mcast_conn->socket_, p_mcast_conn->multiAddr_->ai_addr, (int) p_mcast_conn->multiAddr_->ai_addrlen);
		if (rc == SOCKET_ERROR)
		{
			debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
			goto cleanup;
		}
	}
	return 0;
cleanup:
	return -1;
}

struct mcast_connection * setup_multicast_2(char * p_multicast_addr, char * p_port)
{
	struct mcast_connection * p_mcast_conn;	
	int rc;
	p_mcast_conn 				= HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct mcast_connection));
	rc = setup_multicast_3(p_multicast_addr, p_port, p_mcast_conn);
	if (0 == rc)
		return p_mcast_conn;
	return NULL;
}

int close_multicast(struct mcast_connection * p_mcast_conn)
{
	if (NULL == p_mcast_conn)
		return -E_INVALIDARG;
	freeaddrinfo(p_mcast_conn->bindAddr_);
	freeaddrinfo(p_mcast_conn->resolveAddr_);
	freeaddrinfo(p_mcast_conn->multiAddr_);
	closesocket(p_mcast_conn->socket_);
	return 0;
}

int setup_mcast(struct conn_data * p_conn_data)
{
	WSADATA             wsd;
	struct addrinfo    *resmulti=NULL;
	int	rc;
	srand((unsigned int)time(NULL));
	if ((rc = WSAStartup(MAKEWORD(1, 1), &wsd)) != 0)
	{
		return -1;
	}
	rc = setup_multicast(&p_conn_data->connection_);
	if (rc <0)
	{
		return -1;
	}
	memset(&p_conn_data->my_hello_, 0, sizeof(struct hello_packet));
	p_conn_data->my_hello_.type_ = g_dev_type;
	generate_random_tei(&p_conn_data->my_hello_.tei_);
	gettimeofday(&p_conn_data->next_hello_send_timeout_, NULL);
	return 0;
}

int act_on_timer(struct conn_data * p_conn_data)
{
	size_t purged_count;
	struct timeval now;
	gettimeofday(&now, NULL);
	/* If the time comes, send hello packet again */
	if (timercmp(&now, &p_conn_data->next_hello_send_timeout_,>))
	{
		/* Here send hello packet again */
		send_hello(p_conn_data);
		/* Update timeout value */
		timeradd(&now, &hello_send_timer_value, &p_conn_data->next_hello_send_timeout_);
	}
	/* If received hello packet from elsewhere... */
	if( 0 < recv_hello(p_conn_data, &new_data_timeout) && !is_hello_mine(p_conn_data))
	{
		/* ...update neighbours list */
		const char * p_my_hello = (const char *)&p_conn_data->my_hello_;
		const char * p_other_hello = (const char *)&p_conn_data->buffer_;
		debug_outputln("%s %d : "
				"%2.2x%2.2x%2.2x%2.2x "
				" %2.2x%2.2x%2.2x%2.2x ", __FILE__, __LINE__, 
				p_my_hello[0], p_my_hello[1], p_my_hello[2], p_my_hello[3],
				p_other_hello[0], p_other_hello[1], p_other_hello[2], p_other_hello[3]);
		update_new(p_conn_data, &purge_entry_timer_value);
	}
	/* Purge all that have timed out */
	purged_count = update_missing(p_conn_data);
	return 0;
}

