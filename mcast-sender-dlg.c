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
#include "sender-res.h"

/*! 
 * @brief Length of the text displayed in the preview window.
 */
#define WAV_PREVIEW_LENGTH (512)

struct ui_control_state 
{
    UINT menu_state_;
    UINT control_state_;
    UINT btn_state_;
};

/*!
 * @brief Describes all the 'interesting' UI controls of the sender's main dialog.
 */
struct sender_dialog {
    struct mcast_sender * sender_; /*!< The sender object. This sends out the data on the multicast group. */
    struct abstract_tone * tone_selected_; /*!< Currently selected tone to be played. */
    struct sender_settings settings_; /*!< The sender settings object. Here are multicast group settings stored, and how many bytes to send with each packet. */
    TCHAR wav_preview_text_[WAV_PREVIEW_LENGTH+1];
    HWND hDlg_; /*!< Handle to the main dialog */
    HWND hSettingsBtn; /*!< Handle to the 'Settings' button. */
    HWND hJoinMcastBtn; /*!< Handle to the 'Join Multicast' button. */
    HWND hLeaveMcast; /*!< Handle to the 'Join Multicast' button. */
    HWND hStartSendingBtn; /*!< Handle to the 'Start sending' button. */
    HWND hStopSendingBtn; /*!< Handle to the 'Stop sending' button. */
    HWND hTestToneCheck; /*!< Handle to the 'Test tone' check button. */
    HWND hOpenWav_; /*!< Handle to the 'Open WAV ...' box. */
    HWND hCloseWav_; /*!< Handle to the 'Close WAV ...' box. */
    HWND hWavPreview_; /*!< Handle to the static control in which WAV file details will be displayed. */
    HMENU hMainMenu; /*!< Handle to the main menu. */
    HINSTANCE hInst_; /*!< @brief Global Application instance.  Required for various Windows related stuff. */
};

/**
 * @brief Entry point for the user interface widgets update.
 * @details Compares the previous and current sender state.
 * @param hwnd handle to the main dialog window, holding the user interface widgets.
 */
