/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file mcast-settings.c
 * @brief Functions for multicast group membership settings manipulation.
 * @author T.Ostaszewski
 * @date Jan-2012
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
 */
#include "pcc.h"
#include "mcast-settings.h"

/*!
 * @brief Default multicast group IPv4 address.
 */
#define DEFAULT_MCASTADDRV4 "234.5.6.7"

/*!
 * @brief Default multicast group IPv6 address.
 */
#define DEFAULT_MCASTADDRV6 "ff12::1"

/*!
 * @brief Default multicast port
 */
#define DEFAULT_MCASTPORT (25000)

/*!
 * @brief Default TTL value.
 */
#define DEFAULT_TTL (8)

static struct mcast_settings g_default_settings;

int mcast_settings_get_default(struct mcast_settings * p_target)
{
	unsigned long net_addr = inet_addr(DEFAULT_MCASTADDRV4);
	p_target->bReuseAddr_ = TRUE;
	memcpy(&p_target->mcast_addr_.sin_addr, &net_addr, sizeof(unsigned long));
	p_target->mcast_addr_.sin_port = htons(DEFAULT_MCASTPORT);
    return 1;
}

/*! 
 * @brief Minimum IPv4 multicast group address.
 */
#define MIN_MCAST_ADDR (0xe0000000)

/*! 
 * @brief Maximum IPv4 multicast group address.
 */
#define MAX_MCAST_ADDR (0xefffffff)

int mcast_settings_validate(struct mcast_settings const * p_settings)
{
    unsigned short port = ntohs(p_settings->mcast_addr_.sin_port);
    unsigned long addr  = ntohl(p_settings->mcast_addr_.sin_addr.s_addr); 
    if (port < 1024)
        return 0;
    if (addr < MIN_MCAST_ADDR || addr > MAX_MCAST_ADDR)
        return 0;
    return 1;
}

void mcast_settings_copy(struct mcast_settings * p_dest, struct mcast_settings const * p_source)
{
	struct mcast_settings tmp;	
	CopyMemory(&tmp, p_source);
	CopyMemory(p_dest, &tmp);
}

void mcast_settings_swap(struct mcast_settings * p_left, struct mcast_settings * p_right)
{
	struct mcast_settings tmp;
	CopyMemory(&tmp, p_left);
	CopyMemory(p_left, p_right);
	CopyMemory(p_right, &tmp);
}

