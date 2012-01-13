/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file pcc.h
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
 * @brief Pre compiled header file.
 * @details This file is used to create a pre compiled header. The whole point of having a pre compiled header
 * is to save on compile time. The pre compiled header therefore contains #include directives on a lot of header
 * files that:
 * \li include a lot of other header files
 * \li do not change a lot
 * The perfect candidate to put into #include <> into pre compiled header are therefore all the standard library files (stdlib.h, stdint.h and so on), platform specific files (asm/x86.h) and vendor specific files (windows.h and so on).
 */
#if !defined PCC_H_10C143CE_7FF9_4E22_8187_6EBF9DAEAA7B
#define PCC_H_10C143CE_7FF9_4E22_8187_6EBF9DAEAA7B

#define _CRT_SECURE_NO_WARNINGS 
#define WIN32_LEAN_AND_MEAN

#include <assert.h>
#include <commctrl.h>
#include <dsound.h>
#include <MMReg.h>
#include <Objbase.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Strsafe.h>
#include <tchar.h>
#include <time.h>
#include <windows.h>
#include <windowsx.h>
#include <Winerror.h>
#include "std-int.h" /* Wrapper for <stdint.h> which is not avaiable on every compiler. */
#include "winsock_adapter.h"

#endif /* PCC_H_10C143CE_7FF9_4E22_8187_6EBF9DAEAA7B */

