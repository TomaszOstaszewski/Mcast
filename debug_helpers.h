/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file debug_helpers.h
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
 * @brief
 * @details
 */
#if !defined DEBUG_HELPERS_H_4D66D49A_52F1_4AFE_BB0D_F82EBFD5439A
#define DEBUG_HELPERS_H_4D66D49A_52F1_4AFE_BB0D_F82EBFD5439A

#if defined __cplusplus
extern "C" {
#endif 
#include <windows.h>

/*!
 * @brief Outputs a formated text into the debug window 
 * @details The debug window is the one to which we write using OutputDebugString. 
 * Usually, it is attached to a debugger, but can also be seen using DebugView application
 * from Sysinternal.
 * @param[in] formatString
 * @return
 */
HRESULT debug_output(LPCTSTR formatString, ...);

/*!
 * @brief Outputs a formated text into the debug window, appends newline at the end.
 * @details The debug window is the one to which we write using OutputDebugString. 
 * Usually, it is attached to a debugger, but can also be seen using DebugView application
 * from Sysinternal.
 * @param[in] formatString
 * @return
 */
void ErrorHandlerEx(WORD wLine, LPSTR lpszFile);

#if defined __cplusplus
}
#endif 

#endif /* DEBUG_HELPERS_H_4D66D49A_52F1_4AFE_BB0D_F82EBFD5439A */

