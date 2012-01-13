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
 *  @brief Pointer to the WAV file being send.
 */
static master_riff_chunk_t *   g_pWavChunk;

/**
 */
static struct mcast_settings const * g_p_defaultSettings;

/**
 */
static struct mcast_receiver * g_receiver;

/**
 * @brief Global Application instance.
 * @details Required for various Windows related stuff.
 */
HINSTANCE   g_hInst;

/**
 * @brief Main UI update function.
 * @details Updates the UI widgets state so they reflect the internal state of the program.
 * Mainly lights up and ghosts out various controls.
 * @param[in] hDlg - a handle to the window to be updated
 */
static void UpdateUI(HWND hDlg)
{
    static receiver_state_t prev_state = -1;
    receiver_state_t new_state = receiver_get_state(g_receiver);
    if (prev_state != new_state)
    {
        static HWND hSettingsBtn = NULL, 
                    hJoinMcastBtn = NULL, 
                    hLeaveMcast = NULL, 
                    hPlay = NULL, 
                    hStop = NULL,
                    hStartRcv = NULL,
                    hStopRcv = NULL;
        static HMENU hMainMenu = NULL;
        if (NULL == hSettingsBtn)
            hSettingsBtn = GetDlgItem(hDlg, ID_RECEIVER_SETTINGS);    
        if (NULL == hJoinMcastBtn)
            hJoinMcastBtn = GetDlgItem(hDlg, ID_RECEIVER_JOINMCAST);  
        if (NULL == hPlay)
            hPlay = GetDlgItem(hDlg, ID_RECEIVER_PLAY);    
        if (NULL == hStop)
            hStop = GetDlgItem(hDlg, ID_RECEIVER_STOP);  
        if (NULL == hLeaveMcast)
            hLeaveMcast = GetDlgItem(hDlg, ID_RECEIVER_LEAVEMCAST);
        if (NULL == hStopRcv)
            hStopRcv = GetDlgItem(hDlg, ID_RECEIVER_STOPRCV);
        if (NULL == hStartRcv)
            hStartRcv = GetDlgItem(hDlg, ID_RECEIVER_STARTRCV);
        if (NULL == hMainMenu)
            hMainMenu = GetMenu(hDlg);
        switch (new_state)
        {
            case RECEIVER_INITIAL:
                EnableMenuItem(hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(hSettingsBtn, TRUE);
                EnableWindow(hJoinMcastBtn, TRUE);
                EnableWindow(hLeaveMcast, FALSE);
                EnableWindow(hPlay, TRUE);
                EnableWindow(hStop, FALSE);
                EnableWindow(hStartRcv, FALSE);
                EnableWindow(hStopRcv, FALSE);
                SetFocus(hJoinMcastBtn);
                break;
            case RECEIVER_MCASTJOINED:
                EnableMenuItem(hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(hSettingsBtn, FALSE);
                EnableWindow(hJoinMcastBtn, FALSE);
                EnableWindow(hLeaveMcast, TRUE);
                EnableWindow(hPlay, TRUE);
                EnableWindow(hStop, FALSE);
                EnableWindow(hStartRcv, TRUE);
                EnableWindow(hStopRcv, FALSE);
                SetFocus(hStartRcv);
                break;
            case RECEIVER_PLAYING:
                EnableMenuItem(hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_ENABLED);
                EnableWindow(hSettingsBtn, TRUE);
                EnableWindow(hJoinMcastBtn, TRUE);
                EnableWindow(hLeaveMcast, FALSE);
                EnableWindow(hPlay, FALSE);
                EnableWindow(hStop, TRUE);
                EnableWindow(hStartRcv, FALSE);
                EnableWindow(hStopRcv, FALSE);
                SetFocus(hStopRcv);
                break;
            case RECEIVER_MCASTJOINED_PLAYING:
                EnableMenuItem(hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(hSettingsBtn, FALSE);
                EnableWindow(hJoinMcastBtn, FALSE);
                EnableWindow(hLeaveMcast, TRUE);
                EnableWindow(hPlay, FALSE);
                EnableWindow(hStop, TRUE);
                EnableWindow(hStartRcv, TRUE);
                EnableWindow(hStopRcv, FALSE);
                SetFocus(hStartRcv);
                break;
            case RECEIVER_RECEIVING:
                EnableMenuItem(hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(hSettingsBtn, FALSE);
                EnableWindow(hJoinMcastBtn, FALSE);
                EnableWindow(hLeaveMcast, FALSE);
                EnableWindow(hPlay, TRUE);
                EnableWindow(hStop, FALSE);
                EnableWindow(hStartRcv, FALSE);
                EnableWindow(hStopRcv, TRUE);
                SetFocus(hPlay);
                break;
            case RECEIVER_RECEIVING_PLAYING:
                EnableMenuItem(hMainMenu, ID_RECEIVER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_LEAVEMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STARTRCV, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STOPRCV, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_PLAY, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMainMenu, ID_RECEIVER_STOP, MF_BYCOMMAND | MF_ENABLED);
                EnableWindow(hSettingsBtn, FALSE);
                EnableWindow(hJoinMcastBtn, FALSE);
                EnableWindow(hLeaveMcast, FALSE);
                EnableWindow(hPlay, FALSE);
                EnableWindow(hStop, TRUE);
                EnableWindow(hStartRcv, FALSE);
                EnableWindow(hStopRcv, TRUE);
                SetFocus(hStop);
                break;
            default:
                break;
        }
        prev_state = new_state;
    }
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
            {
                int result;
                result = init_master_riff(&g_pWavChunk, g_hInst, MAKEINTRESOURCE(IDR_0_1));
                assert(0 == result);
                if (0 == result)
                {
                    WAVEFORMATEX * p_wfex = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WAVEFORMATEX)); 
                    copy_waveformatex_2_WAVEFORMATEX(p_wfex, &g_pWavChunk->format_chunk_.format_);
                    g_p_defaultSettings = get_default_mcast_settings();
                    g_receiver = receiver_init(p_wfex, g_p_defaultSettings);
                    assert(g_receiver);
                }
            } 
            return TRUE;
        case WM_COMMAND:
            switch(wParam)
            {
                case ID_RECEIVER_SETTINGS:
                    if (RECEIVER_INITIAL == receiver_get_state(g_receiver))
                    {
                        struct receiver_settings settings = { 0 };
                        receiver_settings_do_dialog(hDlg, &settings);
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

