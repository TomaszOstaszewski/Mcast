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
#include "abstract-tone.h"
#include "wave_utils.h"
#include "resource.h"

/*! 
 * @brief Length of the text displayed in the preview window.
 */
#define WAV_PREVIEW_LENGTH (512)

/*!
 * @brief Defines all the possible states of the UI.
 */
typedef enum e_sender_ui_state {
    UI_SENDER_INITIAL = 0, /*!< Initial state, no mcast group joined, not sending anything. */
    UI_SENDER_IN_MCAST_GROUP = 0x1, /*!< . */
    UI_SENDER_IN_MCAST_SENDING = 0x3, /*!< . */
} sender_ui_state_t;

/*!
 * @brief Describes all the 'interesting' UI controls of the sender's main dialog.
 */
struct sender_dialog {
    struct mcast_sender * sender_; /*!< The sender object. This sends out the data on the multicast group. */
    struct sender_settings settings_; /*!< The sender settings object. Here are multicast group settings stored, and how many bytes to send with each packet. */
    sender_ui_state_t ui_state_; /*!< The UI state. */
    HWND hDlg_; /*!< Handle to the main dialog */
    HMENU hMainMenu; /*!< Handle to the main menu. */
    HINSTANCE hInst_; /*!< @brief Global Application instance.  Required for various Windows related stuff. */
};

/*!
 * @brief Defines the entity that collection of which makes up the focus table.
 */
struct ui_focus {
    sender_ui_state_t state_; /*!< The UI state */
    UINT nID_; /*!< The control ID. This control will get the focus for given UI state. */
};

/*!
 * @brief Defines which control shall get the inital focus in each UI state.
 */
static const struct ui_focus focus_table[] = {
    {UI_SENDER_INITIAL, ID_SENDER_JOINMCAST},
    {UI_SENDER_IN_MCAST_GROUP, ID_SENDER_START_RECORDING},
    {UI_SENDER_IN_MCAST_SENDING, ID_SENDER_STOP_RECORDING},
};

/*!
 * @brief Defines the state (enabled/disabled, checked/unchecke) of the UI control.
 */
struct ui_control_state {
    sender_ui_state_t state_; /*!< The UI state */
    UINT nID_; /*!< Control's ID. The control with this ID will be enabled/disabled and checked/unchecked upon entering the UI state */
    BOOL enabled_; /*!< Whether or not to enable item. */
    BOOL checked_; /*!< Whether or not to check item. */
};

/*!
 * @brief Defines how controls shall be ghosted-out/enabled or checked in each UI state.
 */
static const struct ui_control_state controls_state_table[] = {
    /* Controls state for 'UI_SENDER_INITIAL' state */
    {UI_SENDER_INITIAL,ID_SENDER_SETTINGS, TRUE, FALSE},
    {UI_SENDER_INITIAL,ID_SENDER_JOINMCAST, TRUE, FALSE},
    {UI_SENDER_INITIAL,ID_SENDER_LEAVEMCAST, FALSE, FALSE},
    {UI_SENDER_INITIAL,ID_SENDER_STOP_RECORDING, FALSE, FALSE},
    {UI_SENDER_INITIAL,ID_SENDER_START_RECORDING, FALSE, FALSE},
    /* Controls state for 'UI_SENDER_IN_MCAST_GROUP' state */
    {UI_SENDER_IN_MCAST_GROUP,ID_SENDER_SETTINGS, FALSE, FALSE},
    {UI_SENDER_IN_MCAST_GROUP,ID_SENDER_JOINMCAST, FALSE, FALSE},
    {UI_SENDER_IN_MCAST_GROUP,ID_SENDER_LEAVEMCAST, TRUE, FALSE},
    {UI_SENDER_IN_MCAST_GROUP,ID_SENDER_START_RECORDING, TRUE, FALSE},
    {UI_SENDER_IN_MCAST_GROUP,ID_SENDER_STOP_RECORDING, FALSE, FALSE},
    /* Controls state for 'UI_SENDER_IN_MCAST_SENDING' state */
    {UI_SENDER_IN_MCAST_SENDING,ID_SENDER_SETTINGS, FALSE, FALSE},
    {UI_SENDER_IN_MCAST_SENDING,ID_SENDER_JOINMCAST, FALSE, FALSE},
    {UI_SENDER_IN_MCAST_SENDING,ID_SENDER_LEAVEMCAST, FALSE, FALSE},
    {UI_SENDER_IN_MCAST_SENDING,ID_SENDER_START_RECORDING, FALSE, FALSE},
    {UI_SENDER_IN_MCAST_SENDING,ID_SENDER_STOP_RECORDING, TRUE, FALSE},
};

