/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file mcast-sender-dlg.c
 * @brief Sender dialog application main file.
 * @details 
 * @date 03-Jan-2011
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
 */
#include "pcc.h"
#include "debug_helpers.h"
#include "message-loop.h"
#include "mcast-sender-state-machine.h"
#include "sender-settings-dlg.h"
#include "sender-settings.h"
#include "about-dialog.h"
#include "sender-res.h"

/*!
 * @brief Describes all the 'interesting' UI controls of the sender's main dialog.
 */
struct sender_dialog {
    HWND hDlg_; /*!< Handle to the main dialog */
    HWND hSettingsBtn; /*!< Handle to the 'Settings' button. */
    HWND hJoinMcastBtn; /*!< Handle to the 'Join Multicast' button. */
    HWND hLeaveMcast; /*!< Handle to the 'Join Multicast' button. */
    HWND hStartSendingBtn; /*!< Handle to the 'Start sending' button. */
    HWND hStopSendingBtn; /*!< Handle to the 'Stop sending' button. */
    HWND hTestToneCheck; /*!< Handle to the 'Test tone' check button. */
    HWND hOpenWav; /*!< Handle to the 'Open WAV ...' box. */
    HMENU hMainMenu; /*!< Handle to the main menu. */
    struct mcast_sender * sender_; /*!< The sender object. This sends out the data on the multicast group. */
    struct sender_settings settings_; /*!< The sender settings object. Here are multicast group settings stored, and how many bytes to send with each packet. */
    BOOL bPlayWav_; /*!< Whether or not to play the selected WAV or the test tone */
    HINSTANCE hInst_; /*!< @brief Global Application instance.  Required for various Windows related stuff. */
};

/**
 * @brief A pointer to the sender dialog object. This pointer is used in all the dialog routines.
 */
static struct sender_dialog * g_sender_dlg;

/**
 * @brief Entry point for the user interface widgets update.
 * @details Compares the previous and current sender state.
 * @param hDlg handle to the main dialog window, holding the user interface widgets.
 */
