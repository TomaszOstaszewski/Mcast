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
static HWND g_packet_delay_edit;
static HWND g_packet_length_edit;
static HWND g_packet_delay_spin;
static HWND g_packet_length_spin;

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
	switch (uMessage)
	{
		case WM_INITDIALOG:
			return HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, Handle_wm_initdialog);
		case WM_COMMAND:
			switch(wParam)
			{
				case IDC_MCAST_SETTINGS: 
					get_settings_from_dialog(g_hInst, hDlg, &g_settings->mcast_settings_);
					break;
				case IDCANCEL:
				case IDOK:
					{
						//int32_t pos = SendMessage(g_send_delay_slider, TBM_GETPOS, 0, 0);
						//debug_outputln("%s %d : %d\n", __FILE__, __LINE__, pos);
					}
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

