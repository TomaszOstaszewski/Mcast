#include "pcc.h"
#include "resource.h"
#include "mcast-sender-settings-dlg.h"

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

void do_dialog(HINSTANCE hInst, HWND hParent)
{
	DialogBox(hInst, MAKEINTRESOURCE(IDD_SENDER_SETTINGS), hParent, McastSettingsProc);
}
 