static void UpdateUI(HWND hwnd)
{
    static sender_state_t prev_state = -1;
    static struct sender_dialog * p_dlg = NULL;
    sender_state_t curr_state;
    if (NULL == p_dlg)
    {
        p_dlg = (struct sender_dialog*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    assert(p_dlg);
    curr_state = sender_get_current_state(p_dlg->sender_);
    if (prev_state != curr_state)
    {
        switch (curr_state)
        {
            case SENDER_INITIAL:
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_SETTINGS, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_JOINMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_STARTSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_STOPSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(p_dlg->hMainMenu, ID_OPEN_WAV, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(p_dlg->hMainMenu, ID_CLOSE_WAV, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hSettingsBtn, TRUE);
                EnableWindow(p_dlg->hJoinMcastBtn, TRUE);
                EnableWindow(p_dlg->hLeaveMcast, FALSE);
                EnableWindow(p_dlg->hStartSendingBtn, FALSE);
                EnableWindow(p_dlg->hStopSendingBtn, FALSE);
                EnableWindow(p_dlg->hTestToneCheck, TRUE);
                EnableWindow(p_dlg->hOpenWav_, TRUE);
                EnableWindow(p_dlg->hCloseWav_, FALSE);
                Button_SetCheck(p_dlg->hTestToneCheck, FALSE);
                p_dlg->wav_preview_text_[0] = _T('\0');
                SetWindowText(p_dlg->hWavPreview_, p_dlg->wav_preview_text_);
                SetFocus(p_dlg->hJoinMcastBtn);
                break;
            case SENDER_TONE_SELECTED:
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_SETTINGS, MF_BYCOMMAND | MF_ENABLED);
                EnableWindow(p_dlg->hSettingsBtn, TRUE);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_JOINMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableWindow(p_dlg->hJoinMcastBtn, TRUE);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hLeaveMcast, FALSE);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_STARTSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hStartSendingBtn, FALSE);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_STOPSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hStopSendingBtn, FALSE);
                EnableMenuItem(p_dlg->hMainMenu, ID_OPEN_WAV, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hOpenWav_, FALSE);
                assert(p_dlg->tone_selected_);
                switch(abstract_tone_get_type(p_dlg->tone_selected_))
                {
                    case EMBEDDED_TEST_TONE: 
                        EnableWindow(p_dlg->hCloseWav_, FALSE);
                        EnableMenuItem(p_dlg->hMainMenu, ID_CLOSE_WAV, MF_BYCOMMAND | MF_GRAYED);
                        Button_SetCheck(p_dlg->hTestToneCheck, TRUE);
                        break;
                    case EXTERNAL_WAV_TONE:
                        EnableWindow(p_dlg->hCloseWav_, TRUE);
                        EnableMenuItem(p_dlg->hMainMenu, ID_CLOSE_WAV, MF_BYCOMMAND | MF_ENABLED);
                        Button_SetCheck(p_dlg->hTestToneCheck, FALSE);
                        EnableWindow(p_dlg->hTestToneCheck, FALSE);
                        break;
                    default:
                        assert(0);
                        break;
                }
                abstract_tone_dump(p_dlg->tone_selected_, p_dlg->wav_preview_text_, WAV_PREVIEW_LENGTH);
                SetWindowText(p_dlg->hWavPreview_, p_dlg->wav_preview_text_);
                SetFocus(p_dlg->hJoinMcastBtn);
                break;
            case SENDER_MCASTJOINED_TONESELECTED:
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hSettingsBtn, FALSE);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hJoinMcastBtn, FALSE);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_LEAVEMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableWindow(p_dlg->hLeaveMcast, TRUE);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_STARTSENDING, MF_BYCOMMAND | MF_ENABLED);
                EnableWindow(p_dlg->hStartSendingBtn, TRUE);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_STOPSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hStopSendingBtn, FALSE);
                EnableMenuItem(p_dlg->hMainMenu, ID_OPEN_WAV, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hOpenWav_, FALSE);
                assert(p_dlg->tone_selected_);
                switch(abstract_tone_get_type(p_dlg->tone_selected_))
                {
                    case EMBEDDED_TEST_TONE: 
                        EnableMenuItem(p_dlg->hMainMenu, ID_CLOSE_WAV, MF_BYCOMMAND | MF_GRAYED);
                        EnableWindow(p_dlg->hCloseWav_, FALSE);
                        EnableWindow(p_dlg->hTestToneCheck, TRUE);
                        Button_SetCheck(p_dlg->hTestToneCheck, TRUE);
                        break;
                    case EXTERNAL_WAV_TONE:
                        EnableMenuItem(p_dlg->hMainMenu, ID_CLOSE_WAV, MF_BYCOMMAND | MF_ENABLED);
                        EnableWindow(p_dlg->hCloseWav_, TRUE);
                        Button_SetCheck(p_dlg->hTestToneCheck, FALSE);
                        EnableWindow(p_dlg->hTestToneCheck, FALSE);
                        break;
                    default:
                        assert(0);
                        break;
                }
                abstract_tone_dump(p_dlg->tone_selected_, p_dlg->wav_preview_text_, WAV_PREVIEW_LENGTH);
                SetWindowText(p_dlg->hWavPreview_, p_dlg->wav_preview_text_);
                SetFocus(p_dlg->hStartSendingBtn);
                break;
             case SENDER_MCAST_JOINED:
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hSettingsBtn, FALSE);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hJoinMcastBtn, FALSE);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_LEAVEMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableWindow(p_dlg->hLeaveMcast, TRUE);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_STARTSENDING, MF_BYCOMMAND | MF_ENABLED);
                EnableWindow(p_dlg->hStartSendingBtn, FALSE);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_STOPSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hStopSendingBtn, FALSE);
                EnableWindow(p_dlg->hTestToneCheck, TRUE);
                EnableWindow(p_dlg->hOpenWav_, TRUE);
                EnableWindow(p_dlg->hCloseWav_, FALSE);
                assert(NULL == p_dlg->tone_selected_);
                Button_SetCheck(p_dlg->hTestToneCheck, FALSE);
                p_dlg->wav_preview_text_[0] = _T('\0');
                SetWindowText(p_dlg->hWavPreview_, p_dlg->wav_preview_text_);
                SetFocus(p_dlg->hLeaveMcast);
                break;
           case SENDER_SENDING:
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hSettingsBtn, FALSE);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hJoinMcastBtn, FALSE);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hLeaveMcast, FALSE);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_STARTSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(p_dlg->hStartSendingBtn, FALSE);
                EnableMenuItem(p_dlg->hMainMenu, ID_SENDER_STOPSENDING, MF_BYCOMMAND | MF_ENABLED);
                EnableWindow(p_dlg->hStopSendingBtn, TRUE);
                EnableWindow(p_dlg->hTestToneCheck, FALSE);
                EnableWindow(p_dlg->hOpenWav_, FALSE);
                EnableWindow(p_dlg->hCloseWav_, FALSE);
                SetFocus(p_dlg->hStopSendingBtn);
                break;
            default:
                break;
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
    p_dlg->hSettingsBtn = GetDlgItem(hwnd, ID_SENDER_SETTINGS);    
    assert(p_dlg->hSettingsBtn);
    p_dlg->hJoinMcastBtn = GetDlgItem(hwnd, ID_SENDER_JOINMCAST);  
    assert(p_dlg->hJoinMcastBtn);
    p_dlg->hStartSendingBtn = GetDlgItem(hwnd, ID_SENDER_STARTSENDING);    
    assert(p_dlg->hStartSendingBtn);
    p_dlg->hStopSendingBtn = GetDlgItem(hwnd, ID_SENDER_STOPSENDING);  
    assert(p_dlg->hStopSendingBtn);
    p_dlg->hLeaveMcast = GetDlgItem(hwnd, ID_SENDER_LEAVEMCAST);
    assert(p_dlg->hLeaveMcast );
    p_dlg->hMainMenu = GetMenu(hwnd);
    assert(p_dlg->hMainMenu);
    p_dlg->hTestToneCheck = GetDlgItem(hwnd, ID_TEST_TONE);
    assert(p_dlg->hTestToneCheck);
    p_dlg->hOpenWav_ = GetDlgItem(hwnd, ID_OPEN_WAV);
    assert(p_dlg->hOpenWav_);
    p_dlg->hCloseWav_ = GetDlgItem(hwnd, ID_CLOSE_WAV);
    assert(p_dlg->hCloseWav_);
    p_dlg->hWavPreview_ = GetDlgItem(hwnd, IDC_WAV_PREVIEW);
    assert(p_dlg->hWavPreview_);
    result = get_default_settings(&p_dlg->settings_);
    assert(result);
    p_dlg->sender_ = sender_create(&p_dlg->settings_);
    assert(p_dlg->sender_);
    p_dlg->tone_selected_ = abstract_tone_create(EMBEDDED_TEST_TONE, MAKEINTRESOURCE(IDR_0_1));
    assert(p_dlg->tone_selected_);
    sender_handle_selecttone(p_dlg->sender_, p_dlg->tone_selected_);
    UpdateUI(hwnd);
    return TRUE;
}

