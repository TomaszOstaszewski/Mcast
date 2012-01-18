/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file mcast-receiver-dlg.c
 * @author T. Ostaszewski
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
 * @date 04-Jan-2012
 * @brief Receiver's entry point.
 * @details This file contains the receiver's dialog front end. This is
 * where the UI meets the internal state management.
 */
#include "pcc.h"
#include "mcast_setup.h"
#include "resource.h" 
#include "debug_helpers.h"
#include "dsoundplay.h"
#include "fifo-circular-buffer.h"
#include "wave_utils.h"
#include "mcast-receiver-state-machine.h"
#include "message-loop.h"
#include "mcast-settings.h"
#include "receiver-settings-dlg.h"
#include "receiver-settings.h"

/**
 * 
 */
static struct mcast_receiver * g_receiver;

/**
 * 
 */
static struct receiver_settings g_settings;

/**
 * @brief Global Application instance.
 * @details Required for various Windows related stuff.
 */
HINSTANCE   g_hInst;

struct ui_controls {
    HWND hSettingsBtn;
    HWND hJoinMcastBtn;
    HWND hLeaveMcast;
    HWND hPlay;
    HWND hStop;
    HWND hStartRcv;
    HWND hStopRcv;
    HWND hProgressBar;
    HMENU hMainMenu;
} g_controls;

/**
 * @brief Helper UI update function.
 * @details Updates the UI widgets state so they reflect the internal state of the program.
 * Mainly lights up and ghosts out various controls.
 * @param[in] hDlg a handle to the window to be updated
 * @param[in] curr_state current receiver state. Control on the dialog passed with hDlg will be updated accordingly.
 */
static void UpdateUIwithCurrentState(struct ui_controls * p_controls, receiver_state_t curr_state)
{
    switch (curr_state)
    {
        case RECEIVER_INITIAL:
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_GRAYED);
            EnableWindow(p_controls->hSettingsBtn, TRUE);
            EnableWindow(p_controls->hJoinMcastBtn, TRUE);
            EnableWindow(p_controls->hLeaveMcast, FALSE);
            EnableWindow(p_controls->hPlay, TRUE);
            EnableWindow(p_controls->hStop, FALSE);
            EnableWindow(p_controls->hStartRcv, FALSE);
            EnableWindow(p_controls->hStopRcv, FALSE);
            SetFocus(p_controls->hJoinMcastBtn);
            break;
        case RECEIVER_MCASTJOINED:
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_GRAYED);
            EnableWindow(p_controls->hSettingsBtn, FALSE);
            EnableWindow(p_controls->hJoinMcastBtn, FALSE);
            EnableWindow(p_controls->hLeaveMcast, TRUE);
            EnableWindow(p_controls->hPlay, TRUE);
            EnableWindow(p_controls->hStop, FALSE);
            EnableWindow(p_controls->hStartRcv, TRUE);
            EnableWindow(p_controls->hStopRcv, FALSE);
            SetFocus(p_controls->hStartRcv);
            break;
        case RECEIVER_PLAYING:
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_DISABLED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_ENABLED);
            EnableWindow(p_controls->hSettingsBtn, FALSE);
            EnableWindow(p_controls->hJoinMcastBtn, TRUE);
            EnableWindow(p_controls->hLeaveMcast, FALSE);
            EnableWindow(p_controls->hPlay, FALSE);
            EnableWindow(p_controls->hStop, TRUE);
            EnableWindow(p_controls->hStartRcv, FALSE);
            EnableWindow(p_controls->hStopRcv, FALSE);
            SetFocus(p_controls->hStop);
            break;
        case RECEIVER_MCASTJOINED_PLAYING:
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_GRAYED);
            EnableWindow(p_controls->hSettingsBtn, FALSE);
            EnableWindow(p_controls->hJoinMcastBtn, FALSE);
            EnableWindow(p_controls->hLeaveMcast, TRUE);
            EnableWindow(p_controls->hPlay, FALSE);
            EnableWindow(p_controls->hStop, TRUE);
            EnableWindow(p_controls->hStartRcv, TRUE);
            EnableWindow(p_controls->hStopRcv, FALSE);
            SetFocus(p_controls->hStartRcv);
            break;
        case RECEIVER_RECEIVING:
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_GRAYED);
            EnableWindow(p_controls->hSettingsBtn, FALSE);
            EnableWindow(p_controls->hJoinMcastBtn, FALSE);
            EnableWindow(p_controls->hLeaveMcast, FALSE);
            EnableWindow(p_controls->hPlay, TRUE);
            EnableWindow(p_controls->hStop, FALSE);
            EnableWindow(p_controls->hStartRcv, FALSE);
            EnableWindow(p_controls->hStopRcv, TRUE);
            SetFocus(p_controls->hPlay);
            break;
        case RECEIVER_RECEIVING_PLAYING:
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(p_controls->hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_ENABLED);
            EnableWindow(p_controls->hSettingsBtn, FALSE);
            EnableWindow(p_controls->hJoinMcastBtn, FALSE);
            EnableWindow(p_controls->hLeaveMcast, FALSE);
            EnableWindow(p_controls->hPlay, FALSE);
            EnableWindow(p_controls->hStop, TRUE);
            EnableWindow(p_controls->hStartRcv, FALSE);
            EnableWindow(p_controls->hStopRcv, TRUE);
            SetFocus(p_controls->hStop);
            break;
        default:
            break;
    }
}
/**
 * @brief Main UI update function.
 * @details Updates the UI widgets if the reciever state changes (i.e. previously stored state is different than current one).
 * @param[in] hDlg a handle to the dialog window to be updated
 * @attention This is usually called by the application's idle handler.
 */
