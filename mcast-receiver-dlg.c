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
#include "debug_helpers.h"
#include "dsoundplay.h"
#include "circular-buffer-uint8.h"
#include "wave_utils.h"
#include "mcast-receiver-state-machine.h"
#include "message-loop.h"
#include "mcast-settings.h"
#include "receiver-settings-dlg.h"
#include "receiver-settings.h"
#include "about-dialog.h"
#include "common-dialogs-res.h"
#include "receiver-res.h"

/*!
 * @brief Maximum number of characters to be placed into the 'buffer bytes' edit control.
 * @details Enugh space for 5 digits + 1 terminating NULL.
 */
#define BUFFER_BYTES_EDIT_TEXT_LIMIT (5+1)

/*!
 * @brief Describes all the 'interesting' UI controls on the main dialog.
 */
struct reciever_dialog {
    HINSTANCE hInst_;
    HWND hDlg_; /*!< Handle to the dialog window. */
    HWND hSettingsBtn; /*!< Handle to the 'Settings' button. */
    HWND hJoinMcastBtn; /*!< Handle to the 'Join Multicast' button. */
    HWND hLeaveMcast; /*!< Handle to the 'Join Multicast' button. */
    HWND hPlay; /*!< Handle to the 'Play' button. */
    HWND hStop; /*!< Handle to the 'Stop' button. */
    HWND hStartRcv; /*!< Handle to the 'Start receiver' button. */
    HWND hStopRcv; /*!< Handle to the 'Stop receiver' button. */
    HWND hProgressBar; /*!< Handle to the progress bar control. */
    HWND hBufferBytesEdit; /*!< Handle to the edit control that displays bytes in the buffer. */
    HMENU hMainMenu; /*!< Handle to the main menu. */
    struct receiver_settings settings_;
    struct mcast_receiver * receiver_;
    TCHAR buffer_bytes_edit_[BUFFER_BYTES_EDIT_TEXT_LIMIT]; /*!< Buffer that holds string to be displayed in the 'buffer bytes' control. */
};

/*!
 * @brief Defines time how often the UI is updated.
 */
#define UI_UPDATE_TIMER_MS (500)

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
    struct reciever_dialog * p_dlg;
    p_dlg = (struct reciever_dialog *)lParam;
    assert(p_dlg);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)p_dlg); 
    result = receiver_settings_get_default(&p_dlg->settings_);
    assert(result);
    if (result)
    {
        p_dlg->receiver_ = receiver_create(&p_dlg->settings_);
        assert(p_dlg->receiver_);
        p_dlg->hDlg_ = hwnd;
        p_dlg->hSettingsBtn = GetDlgItem(hwnd, ID_RECEIVER_SETTINGS);    
        p_dlg->hJoinMcastBtn = GetDlgItem(hwnd, ID_RECEIVER_JOINMCAST);  
        p_dlg->hPlay = GetDlgItem(hwnd, ID_RECEIVER_PLAY);    
        p_dlg->hStop = GetDlgItem(hwnd, ID_RECEIVER_STOP);  
        p_dlg->hLeaveMcast = GetDlgItem(hwnd, ID_RECEIVER_LEAVEMCAST);
        p_dlg->hStopRcv = GetDlgItem(hwnd, ID_RECEIVER_STOPRCV);
        p_dlg->hStartRcv = GetDlgItem(hwnd, ID_RECEIVER_STARTRCV);
        p_dlg->hProgressBar = GetDlgItem(hwnd, IDC_RECEIVER_BUFFER_FILL);
        p_dlg->hBufferBytesEdit = GetDlgItem(hwnd, IDC_BUFFER_BYTES_EDIT);
        p_dlg->hMainMenu = GetMenu(hwnd);
        assert(p_dlg->hSettingsBtn);
        assert(p_dlg->hJoinMcastBtn);
        assert(p_dlg->hPlay);
        assert(p_dlg->hStop);
        assert(p_dlg->hLeaveMcast);
        assert(p_dlg->hStopRcv);
        assert(p_dlg->hStartRcv);
        assert(p_dlg->hProgressBar);
        assert(p_dlg->hBufferBytesEdit);
        assert(p_dlg->hMainMenu);
        SendMessage(p_dlg->hProgressBar, PBM_SETRANGE32, 0, fifo_circular_buffer_get_capacity(receiver_get_fifo(p_dlg->receiver_)));
        SendMessage(p_dlg->hBufferBytesEdit, EM_SETLIMITTEXT, (WPARAM)BUFFER_BYTES_EDIT_TEXT_LIMIT, (LPARAM)0);
        SetTimer(hwnd, IDT_TIMER_1, UI_UPDATE_TIMER_MS , NULL);
    }
    else 
    {
        EndDialog(hwnd, IDCANCEL);
    }
    return TRUE;
}

