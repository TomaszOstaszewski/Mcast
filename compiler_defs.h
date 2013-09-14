/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file debug_helpers.h
 * @brief DebugView interface header.
 * @details Contains declarations of functions that print output to the Debug Console. This is more fancy wrapper for OutputDebugString API. 
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
 */
#if !defined COMPILER_DEFS_H_3F0740A0_908E_4325_AD5C_83EBB2DD3EFA
#define COMPILER_DEFS_H_3F0740A0_908E_4325_AD5C_83EBB2DD3EFA

#include "platform-sockets.h"

#if defined WIN32
#	define THREAD_LOCAL __declspec(thread)
#	define PACKED 

#else
#	define THREAD_LOCAL __thread
#	define CopyMemory(t,s,c) memcpy(t,s,c)
#	define ZeroMemory(p,c) memset(p, 0, c)
#	define PACKED __attribute__((packed))

#endif

unsigned long get_last_socket_error(void);

#endif /*if !defined COMPILER_DEFS_H_3F0740A0_908E_4325_AD5C_83EBB2DD3EFA */
