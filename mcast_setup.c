/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file mcast_setup.c
 * @brief Helper routines for joining and leaving an multicast group.  
 * @details One of the advantages of using the setsockopt over WSAJoinLeaf is
 * the ability to join multiple multicast groups on a single socket
 * which is not possible with WSAJoinLeaf.
 *
 * @attention Also note that because we include winsock2.h we must link with
 * ws2_32.lib and not with wsock32.lib.
 *
 * This sample uses the new getaddrinfo/getnameinfo functions which are new to 
 * Windows XP. To run this sample on older OSes, include the following header
 * file which makes it work automagically.
 * @author T. Ostaszewski
 * @date 04-Jan-2012
 */

#include "pcc.h"
#include "winsock_adapter.h"
#include "mcast_setup.h"
#include "conn_data.h"
#include "hello.h"
#include "mcast_utils.h"
#include "winsock_adapter.h"
#include "resolve.h"
#include "debug_helpers.h"

/*!
 * @brief
 */ 
#define DEFAULT_TTL    8

static char *gInterface=NULL;          // Interface to join the multicast group on

int setup_multicast(BOOL bConnect, BOOL bReuseAddr, char * bindAddr, uint8_t nTTL, char * p_multicast_addr, char * p_port, struct mcast_connection * p_mcast_conn)
{
	int rc;
	p_mcast_conn->multiAddr_ 	= ResolveAddress(p_multicast_addr, p_port, AF_UNSPEC, SOCK_DGRAM, IPPROTO_UDP);
	if (NULL == p_mcast_conn->multiAddr_)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, WSAGetLastError(), WSAGetLastError());
		goto cleanup;
	}
	// Resolve the binding address
	p_mcast_conn->bindAddr_ 	= ResolveAddress(bindAddr, p_port, p_mcast_conn->multiAddr_->ai_family, p_mcast_conn->multiAddr_->ai_socktype, p_mcast_conn->multiAddr_->ai_protocol);
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
	rc = SetMulticastTtl(p_mcast_conn->socket_, p_mcast_conn->multiAddr_->ai_family, nTTL);
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

int setup_multicast_default(char * p_multicast_addr, char * p_port, struct mcast_connection * p_mcast_conn)
{
    return setup_multicast(FALSE, TRUE, NULL, DEFAULT_TTL, p_multicast_addr, p_port, p_mcast_conn);
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