static void update_fifo_receiver_bytes_edit_control(struct reciever_dialog * p_dlg, uint32_t items_count)
{
    StringCchPrintf(p_dlg->buffer_bytes_edit_, BUFFER_BYTES_EDIT_TEXT_LIMIT, "%u", items_count);
    SetWindowText(p_dlg->hBufferBytesEdit, p_dlg->buffer_bytes_edit_);
}

/**
 * @brief Main UI update function.
 * @details Updates the UI widgets if the reciever state changes (i.e. previously stored state is different than current one).
 * @param[in] hwnd a handle to the dialog window to be updated
 * @attention This is usually called by the application's idle handler.
 */
static void UpdateUI(HWND hwnd)
{
    static struct reciever_dialog * p_dlg = NULL;
    static receiver_state_t prev_state = -1;
    receiver_state_t new_state;
    struct fifo_circular_buffer * fifo;
    uint32_t items_count;

    if (NULL == p_dlg)
    {
        p_dlg = (struct reciever_dialog*)GetWindowLongPtr(hwnd, GWLP_USERDATA); 
        assert(p_dlg);
    }
    new_state = receiver_get_state(p_dlg->receiver_);
    /* Enable/disable controls only if state changes. */
    if (prev_state != new_state)
    {
        switch (new_state)
        {
            case RECEIVER_INITIAL:
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hSettingsBtn, TRUE);
                EnableWindow(p_dlg->hJoinMcastBtn, TRUE);
                EnableWindow(p_dlg->hLeaveMcast, FALSE);
                EnableWindow(p_dlg->hPlay, TRUE);
                EnableWindow(p_dlg->hStop, FALSE);
                EnableWindow(p_dlg->hStartRcv, FALSE);
                EnableWindow(p_dlg->hStopRcv, FALSE);
                SetFocus(p_dlg->hJoinMcastBtn);
                break;
            case RECEIVER_MCASTJOINED:
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hSettingsBtn, FALSE);
                EnableWindow(p_dlg->hJoinMcastBtn, FALSE);
                EnableWindow(p_dlg->hLeaveMcast, TRUE);
                EnableWindow(p_dlg->hPlay, TRUE);
                EnableWindow(p_dlg->hStop, FALSE);
                EnableWindow(p_dlg->hStartRcv, TRUE);
                EnableWindow(p_dlg->hStopRcv, FALSE);
                SetFocus(p_dlg->hStartRcv);
                break;
            case RECEIVER_PLAYING:
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_DISABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_ENABLED);
                EnableWindow(p_dlg->hSettingsBtn, FALSE);
                EnableWindow(p_dlg->hJoinMcastBtn, TRUE);
                EnableWindow(p_dlg->hLeaveMcast, FALSE);
                EnableWindow(p_dlg->hPlay, FALSE);
                EnableWindow(p_dlg->hStop, TRUE);
                EnableWindow(p_dlg->hStartRcv, FALSE);
                EnableWindow(p_dlg->hStopRcv, FALSE);
                SetFocus(p_dlg->hStop);
                break;
            case RECEIVER_MCASTJOINED_PLAYING:
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hSettingsBtn, FALSE);
                EnableWindow(p_dlg->hJoinMcastBtn, FALSE);
                EnableWindow(p_dlg->hLeaveMcast, TRUE);
                EnableWindow(p_dlg->hPlay, FALSE);
                EnableWindow(p_dlg->hStop, TRUE);
                EnableWindow(p_dlg->hStartRcv, TRUE);
                EnableWindow(p_dlg->hStopRcv, FALSE);
                SetFocus(p_dlg->hStartRcv);
                break;
            case RECEIVER_RECEIVING:
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hSettingsBtn, FALSE);
                EnableWindow(p_dlg->hJoinMcastBtn, FALSE);
                EnableWindow(p_dlg->hLeaveMcast, FALSE);
                EnableWindow(p_dlg->hPlay, TRUE);
                EnableWindow(p_dlg->hStop, FALSE);
                EnableWindow(p_dlg->hStartRcv, FALSE);
                EnableWindow(p_dlg->hStopRcv, TRUE);
                SetFocus(p_dlg->hPlay);
                break;
            case RECEIVER_RECEIVING_PLAYING:
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_ENABLED);
                EnableWindow(p_dlg->hSettingsBtn, FALSE);
                EnableWindow(p_dlg->hJoinMcastBtn, FALSE);
                EnableWindow(p_dlg->hLeaveMcast, FALSE);
                EnableWindow(p_dlg->hPlay, FALSE);
                EnableWindow(p_dlg->hStop, TRUE);
                EnableWindow(p_dlg->hStartRcv, FALSE);
                EnableWindow(p_dlg->hStopRcv, TRUE);
                SetFocus(p_dlg->hStop);
                break;
            default:
                break;
        }
        prev_state = new_state;
    }
    fifo = receiver_get_fifo(p_dlg->receiver_);
    items_count = fifo_circular_buffer_get_items_count(fifo);
    /* Update the progress byte control */
    SendMessage(p_dlg->hProgressBar, PBM_SETPOS, items_count, 0);
    /* Update the edit control */
    update_fifo_receiver_bytes_edit_control(p_dlg, items_count);
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
    static struct reciever_dialog * p_dlg = NULL;
    if (NULL == p_dlg)
    {
        p_dlg = (struct reciever_dialog*)GetWindowLongPtr(hDlg, GWLP_USERDATA); 
    }
    switch (uMessage)
    {
        case WM_INITDIALOG:
            return HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, Handle_wm_initdialog);
        case WM_COMMAND:
            switch (wParam)
            {
                case ID_RECEIVER_SETTINGS:
                    if (RECEIVER_INITIAL == receiver_get_state(p_dlg->receiver_) && receiver_settings_do_dialog(hDlg, &p_dlg->settings_))
                    {
                        int result;
                        result = receiver_destroy(p_dlg->receiver_); 
                        assert(result);
                        if (result)
                        {
                            p_dlg->receiver_ = receiver_create(&p_dlg->settings_);
                            assert(p_dlg->receiver_);
                            debug_outputln("%s %5.5d : %d", __FILE__, __LINE__, fifo_circular_buffer_get_capacity(receiver_get_fifo(p_dlg->receiver_)));
                            SendMessage(p_dlg->hProgressBar, PBM_SETRANGE32, 0, fifo_circular_buffer_get_capacity(receiver_get_fifo(p_dlg->receiver_)));
                            SendMessage(p_dlg->hProgressBar, PBM_SETPOS, 0, 0);
                        }
                    }
                    else
                    {
                        debug_outputln("%s %5.5d : %d", __FILE__, __LINE__, receiver_get_state(p_dlg->receiver_));
                    }
                    break;
                case ID_RECEIVER_JOINMCAST:
                    handle_mcastjoin(p_dlg->receiver_);
                    break;
                case ID_RECEIVER_LEAVEMCAST:
                    handle_mcastleave(p_dlg->receiver_);
                    break;
                case ID_RECEIVER_PLAY:
                    handle_play(p_dlg->receiver_, hDlg);
                    break;
                case ID_RECEIVER_STOP:
                    handle_stop(p_dlg->receiver_);
                    break;
                case ID_RECEIVER_STARTRCV:
                    handle_rcvstart(p_dlg->receiver_);
                    break;
                case ID_RECEIVER_STOPRCV:
                    handle_rcvstop(p_dlg->receiver_);
                    break;
                case IDM_ABOUT_RECEIVER:
                    display_about_dialog(hDlg, _T("Receiver"));
                    break;
                case IDOK:
                case IDCANCEL:
                    DestroyWindow(hDlg);
                    break;
            }
            return TRUE;
        case WM_TIMER:
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
static long int on_idle(HWND hwnd, long int count)
{
    switch (count)
    {
        case 0:
            UpdateUI(hwnd);
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
    struct reciever_dialog dlg;
    ZeroMemory(&dlg, sizeof(dlg));
    /* Init Winsock */
    if ((rc = WSAStartup(MAKEWORD(1, 1), &wsd)) != 0)
        return FALSE;
    hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        return FALSE;
    dlg.hInst_ = hInstance;
    //required to use the common controls
    InitCommonControls();
    hMainDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_MAIN_RECEIVER), NULL, ReceiverDlgProc, (LPARAM)&dlg);
    if (NULL == hMainDlg)
        return (-1);
    message_loop(hMainDlg, &on_idle);
    return (int)0;
}

