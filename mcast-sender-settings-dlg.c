/*!
 * @brief
 * @file mcast-sender-settings-dlg.c
 * @author T.Ostaszewski
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
 * @date Jan-2012
 */
#include "pcc.h"
#include "debug_helpers.h"
#include "resource.h"
#include "mcast-sender-settings-dlg.h"

/*!
 *
 */
#define DEFAULT_MCASTADDRV4    "234.5.6.7"

/*!
 *
 */
#define DEFAULT_MCASTADDRV6    "ff12::1"

/*!
 *
 */
#define DEFAULT_MCASTPORT      "25000"

/*!
 *
 */
#define DEFAULT_TTL    (8)


static BOOL Handle_wm_initdialog(HWND hwnd, HWND hWndFocus, LPARAM lParam)
{
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
	static struct sender_settings * p_settings;
	switch (uMessage)
	{
		case WM_INITDIALOG:
			HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, Handle_wm_initdialog);
			p_settings = (struct sender_settings*)lParam;
			p_settings->ipv4_mcast_group_addr_ = inet_addr(DEFAULT_MCASTADDRV4);
			p_settings->mcast_port_ = 25000;
			p_settings->send_delay_ = 88;
			return TRUE;
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

int do_dialog(struct platform_specific_data const * p_platform, struct sender_settings * p_settings)
{
	return DialogBoxParam(p_platform->hInst_, MAKEINTRESOURCE(IDD_SENDER_SETTINGS), p_platform->hParent_, McastSettingsProc, (LPARAM)p_settings);
}

