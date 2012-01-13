/*!
 * @brief
 * @file sender-settings-dlg.c
 * @author T.Ostaszewski
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
static struct sender_settings_bounds * g_bounds;
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
	g_bounds = get_settings_bounds();
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
	switch (uMessage)
	{
		case WM_INITDIALOG:
			return HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, Handle_wm_initdialog);
		case WM_NOTIFY:
			p_notify_header = (NMHDR*)lParam;
			{
				NMUPDOWN * p_up_down = (NMUPDOWN *)p_notify_header;
				switch (p_notify_header->code)
				{
					case UDN_DELTAPOS:
						if (g_packet_delay_spin == p_notify_header->hwndFrom)
						{
							uint16_t new_send_delay;
							new_send_delay = g_settings->send_delay_;
							if ((int)new_send_delay > p_up_down->iDelta)
							{
								new_send_delay -= p_up_down->iDelta;
								if (new_send_delay <= g_bounds->max_packet_delay_ && new_send_delay >= g_bounds->min_packet_delay_)
								{
									g_settings->send_delay_ = new_send_delay;	
								}
							}
						}
						else if (g_packet_length_spin == p_notify_header->hwndFrom)
						{
							uint16_t new_packet_length;
							new_packet_length = g_settings->chunk_size_;
							if ((int)new_packet_length > p_up_down->iDelta)
							{
								new_packet_length -= p_up_down->iDelta;
								if (new_packet_length <= g_bounds->max_chunk_size_ && new_packet_length >= g_bounds->min_chunk_size_)
								{
									g_settings->chunk_size_ = new_packet_length;	
								}
							}
						}
						else
							debug_outputln("%s %d", __FILE__, __LINE__);
						data_to_controls(g_settings);
						break;
				} 	
			}
			break;
		case WM_COMMAND:
			switch(wParam)
			{
				case IDC_MCAST_SETTINGS: 
					get_settings_from_dialog(g_hInst, hDlg, &g_settings->mcast_settings_);
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
	return DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_SENDER_SETTINGS), hWndParent, McastSettingsProc, (LPARAM)p_settings);
}

