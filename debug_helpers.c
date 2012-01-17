/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file debug_helpers.c
 * @author T. Ostaszewski
 * @brief 
 * @details 
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
#include "debug_helpers.h"

/*!
 * @brief Output buffer size for debug_output and debug_outpuln.
 */
#define OUTPUT_BUFFER_LEN (16)

/*!
 * @brief Output buffer size for ErrorHandlerEx.
 */
#define BUFFER_SIZE (256)

/*!
 * 
 */
#define BUFFER_MAX_TCHARS (16*OUTPUT_BUFFER_LEN)

/*!
 * @brief Output buffer for debug_output and debug_outputln
 * @details This is a thread specific global, so each thread has its own copy of the buffer.
 */
static __declspec(thread) TCHAR outputBuffer[OUTPUT_BUFFER_LEN];

/*!
 * 
 */
static __declspec(thread) TCHAR g_lines[BUFFER_MAX_TCHARS];

/*!
 * 
 */
static __declspec(thread) UINT  g_write_offset;

HRESULT debug_outputln(LPCTSTR formatString, ...)
{
	HRESULT hr;
	va_list args;
	va_start(args, formatString);
	hr = StringCchVPrintf(outputBuffer, OUTPUT_BUFFER_LEN, formatString, args);
	if (SUCCEEDED(hr) || hr == STRSAFE_E_INSUFFICIENT_BUFFER)
    {
		OutputDebugString(outputBuffer);
    }
    va_end(args);
	return hr;
}

void debug_output_flush(void)
{
    /* Write all the lines and zero-out the write offset */
    g_lines[g_write_offset] = '\0';
    OutputDebugString(&g_lines[0]);
    g_write_offset = 0;
}

HRESULT debug_outputln_buffered(LPCTSTR formatString, ...)
{
	HRESULT hr;
    LPTSTR pszDestEnd;
	va_list args;
	va_start(args, formatString);
    /* Write a formatted string into the temporary buffer */
	hr = StringCchVPrintfEx(outputBuffer, OUTPUT_BUFFER_LEN, &pszDestEnd, NULL, 0, formatString, args);
	if (SUCCEEDED(hr) || hr == STRSAFE_E_INSUFFICIENT_BUFFER)
    {
        /* Check sum of write_offset + string length,
         * if more than output buffer size, then flush output buffer;
         * copy from temporary buffer @ write offset
         * replace last character with '\n'
         */
        UINT new_string_length = pszDestEnd - outputBuffer + 1; /* We count the terminating null - it will be replaced by the '\n' character. */
        UINT new_write_offset = g_write_offset + new_string_length;
        *pszDestEnd = '\n';
        if (new_write_offset>BUFFER_MAX_TCHARS)
        {
            debug_output_flush();
        }
        /* CopyMemory - as we know the length and we know the source string may not be NULL terminated */
        CopyMemory(&g_lines[g_write_offset], outputBuffer, new_string_length*sizeof(TCHAR)); 
        g_write_offset += new_string_length;
    }
    va_end(args);
    return hr;
}