/**
 * @brief Entry point for the user interface widgets update.
 * @details Compares the previous and current sender state. Updates the UI widgets state
 * if it detects a change. We need to handle this manually here. Otherwise, the focus may
 * stay with a ghosted out item, which is not desirable, as it puzzles and perplexes the end user.
 * @param hwnd handle to the main dialog window, holding the user interface widgets.
 */
static void UpdateUI(HWND hwnd)
{
    static sender_ui_state_t prev_state = -1;
    static struct sender_dialog * p_dlg = NULL;
    sender_ui_state_t curr_state;
    if (NULL == p_dlg)
    {
        p_dlg = (struct sender_dialog*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    assert(p_dlg);
    curr_state = p_dlg->ui_state_;
    if (prev_state != curr_state)
    {
        size_t idx;
        /* Iterate over the entire UI widget enable/disable table */
        /* find all rows that matche current UI state */
        /* Update enable/disable/check state UI according to the table */
        for (idx = 0; idx < COUNTOF_ARRAY(controls_state_table); ++idx)
        {
            if (controls_state_table[idx].state_ == curr_state)
            {
                UINT nMenuEnabled = MF_GRAYED, nMenuChecked = MF_UNCHECKED;
                if (controls_state_table[idx].enabled_)
                    nMenuEnabled  = MF_ENABLED;
                if (controls_state_table[idx].checked_)
                    nMenuChecked = MF_CHECKED;
                EnableMenuItem(p_dlg->hMainMenu, controls_state_table[idx].nID_, MF_BYCOMMAND | nMenuEnabled);
                CheckMenuItem(p_dlg->hMainMenu, controls_state_table[idx].nID_, MF_BYCOMMAND | nMenuChecked);
                hwnd = GetDlgItem(p_dlg->hDlg_, controls_state_table[idx].nID_);
                assert(hwnd);
                EnableWindow(hwnd, controls_state_table[idx].enabled_);
                Button_SetCheck(hwnd, controls_state_table[idx].checked_);
            } 
        }
        /* Iterate over the entire UI widget focus table */
        /* find all rows that matche current UI state */
        /* Update focus UI according to the table */
        for (idx = 0; idx < COUNTOF_ARRAY(focus_table); ++idx)
        {
            if (focus_table[idx].state_ == curr_state)
            {
                hwnd = GetDlgItem(p_dlg->hDlg_, focus_table[idx].nID_);
                assert(hwnd);
                SetFocus(hwnd); 
            }
        } 
        prev_state = curr_state;
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
    struct sender_dialog * p_dlg;
    p_dlg = (struct sender_dialog*)lParam;
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)p_dlg);
    assert(p_dlg);
    p_dlg->hDlg_ = hwnd;
    assert(p_dlg->hDlg_);
    p_dlg->hMainMenu = GetMenu(hwnd);
    assert(p_dlg->hMainMenu);
    result = get_default_settings(&p_dlg->settings_);
    assert(result);
    p_dlg->sender_ = sender_create(&p_dlg->settings_);
    assert(p_dlg->sender_);
    UpdateUI(hwnd);
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
    static struct sender_dialog * p_dlg = NULL;
    if (NULL == p_dlg)
    {
        p_dlg = (struct sender_dialog*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
    }
    switch (uMessage)
    {
        case WM_INITDIALOG:
            return HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, Handle_wm_initdialog);
        case WM_COMMAND:
            assert(p_dlg);
            switch(LOWORD(wParam))
            {
                case IDM_SENDER_ABOUT:
                    display_about_dialog(hDlg, _T("Sender"));
                    break;
                case ID_SENDER_SETTINGS:
                    switch (p_dlg->ui_state_)
                    {
                        case UI_SENDER_INITIAL:
                            /* Open up the settings dialog with the MCAST settings parameters */
                            if (sender_settings_from_dialog(hDlg, &p_dlg->settings_))
                            {
                                if (p_dlg->sender_)
                                    sender_destroy(p_dlg->sender_);
                                p_dlg->sender_ = sender_create(&p_dlg->settings_);
                                assert(p_dlg->sender_);
                            }
                            break;
                        default:
                            assert(0);
                            break;
                    }
                    break;
                case ID_SENDER_JOINMCAST:
                    switch (p_dlg->ui_state_)
                    {
                        case UI_SENDER_INITIAL:
                            if (sender_handle_mcastjoin(p_dlg->sender_))
                                p_dlg->ui_state_ = UI_SENDER_IN_MCAST_GROUP;
                            break;
                        default:
                            assert(0);
                            break;
                    }
                    break;
                case ID_SENDER_LEAVEMCAST:
                    switch (p_dlg->ui_state_)
                    {
                        case UI_SENDER_IN_MCAST_GROUP:
                            if (sender_handle_mcastleave(p_dlg->sender_))
                                p_dlg->ui_state_ = UI_SENDER_INITIAL;
                            break;
                        default:
                            assert(0);
                            break;
                    }
                    break;
                case ID_SENDER_STOP_RECORDING:
                    debug_outputln("%4.4u %s ", __LINE__, __FILE__);
                    switch (p_dlg->ui_state_)
                    {
                        case UI_SENDER_IN_MCAST_SENDING:
                            if (sender_handle_stoprecording(p_dlg->sender_))
                            {
                                p_dlg->ui_state_ = UI_SENDER_IN_MCAST_GROUP;
                            }
                            break;
                        default:
                            debug_outputln("%4.4u %s ", __LINE__, __FILE__);
                            break;
                    }
                    break;
                case ID_SENDER_START_RECORDING:
                    switch (p_dlg->ui_state_)
                    {
                        case UI_SENDER_IN_MCAST_GROUP:
                            debug_outputln("%4.4u %s ", __LINE__, __FILE__);
                            if (sender_handle_startrecording(p_dlg->sender_))
                            {
                                p_dlg->ui_state_ = UI_SENDER_IN_MCAST_SENDING;
                            }
                            break;
                        default:
                            debug_outputln("%4.4u %s ", __LINE__, __FILE__);
                            break;
                    }
                    break;
                case IDOK:
                case IDCANCEL: 
                    sender_destroy(p_dlg->sender_);
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

/*!
 * @brief The user-provided entry point for a graphical Windows-based application.
 * @param hInstance A handle to the current instance of the application.
 * @param hPrevInstance A handle to the previous instance of the application. This parameter <b>is always</b> NULL. 
 * @param lpCmdLine The command line for the application, excluding the program name. To retrieve the entire command line, use the GetCommandLine function. 
 * @param nCmdShow Controls how the window is to be shown. This parameter can be one of the following values.
 * @return If the function succeeds, terminating when it receives a WM_QUIT message, it should return the exit value contained in that message's wParam parameter. If the function terminates before entering the message loop, it should return zero. 
 */
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND hDlg;
    HRESULT hr;
    WSADATA wsd;
    int	rc;
    struct sender_dialog sender_dlg;
    ZeroMemory(&sender_dlg, sizeof(sender_dlg));
    /* Init Winsock */
    if ((rc = WSAStartup(MAKEWORD(1, 1), &wsd)) != 0)
        return 0;
    /* Init COM, this is needed for DirectX */
    hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        return 0;
    sender_dlg.hInst_ = hInstance;
    /* required to use the common controls */
    InitCommonControls();

    hDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_MAIN_SENDER), NULL, SenderDlgProc, (LPARAM)&sender_dlg);
    if (NULL == hDlg)
        return 0;
    message_loop(hDlg, &on_idle);
    CoUninitialize();
    return (int)0;
}

