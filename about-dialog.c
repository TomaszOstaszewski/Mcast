/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file about-dialog.c
 * @author T.Ostaszewski
 * @date Jan-2012
 * @brief The "About ..." modal dialog implementation.
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

/*!
 * @brief Maximum number of characters that are to appear on the version control.
 */
#define MODULE_VERSION_STRING_LENGTH (16)

/*!
 * @brief Structure that holds "About.." dialog data.
 * @details Here we store both data to be displayed onto the dialog controls, as well as
 * the handles to the dialog controls. 
 */
struct about_dlg_data {
	HWND hDlg_; /*!< Handle to the dialog window itself. */
    HWND hVersion_; /*!< Handle to the control holding the version information. */
    HWND hModuleName_; /*!< Handle to the control holding the module path information. */
    TCHAR module_name[MAX_PATH+1]; /*!< Buffer in which we store module path. */
    TCHAR module_version_string[MODULE_VERSION_STRING_LENGTH]; /*!< Buffer in which we store module version. */
    LPCTSTR pszCaption_;
};

static BOOL Handle_wm_initdialog(HWND hwnd, HWND hWndFocus, LPARAM lParam)
{
    struct about_dlg_data * p_dlg;
    HRESULT hr = E_FAIL;
    DWORD dwSize;
    uint8_t * data;
    UINT                uiVerLen = 0;
    VS_FIXEDFILEINFO*   pFixedInfo = 0;     // pointer to fixed file info structure
    p_dlg = (struct about_dlg_data*)lParam;
    assert(p_dlg);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)p_dlg);
	p_dlg->hDlg_ = hwnd;
    p_dlg->hVersion_ = GetDlgItem(hwnd, IDC_VERSION_INFO);
    assert(p_dlg->hVersion_);
    p_dlg->hModuleName_ = GetDlgItem(hwnd, IDC_MODULE_NAME);
    assert(p_dlg->hModuleName_);

    if (_T('\0') == p_dlg->module_name[0])
    {
        GetModuleFileName(NULL, p_dlg->module_name, MAX_PATH+1);
    }
    SetWindowText(p_dlg->hModuleName_, p_dlg->module_name);
    dwSize = GetFileVersionInfoSize(p_dlg->module_name, &dwSize);
    data = alloca(dwSize);
    GetFileVersionInfo(p_dlg->module_name, 0, dwSize, (void *)&data[0]);
    // get the fixed file info (language-independend) 
    if (0 != VerQueryValue(&data[0], TEXT("\\"), (void**)&pFixedInfo, (UINT *)&uiVerLen))
    {
        hr = StringCchPrintf(p_dlg->module_version_string, MODULE_VERSION_STRING_LENGTH, "%u.%u.%u.%u", 
                HIWORD (pFixedInfo->dwProductVersionMS),
                LOWORD (pFixedInfo->dwProductVersionMS),
                HIWORD (pFixedInfo->dwProductVersionLS),
                LOWORD (pFixedInfo->dwProductVersionLS));
    }
    if (SUCCEEDED(hr))
    {
        SetWindowText(p_dlg->hVersion_, p_dlg->module_version_string);
    }
    else
    {
        debug_outputln("%s %u : %u", __FILE__, __LINE__, GetLastError());
    }
    SetWindowText(hwnd, p_dlg->pszCaption_);
    return TRUE;
} 

static INT_PTR CALLBACK VersionDialogProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    struct about_dlg_data * p_dlg;
    p_dlg = (struct about_dlg_data *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    switch (uMessage)
    {
        case WM_INITDIALOG:
            return HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, Handle_wm_initdialog);
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

void display_about_dialog(HWND hWndParent, LPCTSTR pszCaption)
{
    struct about_dlg_data dlg;
    ZeroMemory(&dlg, sizeof(dlg));
    dlg.pszCaption_ = pszCaption;
	DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_VERSION_DIALOG), hWndParent, VersionDialogProc, (LPARAM)&dlg);
}