static LPCTSTR getWavFileName(HWND hwnd)
{
    static TCHAR pszFileNameBuffer[MAX_PATH+1];
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = pszFileNameBuffer;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(pszFileNameBuffer);
    ofn.lpstrFilter = "WAV (*.wav)\0*.wav\0All\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)==TRUE) 
        return pszFileNameBuffer;
    return NULL;
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
            switch(LOWORD(wParam))
            {
                case ID_SENDER_SETTINGS:
                    assert(p_dlg);
                    curr_state = sender_get_current_state(p_dlg->sender_);
                    switch (curr_state)
                    {
                        case SENDER_INITIAL:
                        case SENDER_TONE_SELECTED:
                            /* Open up the settings dialog with the MCAST settings parameters */
                            if (sender_settings_from_dialog(hDlg, &p_dlg->settings_))
                            {
                                sender_destroy(p_dlg->sender_);
                                p_dlg->sender_ = sender_create(&p_dlg->settings_);
                                assert(p_dlg->sender_);
                            }
                            break;
                        default:
                            debug_outputln("%s %4.4u", __FILE__, __LINE__);
                            break;
                    }
                    break;
                case ID_SENDER_JOINMCAST:
                    sender_handle_mcastjoin(p_dlg->sender_);
                    break;
                case ID_SENDER_LEAVEMCAST:
                    sender_handle_mcastleave(p_dlg->sender_);
                    break;
                case ID_SENDER_STARTSENDING:
                    sender_handle_startsending(p_dlg->sender_);
                    break;
                case ID_SENDER_STOPSENDING:
                    sender_handle_stopsending(p_dlg->sender_);
                    break;
                case ID_TEST_TONE:
                    if (p_dlg->tone_selected_)
                    {
                        sender_handle_deselecttone(p_dlg->sender_);
                        abstract_tone_destroy(p_dlg->tone_selected_);
                        p_dlg->tone_selected_ = NULL;
                    }
                    else
                    {
                        p_dlg->tone_selected_ = abstract_tone_create(EMBEDDED_TEST_TONE, MAKEINTRESOURCE(IDR_0_1));
                        assert(p_dlg->tone_selected_);
                        sender_handle_selecttone(p_dlg->sender_, p_dlg->tone_selected_);
                    }
                    break;
                case ID_OPEN_WAV:
                    {
                        LPCTSTR pszFileName = getWavFileName(hDlg);
                        if (NULL != pszFileName)
                        {
                            p_dlg->tone_selected_ = abstract_tone_create(EXTERNAL_WAV_TONE, pszFileName);
                            assert(p_dlg->tone_selected_);
                            sender_handle_selecttone(p_dlg->sender_, p_dlg->tone_selected_);
                            debug_outputln("%s %4.4u : %s %p", __FILE__, __LINE__, pszFileName, p_dlg->tone_selected_);
                        }
                    }
                    break;
               case ID_CLOSE_WAV:
                    assert(p_dlg->tone_selected_);
                    sender_handle_deselecttone(p_dlg->sender_);
                    abstract_tone_destroy(p_dlg->tone_selected_);
                    p_dlg->tone_selected_ = NULL;
                    break;
               case IDM_SENDER_ABOUT:
                    display_about_dialog(hDlg);
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

