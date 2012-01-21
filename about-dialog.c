/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @brief
 * @file about-dialog.c
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
#include "about-dialog.h"
#include "debug_helpers.h"
#include "common-dialogs-res.h"

static HWND hVersionInfo = NULL;
static HWND hModuleName = NULL;
static TCHAR module_file_name[MAX_PATH+1];
/*!
 * @brief Maximum number of characters that are to appear on the version control.
 */
#define MODULE_VERSION_STRING_LENGTH (16)

static TCHAR module_version_string[MODULE_VERSION_STRING_LENGTH];

static INT_PTR CALLBACK VersionDialogProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage)
    {
        case WM_INITDIALOG:
            hVersionInfo = GetDlgItem(hDlg, IDC_VERSION_INFO);
            hModuleName = GetDlgItem(hDlg, IDC_MODULE_NAME);
            assert(hVersionInfo);
            assert(hModuleName);
            if (_T('\0') == module_file_name[0])
                GetModuleFileName(NULL, module_file_name, MAX_PATH+1);
            SetWindowText(hModuleName, module_file_name);
            {
                DWORD dwSize;
                dwSize = GetFileVersionInfoSize(module_file_name, &dwSize);
                {
                    uint8_t * data = alloca(dwSize);
                    UINT                uiVerLen = 0;
                    VS_FIXEDFILEINFO*   pFixedInfo = 0;     // pointer to fixed file info structure
                    GetFileVersionInfo(module_file_name, 0, dwSize, (void *)&data[0]);

                    // get the fixed file info (language-independend) 
                    if( 0 != VerQueryValue(&data[0], TEXT("\\"), (void**)&pFixedInfo, (UINT *)&uiVerLen))
                    {
                        StringCchPrintf(module_version_string, MODULE_VERSION_STRING_LENGTH, "%u.%u.%u.%u", 
                                HIWORD (pFixedInfo->dwProductVersionMS),
                                LOWORD (pFixedInfo->dwProductVersionMS),
                                HIWORD (pFixedInfo->dwProductVersionLS),
                                LOWORD (pFixedInfo->dwProductVersionLS));
                        SetWindowText(hVersionInfo, module_version_string);
                    }
                    else
                    {
                        debug_outputln("%s %u : %u", __FILE__, __LINE__, GetLastError());
                    }
                }
            }
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hDlg, LOWORD(wParam));
                    break;
                default:
                    break;
            }
        default:
            break;
    }
    return FALSE;
}

void display_about_dialog(HWND hWndParent)
{
	DialogBox(NULL, MAKEINTRESOURCE(IDD_VERSION_DIALOG), hWndParent, VersionDialogProc);
}

