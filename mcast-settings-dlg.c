/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/*!
 * @file mcast-settings-dlg.c
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
 */
#include "pcc.h"
#include "mcast-settings.h"
#include "mcast-settings-dlg.h"
#include "resource.h"

extern HINSTANCE g_hInst;

/*!
 * @brief Copy of the mcast_settings object passed by the caller.
 * @details On this copy all the dialog operation is performed.
 */
static struct mcast_settings g_settings;

/*!
 * @brief Handle to the IP address control object.
 */ 
static HWND g_ipaddr_ctrl;

/*!
 * @brief Handle to the IP port edit control.
 */
static HWND g_ipport_edit_ctrl;

/*!
 * @brief Transfer from data to UI
 * @details Takes values from the settings object and presents them on the UI
 * @param[in] p_settings pointer to the settings object whose contents are to be presented on the screen.
 */
static void data_to_controls(struct mcast_settings const * p_settings)
{
	char buffer[8];
	StringCchPrintf(buffer, 8, "%hu", ntohs(p_settings->mcast_addr_.sin_port));
    SetWindowText(g_ipport_edit_ctrl, buffer);
}

/*!
 * @brief Handler for WM_INITDIALOG message.
 * @details This handler does as follows:
 * \li Initializes the control handles
 * \li Presents the settings on the UI
 * @param[in] hwnd handle to the window that received WM_INITDIALOG message
 * @param[in] hwndFocus handle to the Window that is to be got the keyboard focus upon dialog initalizing. 
 * @param[in] lParam client specific parameter passed to DialogBoxParam function. This is a way to pass to the
 * handler some client specific data.
 * @param returns TRUE if the window indicated as hWndFocus is to get keyboard focus. Returns FALSE otherwise.
 */
static BOOL Handle_wm_initdialog(HWND hwnd, HWND hWndFocus, LPARAM lParam)
{
    g_ipaddr_ctrl = GetDlgItem(hwnd, IDC_IPADDRESS1);
    assert(g_ipaddr_ctrl);
    g_ipport_edit_ctrl = GetDlgItem(hwnd, IDC_EDIT1);
    assert(g_ipport_edit_ctrl);

    data_to_controls(&g_settings);
    return TRUE;
} 

/**
 * @brief Multicast settings dialog message processing routine.
 * @details Processes the messages for the dialog, mainly the WM_COMMAND type.
 * @param hDlg
 * @param uMessage
 * @param wParam
 * @param lParam
 * @return  
 */
static INT_PTR CALLBACK McastSettingsProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
		switch (uMessage)
		{
				case WM_INITDIALOG:
						return HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, Handle_wm_initdialog);
				case WM_COMMAND:
						switch(wParam)
						{
								case IDCANCEL:
								case IDOK:
										EndDialog(hDlg, wParam);
										break;
						}
						return TRUE;
		}
		return FALSE;
}

int get_settings_from_dialog(HWND hParent, struct mcast_settings * p_settings)
{
		memcpy(&g_settings, p_settings, sizeof(struct mcast_settings));
		if (IDOK == DialogBox(g_hInst, MAKEINTRESOURCE(IDD_MCAST_SETTINGS), hParent, McastSettingsProc))
		{
				memcpy(p_settings, &g_settings, sizeof(struct mcast_settings));
				return 0;
		}
		return -1;
}

