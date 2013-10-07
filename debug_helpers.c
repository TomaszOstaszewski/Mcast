/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file debug_helpers.c
 * @author T. Ostaszewski
 * @brief DebugView interface implementation.
 * @details More friendly wrappers for OutputDebugString. This 'friendlines' manifests itself with
 * support for buffered write and variable parameters, i.e. printf-like interface.
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
#include <stdarg.h>
#include "compiler_defs.h"

/*!
 * @brief Output buffer size for debug_output and debug_outpuln.
 */
#define OUTPUT_BUFFER_LEN (256)

/*!
 * @brief Output buffer size for ErrorHandlerEx.
 */
#define BUFFER_SIZE (256)

/*!
 * @brief Size of the output string buffer.
 * @attention A buffer of that size will be copied with each thread created. Therefore, it's a matter of balance 
 * between number memory usage and thread safety.
 */
#define BUFFER_MAX_TCHARS (4096)

/*!
 * @brief Output buffer for debug_output and debug_outputln
 * @details This is a thread specific global, so each thread has its own copy of the buffer.
 * @attention This buffer is a thread local variable. Thus, each thread created, whether you want it or not,
 * will have a copy of that buffer.  
 */
static THREAD_LOCAL char g_outputBuffer[OUTPUT_BUFFER_LEN];

/*!
 * @brief Maximum number of characters that fit the buffer.
 * @details If writting a string using debug_outputln() will result with buffer overrun, the entire buffer
 * is flushed. Flushing the buffer is time consuming operation, just because it involves crossing process boundaries.
 * Therefore, if one wants to monitor some close-to-realtime process, she or he shall better use the buffered approach, 
 * whose amortized cost is a lot lower than constant calling debug_outputln(), which will flush the string each time it's 
 * being called.
 */
static THREAD_LOCAL char g_lines[BUFFER_MAX_TCHARS];

static THREAD_LOCAL char g_tmp_buffer[OUTPUT_BUFFER_LEN];

/*!
 * @brief Number of characters already placed in the buffer.
 * @attention This is a thread specific variable. Each thread gets a copy of it.
 */
static THREAD_LOCAL size_t g_write_offset;

#if defined WIN32

static int debug_outputlnA_impl(const char * formatString, va_list args)
{
	HRESULT hr;
	hr = StringCchVPrintfA(g_outputBuffer, OUTPUT_BUFFER_LEN, formatString, args);
	if (SUCCEEDED(hr) || hr == STRSAFE_E_INSUFFICIENT_BUFFER)
    {
		OutputDebugStringA(g_outputBuffer);
    }
    va_end(args);
	return SUCCEEDED(hr);
}

void debug_output_flush(void)
{
    /* Write all the lines and zero-out the write offset */
    g_lines[g_write_offset] = '\0';
    OutputDebugString(&g_lines[0]);
    g_write_offset = 0;
}

int debug_outputln_bufferedA(const char * formatString, ...)
{
	HRESULT hr;
    LPTSTR pszDestEnd;
	va_list args;
	va_start(args, formatString);
    /* Write a formatted string into the temporary buffer */
	hr = StringCchVPrintfEx(g_outputBuffer, OUTPUT_BUFFER_LEN, &pszDestEnd, NULL, 0, formatString, args);
	if (SUCCEEDED(hr) || hr == STRSAFE_E_INSUFFICIENT_BUFFER)
    {
        /* Check if the sum of write_offset and string length is greater,
         * than output buffer size. If so, then flush output buffer
         * copy from temporary buffer @ write offset
         * replace last character with '\n'
         */
        UINT new_string_length = pszDestEnd - g_outputBuffer + 1; /* We count the terminating null, hence +1. This null will be replaced by the '\n' character. */
        UINT new_write_offset = g_write_offset + new_string_length;
        *pszDestEnd = '\n';
        if (new_write_offset>BUFFER_MAX_TCHARS)
        {
            debug_output_flush();
        }
        /* CopyMemory - as we know the length and we know the source string may not be NULL terminated */
        CopyMemory(&g_lines[g_write_offset], g_outputBuffer, new_string_length*sizeof(TCHAR)); 
        g_write_offset += new_string_length;
    }
    va_end(args);
    return hr;
}

#else

#	include <syslog.h>
#	include <string.h>