static void UpdateUI(struct ui_controls * p_controls)
{
    static receiver_state_t prev_state = RECEIVER_INITIAL;
    receiver_state_t new_state = receiver_get_state(g_receiver);
    if (prev_state != new_state)
    {
        UpdateUIwithCurrentState(p_controls, new_state);
        prev_state = new_state;
    }
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
static BOOL Handle_wm_initdialog(HWND hwnd, HWND hWndFocus, LPARAM lParam)
{
    int result;
    result = receiver_settings_get_default(g_hInst, &g_settings);
    assert(result);
    if (result)
    {
        struct ui_controls * p_controls = &g_controls;
        g_receiver = receiver_create(&g_settings);
        assert(g_receiver);
        p_controls->hSettingsBtn = GetDlgItem(hwnd, ID_RECEIVER_SETTINGS);    
        p_controls->hJoinMcastBtn = GetDlgItem(hwnd, ID_RECEIVER_JOINMCAST);  
        p_controls->hPlay = GetDlgItem(hwnd, ID_RECEIVER_PLAY);    
        p_controls->hStop = GetDlgItem(hwnd, ID_RECEIVER_STOP);  
        p_controls->hLeaveMcast = GetDlgItem(hwnd, ID_RECEIVER_LEAVEMCAST);
        p_controls->hStopRcv = GetDlgItem(hwnd, ID_RECEIVER_STOPRCV);
        p_controls->hStartRcv = GetDlgItem(hwnd, ID_RECEIVER_STARTRCV);
        p_controls->hProgressBar = GetDlgItem(hwnd, IDC_RECEIVER_BUFFER_FILL);
        p_controls->hMainMenu = GetMenu(hwnd);
        assert(p_controls->hSettingsBtn);
        assert(p_controls->hJoinMcastBtn);
        assert(p_controls->hPlay);
        assert(p_controls->hStop);
        assert(p_controls->hLeaveMcast);
        assert(p_controls->hStopRcv);
        assert(p_controls->hStartRcv);
        assert(p_controls->hMainMenu);
        assert(p_controls->hProgressBar);
        SendMessage(p_controls->hProgressBar, PBM_SETRANGE, 0, MAKEWPARAM(0, 65535));
        SetTimer(hwnd, IDT_TIMER_1, 250, NULL);
        UpdateUIwithCurrentState(p_controls, receiver_get_state(g_receiver));
    }
    else 
    {
        EndDialog(hwnd, IDCANCEL);
    }
    return FALSE; /* We return FALSE, as we do call SetFocus() ourselves */
}

/**
 * @brief Receiver dialog message processing routine.
 * @details Processes the dialog messages, mainly the WM_COMMMAND style ones.
 * @param hDlg
 * @param uMessage
 * @param wParam
 * @param lParam
 */
static INT_PTR CALLBACK ReceiverDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage)
    {
        case WM_INITDIALOG:
            return HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, Handle_wm_initdialog);
        case WM_COMMAND:
            switch (wParam)
            {
                case ID_RECEIVER_SETTINGS:
                    if (RECEIVER_INITIAL == receiver_get_state(g_receiver) && receiver_settings_do_dialog(hDlg, &g_settings))
                    {
                        int result;
                        result = receiver_destroy(g_receiver); 
                        assert(result);
                        if (result)
                        {
                            g_receiver = receiver_create(&g_settings);
                            assert(g_receiver);
                        }
                    }
                    else
                    {
                        debug_outputln("%s %5.5d", __FILE__, __LINE__);
                    }
                    break;
                case ID_RECEIVER_JOINMCAST:
                    handle_mcastjoin(g_receiver);
                    break;
                case ID_RECEIVER_LEAVEMCAST:
                    handle_mcastleave(g_receiver);
                    break;
                case ID_RECEIVER_PLAY:
                    handle_play(g_receiver, hDlg);
                    break;
                case ID_RECEIVER_STOP:
                    handle_stop(g_receiver);
                    break;
                case ID_RECEIVER_STARTRCV:
                    handle_rcvstart(g_receiver);
                    break;
                case ID_RECEIVER_STOPRCV:
                    handle_rcvstop(g_receiver);
                    break;
                case IDOK:
                case IDCANCEL:
                    DestroyWindow(hDlg);
                    break;
            }
            return TRUE;
        case WM_TIMER:
            switch (wParam)
            {
                case IDT_TIMER_1:
                    debug_outputln("%s %d : %u", __FILE__, __LINE__, fifo_circular_buffer_get_items_count(receiver_get_fifo(g_receiver)));
                    SendMessage(g_controls.hProgressBar, PBM_SETPOS, fifo_circular_buffer_get_items_count(receiver_get_fifo(g_receiver)), 0);
                    break;
                default:
                    break;
            }
            break;
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
            UpdateUI(&g_controls);
            return 1;
        default:
            return 0;
    }
    return 0;
}

/**
 * @brief Recievers entry point.
 * @details This is the entry point of the receivers application.
 * @param hInstance
 * @param hPrevInstance
 * @param lpCmdLine
 * @param nCmdShow
 * @return
 */
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND hMainDlg;
    HRESULT hr;
    WSADATA wsd;
    int	rc;

    /* Init Winsock */
    if ((rc = WSAStartup(MAKEWORD(1, 1), &wsd)) != 0)
        return FALSE;
    hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        return FALSE;
    g_hInst = hInstance;
    //required to use the common controls
    InitCommonControls();
    hMainDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN_RECEIVER), NULL, ReceiverDlgProc);
    if (NULL == hMainDlg)
        return (-1);
    message_loop(hMainDlg, &on_idle);
    return (int)0;
}

