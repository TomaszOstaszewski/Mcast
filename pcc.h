/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file pcc.h
 * @author T.Ostaszewski
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

