/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file debug_helpers.c
 * @author T. Ostaszewski
 * @date 04-Jan-2012
 * @brief 
 * @details 
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
 * @author T.Ostaszewski
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

/*!
 * @brief Frees the buffer allocated via get_text_hresult
 * @param[in] psz_buffer - pointer to the buffer allocated via get_text_hresult
 * @author T.Ostaszewski
 * @sa get_text_hresult
 */
static void free_text_hresult(const char * psz_buffer)
{
    LocalFree( (LPVOID)psz_buffer );
}

/*!
 * @brief Returns text for given HRESULT
 * @param[in] hr HRESULT code for which text description is to be looked upon
 * @return pointer to a buffer that contains textual description of the HRESULT given
 * @sa free_text_hresult
 * @author T.Ostaszewski
 */
static const char * get_text_hresult(HRESULT hr)
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            (LPTSTR) &lpMsgBuf,
            0,
            NULL);
    // Process any inserts in lpMsgBuf.
    // ...
    // Display the string.
    // Free the buffer.
    return (const char *)lpMsgBuf;
}

void ErrorHandlerEx( WORD wLine, LPSTR lpszFile )
{
	LPVOID lpvMessage;
	DWORD  dwError;
	TCHAR  szBuffer[BUFFER_SIZE];

	// Allow FormatMessage() to look up the error code returned by GetLastError
	dwError = FormatMessage(   FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM, 
			NULL, 
			GetLastError(), 
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), 
			(LPTSTR)&lpvMessage, 
			0, 
			NULL);

	// Check to see if an error occurred calling FormatMessage()
	if (0 == dwError)
	{
		StringCchPrintf(szBuffer, 
				BUFFER_SIZE,
				TEXT("An error occurred calling FormatMessage().")
				TEXT("Error Code %d"), 
				GetLastError());
		MessageBox( NULL, 
				szBuffer, 
				TEXT("Generic"), 
				MB_ICONSTOP | MB_ICONEXCLAMATION);
		return;
	}

	// Display the error information along with the place the error happened.
	StringCchPrintf(szBuffer, BUFFER_SIZE, TEXT("Generic, Line=%d, File=%s"), wLine, lpszFile);
	MessageBox(NULL, lpvMessage, szBuffer, MB_ICONEXCLAMATION | MB_OK);
}




