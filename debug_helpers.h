#if !defined DEBUG_HELPERS_H_4D66D49A_52F1_4AFE_BB0D_F82EBFD5439A
#define DEBUG_HELPERS_H_4D66D49A_52F1_4AFE_BB0D_F82EBFD5439A

#if defined __cplusplus
extern "C" {
#endif 
#include <windows.h>

HRESULT debug_output(LPCTSTR formatString, ...);
HRESULT debug_outputln(LPCTSTR formatString, ...);
void ErrorHandlerEx(WORD wLine, LPSTR lpszFile);

#if defined __cplusplus
}
#endif 

#endif /* DEBUG_HELPERS_H_4D66D49A_52F1_4AFE_BB0D_F82EBFD5439A */