static int debug_outputlnA_impl(const char * formatString, va_list args)
{
	int chars_printed;
	chars_printed = vsnprintf(g_outputBuffer, OUTPUT_BUFFER_LEN, formatString, args);
	if (chars_printed>0)
    {
		fprintf(stderr, "%s", g_outputBuffer);
		return 1;
    }
	return 0;
}

void debug_output_flush(void)
{
    /* Write all the lines and zero-out the write offset */
    g_lines[g_write_offset] = '\0';
    fprintf(stderr, "%s", &g_lines[0]);
    g_write_offset = 0;
}

int debug_outputln_bufferedA(const char * formatString, ...)
{
	int chars_written;
	va_list args;
	va_start(args, formatString);
    /* Write a formatted string into the temporary buffer */
	chars_written = vsnprintf(g_outputBuffer, OUTPUT_BUFFER_LEN, formatString, args);
	if (chars_written > 0) 
    {
		/* Chacke if string was truncated */
		if (chars_written >= OUTPUT_BUFFER_LEN)
		{
			return -1;
		}
		else
		{
			/* Check if we fit the destination buffer */	
			size_t new_length = chars_written;
			if (new_length >= BUFFER_MAX_TCHARS)
			{
				debug_output_flush();
			}
			CopyMemory(&g_lines[g_write_offset], g_outputBuffer, chars_written*sizeof(char));
			g_write_offset += chars_written;
		}
    }
    va_end(args);
    return 0;
}


#endif

int debug_outputlnA(const char * formatString, ...)
{
	int retval;
	va_list args;
	va_start(args, formatString);
	retval = debug_outputlnA_impl(formatString, args);
	va_end(args);
	return retval;
}

int debug_outputlnW(const wchar_t * formatString, ...)
{
	/* Convert the format string to ANSI */
	int retval = -1;
	size_t bytes_written;
	bytes_written = wcstombs(g_tmp_buffer, formatString, sizeof(g_tmp_buffer));	
	if ((bytes_written) != (-1))
	{
		va_list args;
		va_start(args, formatString);
		retval = debug_outputlnA_impl(g_tmp_buffer, args);
		va_end(args);
	}
	return retval;
}

#if 0
int debug_outputln_bufferedA(const char * formatString, ...)
{
	int chars_written;
	va_list args;
	va_start(args, formatString);
    /* Write a formatted string into the temporary buffer */
	chars_written = vsnprintf(g_outputBuffer, OUTPUT_BUFFER_LEN, formatString, args);
	if (chars_written > 0) 
    {
		/* Chacke if string was truncated */
		if (chars_written >= OUTPUT_BUFFER_LEN)
		{
			return -1;
		}
		else
		{
			/* Check if we fit the destination buffer */	
			size_t new_length = chars_written;
			if (new_length >= BUFFER_MAX_TCHARS)
			{
				debug_output_flush();
			}
			COPY_MEMORY(&g_lines[g_write_offset], g_outputBuffer, chars_written*sizeof(char));
			g_write_offset += chars_written;
		}
    }
    va_end(args);
    return hr;
}

int debug_outputln_bufferedA(LPCTSTR formatString, ...)
{
	HRESULT hr;
    LPTSTR pszDestEnd;
	va_list args;
	va_start(args, formatString);
    /* Write a formatted string into the temporary buffer */
	hr = StringCchVPrintfEx(g_outputBuffer, OUTPUT_BUFFER_LEN, &pszDestEnd, NULL, 0, formatString, args);
	if (SUCCEEDED(hr) || hr == STRSAFE_E_INSUFFICIENT_BUFFER)
    {
        /* Check if the sum of write_offset and string length is greater,
         * than output buffer size. If so, then flush output buffer
         * copy from temporary buffer @ write offset
         * replace last character with '\n'
         */
        UINT new_string_length = pszDestEnd - g_outputBuffer + 1; /* We count the terminating null, hence +1. This null will be replaced by the '\n' character. */
        UINT new_write_offset = g_write_offset + new_string_length;
        *pszDestEnd = '\n';
        if (new_write_offset>BUFFER_MAX_TCHARS)
        {
            debug_output_flush();
        }
        /* CopyMemory - as we know the length and we know the source string may not be NULL terminated */
        CopyMemory(&g_lines[g_write_offset], g_outputBuffer, new_string_length*sizeof(TCHAR)); 
        g_write_offset += new_string_length;
    }
    va_end(args);
    return hr;
}

#endif

