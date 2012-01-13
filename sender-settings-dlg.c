/*!
 * @brief
 * @file sender-settings-dlg.c
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
 * @date Jan-2012
 */
#include "pcc.h"
#include "debug_helpers.h"
#include "resource.h"
#include "sender-settings-dlg.h"
#include "mcast-settings-dlg.h"
#include "sender-settings.h"
#include "mcast-settings.h"

extern HINSTANCE g_hInst;

/*!
 * @brief
 */
static struct sender_settings * g_settings;
static HWND g_packet_delay_edit;
static HWND g_packet_length_edit;
static HWND g_packet_delay_spin;
static HWND g_packet_length_spin;

static void data_to_controls(struct sender_settings const * p_settings)
{
	char text_buf[8];
	StringCchPrintf(text_buf, 8, "%d", g_settings->send_delay_);
	SetWindowText(g_packet_delay_edit, text_buf);
	StringCchPrintf(text_buf, 8, "%d", g_settings->chunk_size_);
	SetWindowText(g_packet_length_edit, text_buf);
}

static BOOL Handle_wm_initdialog(HWND hwnd, HWND hWndFocus, LPARAM lParam)
{
	g_settings = (struct sender_settings*)lParam;
	g_packet_delay_edit = GetDlgItem(hwnd, IDC_PACKET_DELAY_EDIT);
	g_packet_length_edit = GetDlgItem(hwnd, IDC_PACKET_LENGTH_EDIT);
	g_packet_delay_spin = GetDlgItem(hwnd, IDC_PACKET_DELAY_SPIN);
	g_packet_length_spin = GetDlgItem(hwnd, IDC_PACKET_LENGTH_SPIN);
	assert(g_packet_delay_edit);
	assert(g_packet_length_edit);
	assert(g_packet_delay_spin);
	assert(g_packet_length_spin);
	SendMessage(g_packet_delay_spin, UDM_SETBUDDY, (WPARAM)g_packet_delay_edit, (LPARAM)0);
	SendMessage(g_packet_length_spin, UDM_SETBUDDY, (WPARAM)g_packet_length_edit, (LPARAM)0);
	SendMessage(g_packet_delay_spin, UDM_SETPOS, (WPARAM)0, (LPARAM)0);
	SendMessage(g_packet_length_spin, UDM_SETPOS, (WPARAM)0, (LPARAM)0);
	data_to_controls(g_settings);
	return FALSE;
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
	NMHDR * p_notify_header;
	NMUPDOWN * p_up_down;
	struct sender_settings sender_settings_copy;
	switch (uMessage)
	{
		case WM_INITDIALOG:
			return HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, Handle_wm_initdialog);
		case WM_NOTIFY:
			p_notify_header = (NMHDR*)lParam;
			{
				p_up_down = (NMUPDOWN *)p_notify_header;
				switch (p_notify_header->code)
				{
					case UDN_DELTAPOS:
						memcpy(&sender_settings_copy, g_settings, sizeof(struct sender_settings));
						if (g_packet_delay_spin == p_notify_header->hwndFrom)
						{
							sender_settings_copy.send_delay_ -= p_up_down->iDelta;
						}
						else if (g_packet_length_spin == p_notify_header->hwndFrom)
						{
							sender_settings_copy.chunk_size_ -= p_up_down->iDelta;
						}
						else
						{
							break;
						}
						if (sender_settings_validate(&sender_settings_copy))
						{
							memcpy(g_settings, &sender_settings_copy, sizeof(struct sender_settings));
							data_to_controls(g_settings);
						}
						break;
					default:
						break;
				}
			}
			break;
		case WM_COMMAND:
			switch(wParam)
			{
				case IDC_MCAST_SETTINGS:
					get_settings_from_dialog(hDlg, &g_settings->mcast_settings_);
					break;
				case IDCANCEL:
				case IDOK:
					EndDialog(hDlg, wParam);
					break;
			}
			return TRUE;
	}
	return FALSE;
}

int do_dialog(HWND hWndParent, struct sender_settings * p_settings)
{
	struct sender_settings settings_copy;
	memcpy(&settings_copy, p_settings, sizeof(struct sender_settings));
	if (IDOK == DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_SENDER_SETTINGS), hWndParent, McastSettingsProc, (LPARAM)&settings_copy))
	{
		memcpy(p_settings, &settings_copy, sizeof(struct sender_settings));
		return 0;
	}
	return 1;
}

