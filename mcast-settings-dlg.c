/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/*!
 * @file mcast-settings-dlg.c
 */
#include "pcc.h"
#include "mcast-settings-dlg.h"
#include "resource.h"

extern HINSTANCE g_hInst;

static struct sender_settings * p_settings;

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

int get_settings_from_dialog(HINSTANCE hInst, HWND hParent, struct mcast_settings * p_settings)
{
	return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_MCAST_SETTINGS), hParent, McastSettingsProc, (LPARAM)p_settings);
}

