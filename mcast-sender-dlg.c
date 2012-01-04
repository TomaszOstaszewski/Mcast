/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file mcast-sender-dlg.c
 * @author
 * @date
 * @brief
 * @details
 */
#include "pcc.h"
#include "mcastui.h"
#include "conn_data.h"
#include "mcast_setup.h"
#include "resource.h"
#include "debug_helpers.h"
#include "dsoundplay.h"
#include "wave_utils.h"
#include "winsock_adapter.h"
#include "message-loop.h"
#include "mcast-sender-state-machine.h"

/**
 * @brief 
 */
HINSTANCE   g_hInst;

/**
 * @brief 
 */
master_riff_chunk_t * g_pWavChunk;

/**
 * @brief
 *
 */
static void UpdateUI(HWND hDlg)
{
    static sender_state_t prev_state = -1;
    sender_state_t curr_state = sender_get_current_state();
    if (prev_state != curr_state)
    {
        static HWND hSettingsBtn = NULL, hJoinMcastBtn = NULL, hLeaveMcast = NULL, hStartSendingBtn = NULL, hStopSendingBtn = NULL;
        static HMENU hMenu = NULL;
        if (NULL == hSettingsBtn)
            hSettingsBtn = GetDlgItem(hDlg, ID_SENDER_SETTINGS);    
        if (NULL == hJoinMcastBtn)
            hJoinMcastBtn = GetDlgItem(hDlg, ID_SENDER_JOINMCAST);  
        if (NULL == hStartSendingBtn)
            hStartSendingBtn = GetDlgItem(hDlg, ID_SENDER_STARTSENDING);    
        if (NULL == hStopSendingBtn)
            hStopSendingBtn = GetDlgItem(hDlg, ID_SENDER_STOPSENDING);  
        if (NULL == hLeaveMcast)
            hLeaveMcast = GetDlgItem(hDlg, ID_SENDER_LEAVEMCAST);
        if (NULL == hMenu)
            hMenu = GetMenu(hDlg);
        switch (curr_state)
        {
            case SENDER_INITIAL:
                EnableMenuItem(hMenu, ID_SENDER_SETTINGS, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMenu, ID_SENDER_JOINMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMenu, ID_SENDER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, ID_SENDER_STARTSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, ID_SENDER_STOPSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(hSettingsBtn, TRUE);
                EnableWindow(hJoinMcastBtn, TRUE);
                EnableWindow(hLeaveMcast, FALSE);
                EnableWindow(hStartSendingBtn, FALSE);
                EnableWindow(hStopSendingBtn, FALSE);
                break;
            case SENDER_MCAST_JOINED:
                EnableMenuItem(hMenu, ID_SENDER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, ID_SENDER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, ID_SENDER_LEAVEMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMenu, ID_SENDER_STARTSENDING, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMenu, ID_SENDER_STOPSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(hSettingsBtn, FALSE);
                EnableWindow(hJoinMcastBtn, FALSE);
                EnableWindow(hLeaveMcast, TRUE);
                EnableWindow(hStartSendingBtn, TRUE);
                EnableWindow(hStopSendingBtn, FALSE);
                break;
            case SENDER_SENDING:
                EnableMenuItem(hMenu, ID_SENDER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, ID_SENDER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, ID_SENDER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, ID_SENDER_STARTSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, ID_SENDER_STOPSENDING, MF_BYCOMMAND | MF_ENABLED);
                EnableWindow(hSettingsBtn, FALSE);
                EnableWindow(hJoinMcastBtn, FALSE);
                EnableWindow(hLeaveMcast, FALSE);
                EnableWindow(hStartSendingBtn, FALSE);
                EnableWindow(hStopSendingBtn, TRUE);
                break;
            default:
                break;
        }
        prev_state = curr_state;
    }
}


/**
 * @brief
 */
INT_PTR CALLBACK SenderDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    sender_state_t curr_state;
    switch (uMessage)
    {
        case WM_INITDIALOG:
            {
                int result = init_master_riff(&g_pWavChunk, g_hInst, MAKEINTRESOURCE(IDR_0_1));
                assert(0 == result);
                sender_initialize(g_pWavChunk);
            }
            return TRUE;
       case WM_COMMAND:
            switch(wParam)
            {
                case ID_SENDER_SETTINGS:
                    curr_state = sender_get_current_state();
                    if (SENDER_INITIAL == curr_state)
                    {
                        /* Open up the settings dialog with the MCAST settings parameters */
                    }
                    else
                    {
                        debug_outputln("%s %5.5d", __FILE__, __LINE__);
                    }
                    break;
                case ID_SENDER_JOINMCAST:
                    sender_handle_mcastjoin();
                    break;
                case ID_SENDER_LEAVEMCAST:
                    sender_handle_mcastleave();
                    break;
                case ID_SENDER_STARTSENDING:
                    sender_handle_startsending();
                    break;
                case ID_SENDER_STOPSENDING:
                    sender_handle_stopsending();
                    break;
                case IDOK:
                case IDCANCEL: 
                    EndDialog(hDlg, wParam);
                    PostQuitMessage(0);
                    break;
            }
            return TRUE;
    } 
    return FALSE;
}

static long int on_idle(HWND hWnd, long int count)
{
    switch (count)
    {
        case 0:
            UpdateUI(hWnd);
            return 1;
        default: 
            return 0;
    }
    return 0;
}


int PASCAL WinMain(  HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
{
    HWND hDlg;
    HRESULT hr;
    WSADATA             wsd;
    int	rc;

    /* Init Winsock */
    if ((rc = WSAStartup(MAKEWORD(1, 1), &wsd)) != 0)
        return FALSE;
    /* Init COM */
    hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        return FALSE;
    g_hInst = hInstance;
    //required to use the common controls
    InitCommonControls();
    hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN_SENDER), NULL, SenderDlgProc);
    if (NULL == hDlg)
        return (-1);
    message_loop(hDlg, &on_idle);
    return (int)0;
}

