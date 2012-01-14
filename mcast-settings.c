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
 * @brief 
 */
#define DEFAULT_MCASTADDRV4 "234.5.6.7"

/*!
 * @brief 
 */
#define DEFAULT_MCASTADDRV6 "ff12::1"

/*!
 * @brief 
 */
#define DEFAULT_MCASTPORT (25000)

/*!
 * @brief
 */
#define DEFAULT_TTL (8)

static struct mcast_settings g_default_settings;

struct mcast_settings const * get_default_mcast_settings(void)
{
	unsigned long net_addr = inet_addr(DEFAULT_MCASTADDRV4);
	g_default_settings.bReuseAddr_ = TRUE;
	memcpy(&g_default_settings.mcast_addr_.sin_addr, &net_addr, sizeof(unsigned long));
	g_default_settings.mcast_addr_.sin_port = htons(DEFAULT_MCASTPORT);
	return &g_default_settings;
}

