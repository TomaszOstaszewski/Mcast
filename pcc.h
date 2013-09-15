/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file pcc.h
 * @brief Pre compiled header file.
 * @details This file is used to create a pre compiled header. The whole point of having a pre compiled header
 * is to save on compile time. The pre compiled header therefore contains a lot of include directives on a lot of header
 * files that:
 * - include a lot of other header files;
 * - do not change a lot during the course of the project.
 *
 * The perfect candidate to put into into the pre-compiled header are therefore:
 * - all the standard library files (stdlib.h, stdint.h and so on);
 * - platform specific files (asm/x86.h); 
 * - and vendor specific files (windows.h and so on).
 *
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
#if !defined PCC_H_10C143CE_7FF9_4E22_8187_6EBF9DAEAA7B
#define PCC_H_10C143CE_7FF9_4E22_8187_6EBF9DAEAA7B

#include "platform-sockets.h"
#include "compiler_defs.h"
#if defined WIN32
#   define _CRT_SECURE_NO_WARNINGS 
#   include <windowsx.h>
#   include <commctrl.h>
#   include <winerror.h>
#   include <mmreg.h>
#   include <mmsystem.h>
#   include <tchar.h>
#   include <objbase.h>
#   include <malloc.h> /* For alloca() */
#if !defined __MINGW32__
#   undef _ftcscat
#       include <dsound.h>
#       include <strsafe.h>
#   endif
#endif

#include <memory.h>
#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if defined __linux__
#   include <arpa/inet.h>
#   include <errno.h>
#   include <fcntl.h>
#   include <netinet/in.h>
#   include <stdint.h>
#   include <sys/mman.h>
#   include <sys/socket.h>
#   include <sys/stat.h>
#   include <sys/types.h>
#   include <unistd.h>

typedef uint32_t DWORD ;
typedef uint16_t WORD ;
#define NEAR 
#define FAR 
#endif

#include "std-int.h" /* Wrapper for <stdint.h> which is not avaiable on every compiler. */

#endif /* PCC_H_10C143CE_7FF9_4E22_8187_6EBF9DAEAA7B */

