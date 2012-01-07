/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file mcast-sender-dlg.c
 * @author T.Ostaszewski
 * @date 03-Jan-2011
 * @brief Sender dialog application main file.
 * @details 
 */
#include "pcc.h"
#include "resource.h"
#include "debug_helpers.h"
#include "winsock_adapter.h"
#include "message-loop.h"
#include "mcast-sender-state-machine.h"
#include "mcast-sender-settings-dlg.h"
#include "mcast-sender-settings.h"

/**
 * @brief Global Application instance.
 * @details Required for various Windows related stuff.
 */
static HINSTANCE   g_hInst;

/**
 * @brief 
 */
static struct mcast_sender * g_sender;

/**
 * @brief 
 */
static struct sender_settings g_settings;

/**
 * @brief 
 * @param hDlg
 * @param state
 */
static void UpdateUIwithCurrentState(HWND hDlg, sender_state_t state)
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
    switch (state)
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
            SetFocus(hJoinMcastBtn);
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
            SetFocus(hStartSendingBtn);
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
            SetFocus(hStopSendingBtn);
            break;
        default:
            break;
    }
}

/**
 * @brief Updates the UI with application state.
 * @param Just sets the focus to various controls and 
 * lights them up or ghosts them out.
 * @param hDlg
 */
static void UpdateUI(HWND hDlg)
{
    static sender_state_t prev_state = -1;
    sender_state_t curr_state;
    assert(g_sender);
    curr_state = sender_get_current_state(g_sender);
    if (prev_state != curr_state)
    {
        UpdateUIwithCurrentState(hDlg, curr_state);
        prev_state = curr_state;
    }
}

/**
 * @brief Sender dialog message processing routine.
 * @details Processes the messages for the dialog, mainly the WM_COMMAND type.
 * @param hDlg
 * @param uMessage
 * @param wParam
 * @param lParam
 * @return  
 */
static INT_PTR CALLBACK SenderDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    sender_state_t curr_state;
    switch (uMessage)
    {
        case WM_INITDIALOG:
            {
                int result;
                result = get_default_settings(g_hInst, &g_settings);
                assert(0 == result);
                g_sender = sender_create(&g_settings);
                assert(g_sender);
            }
            return TRUE;
       case WM_COMMAND:
            switch(wParam)
            {
                case ID_SENDER_SETTINGS:
                    curr_state = sender_get_current_state(g_sender);
                    if (SENDER_INITIAL == curr_state)
                    {
                        /* Open up the settings dialog with the MCAST settings parameters */
                        struct platform_specific_data platform = { g_hInst, hDlg };
                        if (IDOK == do_dialog(&platform, &g_settings))
                        {
                            sender_destroy(g_sender);
                            g_sender = sender_create(&g_settings);
                            assert(g_sender);
                        }
                    }
                    else
                    {
                        debug_outputln("%s %5.5d", __FILE__, __LINE__);
                    }
                    break;
                case ID_SENDER_JOINMCAST:
                    sender_handle_mcastjoin(g_sender);
                    break;
                case ID_SENDER_LEAVEMCAST:
                    sender_handle_mcastleave(g_sender);
                    break;
                case ID_SENDER_STARTSENDING:
                    sender_handle_startsending(g_sender);
                    break;
                case ID_SENDER_STOPSENDING:
                    sender_handle_stopsending(g_sender);
                    break;
                case IDOK:
                case IDCANCEL: 
                    sender_destroy(g_sender);
                    DestroyWindow(hDlg);
                    break;
            }
            return TRUE;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    } 
    return FALSE;
}

/**
 * @brief Idle processing routine.
 * @details This routine will be called each time there is 
 * no message in the message queue. When it returns a non-zero value,
 * it will be called again, and so on, unless a message shows up in the message
 * queue, or it returns 0. 
 * @param hWnd
 * @param count
 * @return When it returns 0, it wont' be called again until a message shows up
 * in the message queue and its processed. When it returns a non-zero, it may be called again,
 * unless a message shows up in the message queue.
 */
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

/*!
 * @brief Windows entry point.
 * @details
 * @param hInstance
 * @param hPrevInstance
 * @param lpCmdLine
 * @param nCmdShow
 * @return
 */
int PASCAL WinMain(HINSTANCE hInstance,
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