static void UpdateUI(HWND hDlg)
{
    static sender_state_t prev_state = -1;
    sender_state_t curr_state;
    assert(g_sender_dlg);
    curr_state = sender_get_current_state(g_sender_dlg->sender_);
    if (prev_state != curr_state)
    {
        switch (curr_state)
        {
            case SENDER_INITIAL:
                EnableMenuItem(g_sender_dlg->hMainMenu, ID_SENDER_SETTINGS, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(g_sender_dlg->hMainMenu, ID_SENDER_JOINMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(g_sender_dlg->hMainMenu, ID_SENDER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(g_sender_dlg->hMainMenu, ID_SENDER_STARTSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(g_sender_dlg->hMainMenu, ID_SENDER_STOPSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(g_sender_dlg->hSettingsBtn, TRUE);
                EnableWindow(g_sender_dlg->hJoinMcastBtn, TRUE);
                EnableWindow(g_sender_dlg->hLeaveMcast, FALSE);
                EnableWindow(g_sender_dlg->hStartSendingBtn, FALSE);
                EnableWindow(g_sender_dlg->hStopSendingBtn, FALSE);
                EnableWindow(g_sender_dlg->hTestToneCheck, TRUE);
                SetFocus(g_sender_dlg->hJoinMcastBtn);
                break;
            case SENDER_MCAST_JOINED:
                EnableMenuItem(g_sender_dlg->hMainMenu, ID_SENDER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(g_sender_dlg->hMainMenu, ID_SENDER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(g_sender_dlg->hMainMenu, ID_SENDER_LEAVEMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(g_sender_dlg->hMainMenu, ID_SENDER_STARTSENDING, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(g_sender_dlg->hMainMenu, ID_SENDER_STOPSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(g_sender_dlg->hSettingsBtn, FALSE);
                EnableWindow(g_sender_dlg->hJoinMcastBtn, FALSE);
                EnableWindow(g_sender_dlg->hLeaveMcast, TRUE);
                EnableWindow(g_sender_dlg->hStartSendingBtn, TRUE);
                EnableWindow(g_sender_dlg->hStopSendingBtn, FALSE);
                EnableWindow(g_sender_dlg->hTestToneCheck, FALSE);
                SetFocus(g_sender_dlg->hStartSendingBtn);
                break;
            case SENDER_SENDING:
                EnableMenuItem(g_sender_dlg->hMainMenu, ID_SENDER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(g_sender_dlg->hMainMenu, ID_SENDER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(g_sender_dlg->hMainMenu, ID_SENDER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(g_sender_dlg->hMainMenu, ID_SENDER_STARTSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(g_sender_dlg->hMainMenu, ID_SENDER_STOPSENDING, MF_BYCOMMAND | MF_ENABLED);
                EnableWindow(g_sender_dlg->hSettingsBtn, FALSE);
                EnableWindow(g_sender_dlg->hJoinMcastBtn, FALSE);
                EnableWindow(g_sender_dlg->hLeaveMcast, FALSE);
                EnableWindow(g_sender_dlg->hStartSendingBtn, FALSE);
                EnableWindow(g_sender_dlg->hStopSendingBtn, TRUE);
                EnableWindow(g_sender_dlg->hTestToneCheck, FALSE);
                SetFocus(g_sender_dlg->hStopSendingBtn);
                break;
            default:
                break;
        } 
        prev_state = curr_state;
    }
    EnableWindow(g_sender_dlg->hOpenWav, g_sender_dlg->bPlayWav_);
    Button_SetCheck(g_sender_dlg->hTestToneCheck, !g_sender_dlg->bPlayWav_);
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
static BOOL Handle_wm_initdialog(HWND hDlg, HWND hWndFocus, LPARAM lParam)
{
    int result;
    struct sender_dialog * p_dlg;
    g_sender_dlg = (struct sender_dialog*)lParam;
    p_dlg = g_sender_dlg;
    assert(p_dlg);
    p_dlg->hDlg_ = hDlg;
    assert(p_dlg->hDlg_);
    p_dlg->hSettingsBtn = GetDlgItem(hDlg, ID_SENDER_SETTINGS);    
    assert(p_dlg->hSettingsBtn);
    p_dlg->hJoinMcastBtn = GetDlgItem(hDlg, ID_SENDER_JOINMCAST);  
    assert(p_dlg->hJoinMcastBtn);
    p_dlg->hStartSendingBtn = GetDlgItem(hDlg, ID_SENDER_STARTSENDING);    
    assert(p_dlg->hStartSendingBtn);
    p_dlg->hStopSendingBtn = GetDlgItem(hDlg, ID_SENDER_STOPSENDING);  
    assert(p_dlg->hStopSendingBtn);
    p_dlg->hLeaveMcast = GetDlgItem(hDlg, ID_SENDER_LEAVEMCAST);
    assert(p_dlg->hLeaveMcast );
    p_dlg->hMainMenu = GetMenu(hDlg);
    assert(p_dlg->hMainMenu);
    p_dlg->hTestToneCheck = GetDlgItem(hDlg, ID_TEST_TONE);
    assert(p_dlg->hTestToneCheck);
    p_dlg->hOpenWav = GetDlgItem(hDlg, ID_OPEN_WAV);
    assert(p_dlg->hOpenWav);
    result = get_default_settings(&g_sender_dlg->settings_);
    assert(result);
    g_sender_dlg->sender_ = sender_create(&g_sender_dlg->settings_);
    assert(g_sender_dlg->sender_);
    UpdateUI(hDlg);
    return TRUE;
}
 
/**
 * @brief Sender dialog message processing routine.
 * @details Processes the messages for the dialog, mainly the WM_COMMAND type.
 * @param hDlg handle to the dialog window
 * @param uMessage message to be processed
 * @param wParam message specific parameter
 * @param lParam message specific parameter
 * @return  
 */
static INT_PTR CALLBACK SenderDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    sender_state_t curr_state;
    switch (uMessage)
    {
        case WM_INITDIALOG:
           return HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, Handle_wm_initdialog);
       case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case ID_SENDER_SETTINGS:
                    assert(g_sender_dlg);
                    curr_state = sender_get_current_state(g_sender_dlg->sender_);
                    if (SENDER_INITIAL == curr_state)
                    {
                        /* Open up the settings dialog with the MCAST settings parameters */
                        if (sender_settings_from_dialog(hDlg, &g_sender_dlg->settings_))
                        {
                            sender_destroy(g_sender_dlg->sender_);
                            g_sender_dlg->sender_ = sender_create(&g_sender_dlg->settings_);
                            assert(g_sender_dlg->sender_);
                        }
                    }
                    else
                    {
                        debug_outputln("%s %4.4u", __FILE__, __LINE__);
                    }
                    break;
                case ID_SENDER_JOINMCAST:
                    sender_handle_mcastjoin(g_sender_dlg->sender_);
                    break;
                case ID_SENDER_LEAVEMCAST:
                    sender_handle_mcastleave(g_sender_dlg->sender_);
                    break;
                case ID_SENDER_STARTSENDING:
                    sender_handle_startsending(g_sender_dlg->sender_);
                    break;
                case ID_SENDER_STOPSENDING:
                    sender_handle_stopsending(g_sender_dlg->sender_);
                    break;
                case ID_TEST_TONE:
                    g_sender_dlg->bPlayWav_ = !g_sender_dlg->bPlayWav_;
                    break;
                case ID_OPEN_WAV:
                    debug_outputln("%s %4.4u", __FILE__, __LINE__);
                    break;
               case IDM_SENDER_ABOUT:
                    display_about_dialog(hDlg);
                    break;
                case IDOK:
                case IDCANCEL: 
                    sender_destroy(g_sender_dlg->sender_);
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
 * @param hWnd handle to the window whose message loop is idle.
 * @param count The idle cycle counter. Each time a message loop enters idle condition, i.e. there are no messages in the looop, the on_idle() function is called. Each time it is called, this parameter is incremented. This parameter is zeroed each time a message appears in the message queue.
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
    WSADATA wsd;
    int	rc;
    struct sender_dialog sender_dlg;
    ZeroMemory(&sender_dlg, sizeof(sender_dlg));
    /* Init Winsock */
    if ((rc = WSAStartup(MAKEWORD(1, 1), &wsd)) != 0)
        return FALSE;
    /* Init COM */
    hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        return FALSE;
    sender_dlg.hInst_ = hInstance;
    //required to use the common controls
    InitCommonControls();
    
    hDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_MAIN_SENDER), NULL, SenderDlgProc, (LPARAM)&sender_dlg);
    if (NULL == hDlg)
        return (-1);
    message_loop(hDlg, &on_idle);
    return (int)0;
}

