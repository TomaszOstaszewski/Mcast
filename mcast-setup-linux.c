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
 */

#include "pcc.h"
#include "mcast_setup.h"
#include "mcast_utils.h"
#include "resolve.h"
#include "debug_helpers.h"

extern int debug_outputln(const char * format_string, ...);

static void dump_addrinfo(struct addrinfo const * p_info, const char * file, unsigned int line)
{
    char host[NI_MAXHOST] = { 0 };
    FormatAddress(p_info->ai_addr, p_info->ai_addrlen, host, NI_MAXHOST);
    debug_outputln("%s %4.4u : flg:%d fam:%d sot:%d pro:%d can:%s hst:'%15s' nxt:%p", file, line,
        p_info->ai_flags,
        p_info->ai_family,
        p_info->ai_socktype,
        p_info->ai_protocol,
        p_info->ai_canonname,
        host,
        p_info->ai_next);
}

static int set_reuse_addr(SOCKET s)
{
    int optval, rc;
    optval = 1;
    rc = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval));
    if (rc == SOCKET_ERROR)
    {
        return 0;
    }
    return 1;
}

static void dump_locally_bound_socket(SOCKET s, const char * file, unsigned int line)
{
    int rc;
    struct sockaddr_in local_bind = { 0 };
    socklen_t local_data_len = sizeof(local_bind);
    rc = getsockname(s, (struct sockaddr*)&local_bind, &local_data_len);
    if (SOCKET_ERROR != rc)
    {
        debug_outputln("%s %4.4u : %s:%u", file, line, inet_ntoa(local_bind.sin_addr), ntohs(local_bind.sin_port));
    }
    else
    {
        debug_outputln("%s %4.4u : %u", file, line, rc);
    }
}

static int setup_multicast_impl(char * bindAddr, unsigned int nTTL, char * p_multicast_addr, char * p_port, struct mcast_connection * p_mcast_conn)
{
	int rc;
    debug_outputln("%s %d : %s %s %s", __FILE__, __LINE__, bindAddr, p_multicast_addr, p_port);
	p_mcast_conn->multiAddr_ 	= ResolveAddressWithFlags(p_multicast_addr, p_port, AF_UNSPEC, SOCK_DGRAM, IPPROTO_UDP, AI_PASSIVE);
	if (NULL == p_mcast_conn->multiAddr_)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, get_last_socket_error(), get_last_socket_error());
		goto cleanup;
	}
    dump_addrinfo(p_mcast_conn->multiAddr_, __FILE__, __LINE__);
	// Resolve the binding address
	p_mcast_conn->bindAddr_ 	= ResolveAddressWithFlags(bindAddr, p_port, p_mcast_conn->multiAddr_->ai_family, p_mcast_conn->multiAddr_->ai_socktype, p_mcast_conn->multiAddr_->ai_protocol, AI_PASSIVE);
	if (NULL == p_mcast_conn->bindAddr_)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, get_last_socket_error(), get_last_socket_error());
		goto cleanup;
	}
    dump_addrinfo(p_mcast_conn->bindAddr_, __FILE__, __LINE__);
	// 
	// Create the socket. In Winsock 1 you don't need any special
	// flags to indicate multicasting.
	//
	p_mcast_conn->socket_ 		= socket(p_mcast_conn->multiAddr_->ai_family, p_mcast_conn->multiAddr_->ai_socktype, p_mcast_conn->multiAddr_->ai_protocol);
	if (p_mcast_conn->socket_ == INVALID_SOCKET)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, get_last_socket_error(), get_last_socket_error());
		goto cleanup;
	}
    if (!set_reuse_addr(p_mcast_conn->socket_))
    {
        debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, get_last_socket_error(), get_last_socket_error());
        goto cleanup;
    }
	rc = bind(p_mcast_conn->socket_, p_mcast_conn->bindAddr_->ai_addr, (int) p_mcast_conn->bindAddr_->ai_addrlen);
	if (rc == SOCKET_ERROR)
	{
		debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, get_last_socket_error(), get_last_socket_error());
		goto cleanup;
	}
    dump_locally_bound_socket(p_mcast_conn->socket_, __FILE__, __LINE__);
	// Join the multicast group if specified
    rc = join_mcast_group_set_ttl(p_mcast_conn->socket_, p_mcast_conn->multiAddr_, p_mcast_conn->bindAddr_, nTTL);
	return 1;
cleanup:
	return 0;
}

static int setup_multicast_addr(char * bindAddr, uint8_t nTTL, struct sockaddr_in const * p_in_addr, struct mcast_connection * p_mcast_conn)
{
    char port[8];
    int result;
    char * inet_addr;
    inet_addr = inet_ntoa(p_in_addr->sin_addr); 
#if defined WIN32
    StringCchPrintf(port, 8, "%d", ntohs(p_in_addr->sin_port));
#else
    snprintf(port, 8, "%d", ntohs(p_in_addr->sin_port));
#endif
    result = setup_multicast_impl(bindAddr, nTTL, inet_addr, port, p_mcast_conn);
    return result;
}

int setup_multicast_indirect(struct mcast_settings const * p_settings, struct mcast_connection * p_conn)
{
    return setup_multicast_addr(p_settings->bindAddr_, p_settings->nTTL_, &p_settings->mcast_addr_, p_conn);
}

size_t mcast_sendto_flags(struct mcast_connection * p_conn, void const * p_data, size_t data_size, int flags)
{
    return sendto(p_conn->socket_, (const void *)p_data, data_size, flags, p_conn->multiAddr_->ai_addr, (int) p_conn->multiAddr_->ai_addrlen);
}

size_t mcast_sendto(struct mcast_connection * p_conn, void const * p_data, size_t data_size)
{
    return mcast_sendto_flags(p_conn, p_data, data_size, 0);
}

size_t mcast_recvfrom_flags(struct mcast_connection * p_conn, void * p_data, size_t data_size, int flags)
{
    return recvfrom(p_conn->socket_, p_data, data_size, flags, p_conn->multiAddr_->ai_addr, &p_conn->multiAddr_->ai_addrlen);
 }

int mcast_is_new_data(struct mcast_connection * p_conn, size_t dwTimeoutMs)
{
    fd_set read_sel; 
    int result;
    struct timeval timeout, *p_timeout;
    ZeroMemory(&timeout, sizeof(struct timeval));
    FD_ZERO(&read_sel);
    FD_SET(p_conn->socket_, &read_sel);
    if (0xffffffff == dwTimeoutMs)
        p_timeout = NULL;
    else
    {
        timeout.tv_sec = dwTimeoutMs / 1000;
        timeout.tv_usec = (dwTimeoutMs - 1000*(dwTimeoutMs/1000))*1000;
        p_timeout = &timeout; 
    }
    result = select(0 /* This parameter is ignored in WINSOCK */, &read_sel, NULL, NULL, p_timeout);
    if (SOCKET_ERROR == result)
    {
        debug_outputln("%s %u : %u", __FILE__, __LINE__, get_last_socket_error());
    }
    return result;
}

size_t mcast_recvfrom(struct mcast_connection * p_conn, void * p_data, size_t data_size)
{
    return mcast_recvfrom_flags(p_conn, p_data, data_size, 0);
}

int close_multicast(struct mcast_connection * p_mcast_conn)
{
    if (NULL != p_mcast_conn)
    {
        freeaddrinfo(p_mcast_conn->bindAddr_);
        freeaddrinfo(p_mcast_conn->multiAddr_);
        closesocket(p_mcast_conn->socket_);
    }
    return 1;
}

