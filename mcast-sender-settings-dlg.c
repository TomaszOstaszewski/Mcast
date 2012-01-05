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
			p_settings = (struct sender_settings*)lParam;
			p_settings->ipv4_mcast_group_addr_ = inet_addr(DEFAULT_MCASTADDRV4);
			p_settings->mcast_port_ = 25000;
			p_settings->send_delay_ = 88;
			break;
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

void do_dialog(struct platform_specific_data const * p_platform, struct sender_settings * p_settings)
{
	DialogBoxParam(p_platform->hInst_, MAKEINTRESOURCE(IDD_SENDER_SETTINGS), p_platform->hParent_, McastSettingsProc, (LPARAM)p_settings);
}

