/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file debug_helpers.h
 * @author T.Ostaszewski
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
 * @author T.Ostaszewski
 */
HRESULT debug_output(LPCTSTR formatString, ...);

/*!
 * @brief Outputs a formated text into the debug window, appends newline at the end.
 * @details The debug window is the one to which we write using OutputDebugString. 
 * Usually, it is attached to a debugger, but can also be seen using DebugView application
 * from Sysinternal.
 * @param[in] formatString
 * @return
 * @author T.Ostaszewski
 */
HRESULT debug_outputln(LPCTSTR formatString, ...);

/*!
 * @brief Displays a message box with a textual code for HRESULT given
 * @param[in] wLine line number to be displayed in the message box
 * @param[in] lpszFile file name to be displayed in the message box
 * @author T.Ostaszewski
 */
void ErrorHandlerEx(WORD wLine, LPSTR lpszFile);

#if defined __cplusplus
}
#endif 

#endif /* DEBUG_HELPERS_H_4D66D49A_52F1_4AFE_BB0D_F82EBFD5439A */

