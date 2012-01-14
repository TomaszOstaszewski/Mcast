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
#define OUTPUT_BUFFER_LEN (256)

/*!
 * @brief Output buffer size for ErrorHandlerEx.
 */
#define BUFFER_SIZE (256)

/*!
 * @brief Output buffer for debug_output and debug_outputln
 * @details This is a thread specific global, so each thread has its own copy of the buffer.
 */
static __declspec(thread) TCHAR outputBuffer[OUTPUT_BUFFER_LEN];

/*!
 * @brief Helper function, actual implementation of both debug_output and debug_outputln
 * @param[in] formatString string for printf
 * @param[in] args arguments list for printf
 */
static HRESULT debug_output_args(LPCTSTR formatString, va_list args)
{
	HRESULT hr;
	hr = StringCchVPrintf(outputBuffer, OUTPUT_BUFFER_LEN, formatString, args);
	if (SUCCEEDED(hr))
		OutputDebugString(outputBuffer);
	return hr;
}

HRESULT debug_output(LPCTSTR formatString, ...)
{
	HRESULT hr;
	va_list args;
	va_start(args, formatString);
	hr = debug_output_args(formatString, args);
	return hr;
}

HRESULT debug_outputln(LPCTSTR formatString, ...)
{
	HRESULT hr;
	va_list args;
	va_start(args, formatString);
	hr = debug_output_args(formatString, args);
	if (SUCCEEDED(hr))
	{
	//	OutputDebugString("\n");
	}
	return hr;
}

