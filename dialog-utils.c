/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @brief Implementation of the common utility code used all over the place with dialogs.
 * @file dialog-utils.c
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
#include "dialog-utils.h"

/*!
 * @brief Maximum number of digits in 4 byte unsigned decimal.
 * @details As maximum 4 byte unsigned decimal is 2^32= 4294967296, hence the number of digits is 10.
 */
#define UINT32_DIGITS_DEC (10)

/*!
 * @brief Buffer into which we store the contents of the edit control, when asked for 4 byte decimal conversion.
 */
static __declspec(thread) TCHAR g_tls_uint32_buffer[UINT32_DIGITS_DEC+1];

/*!
 * @brief Another helper function taken out of the body of other functions.
 */
static void get_from_edit_control(HWND hControl, TCHAR * buffer, size_t buffer_len)
{
    memset(buffer, 0, buffer_len);
    assert(buffer_len > sizeof(WORD));
    *((WORD *)buffer) = UINT32_DIGITS_DEC;
    SendMessage(hControl, EM_GETLINE, 0, (LPARAM)buffer); 
}

int get_from_edit_uint32_dec(HWND hControl, uint32_t * p_target)
{
    int result;
    uint32_t contents;
    get_from_edit_control(hControl, g_tls_uint32_buffer, sizeof(g_tls_uint32_buffer));
    result = sscanf(g_tls_uint32_buffer, "%u", &contents);
    if (result>0)
    {
        *p_target = contents;
        return 1;
    }
    return 0;
}

int get_from_edit_uint16_dec(HWND hControl, uint16_t * p_target)
{
    int result;
    uint16_t contents;
    get_from_edit_control(hControl, g_tls_uint32_buffer, sizeof(g_tls_uint32_buffer));
    result = sscanf(g_tls_uint32_buffer, "%hu", &contents);
    if (result>0)
    {
        *p_target = contents;
        return 1;
    }
    return 0;
}

int get_from_edit_uint8_dec(HWND hControl, uint8_t * p_target)
{
    int result;
    uint8_t contents;
    get_from_edit_control(hControl, g_tls_uint32_buffer, sizeof(g_tls_uint32_buffer));
    result = sscanf(g_tls_uint32_buffer, "%hhu", &contents);
    if (result>0)
    {
        *p_target = contents;
        return 1;
    }
    return 0;
}

int put_in_edit_control_uint32(HWND hControl, uint32_t value)
{
    BOOL result;
    StringCchPrintf(g_tls_uint32_buffer, UINT32_DIGITS_DEC+1, "%u", value);
    result = SetWindowText(hControl, g_tls_uint32_buffer);
    assert(result);
    return result;
}

int put_in_edit_control_uint16(HWND hControl, uint16_t value)
{
    BOOL result;
    StringCchPrintf(g_tls_uint32_buffer, UINT32_DIGITS_DEC+1, "%hu", value);
    result = SetWindowText(hControl, g_tls_uint32_buffer);
    assert(result);
    return result;
}

int put_in_edit_control_uint8(HWND hControl, uint8_t value)
{
    BOOL result;
    StringCchPrintf(g_tls_uint32_buffer, UINT32_DIGITS_DEC+1, "%hhu", value);
    result = SetWindowText(hControl, g_tls_uint32_buffer);
    assert(result);
    return result;
}

