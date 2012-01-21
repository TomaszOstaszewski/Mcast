/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file receiver-settings-dlg.c
 * @brief Defines the interface needed to obtain receiver's settings from UI, via means of the modal dialog box.
 * @author T.Ostaszewski
 * @date Jan-2012
 * @par License
 * @code Copyright 2012 Tomasz Ostaszewski. All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation 
 *  and/or other materials provided with the distribution.
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
#include "receiver-settings.h"
#include "receiver-settings-dlg.h"
#include "mcast-settings-dlg.h"
#include "receiver-res.h"

/*!
 * @brief Maximum number of digits in the dialogs edit controls.
 */
#define TEXT_LIMIT (5)

/*!
 * @brief A structure that gathers all the control handles in one place.
 */
struct receiver_settings_dlg_controls  
{
    /*!
     * @brief Handle to the poll sleep time delay edit control.
     */
    HWND poll_sleep_time_edit_;
    /*!
     * @brief Handle to the poll sleep time delay spin control.
     */
    HWND poll_sleep_time_spin_;
    /*!
     * @brief Handle to the play buffer size edit control.
     */
    HWND play_buffer_size_edit_;
    /*!
     * @brief Handle to the play buffer size spin control.
     */
    HWND play_buffer_size_spin_;
    /*!
     * @brief Handle to the Multimedit timer timeout edit control.
     */
    HWND mmtimer_edit_;
    /*!
     * @brief Handle to the Multimedit timer timeout spin control.
     */
    HWND mmtimer_spin_;
    /*!
     * @brief Handle to the Multimedit timer timeout spin control.
     */
    HWND btok_;
    /*!
     * @brief Buffer that holds a number typed into one of the edit controls.
     */
    TCHAR text_buffer[TEXT_LIMIT+1];
};

/*!
 * @brief The object that has all the controls.
 */
static struct receiver_settings_dlg_controls g_controls;

/*!
 * @brief A copy of the receiver settings object that this dialog operates on.
 * @details If the user blesses the dialog with an OK button, and all the data
 * validates OK, then this copy becomes the settings object returned to the caller.
 */
static struct receiver_settings g_settings;

/*!
 * @brief Transfer from data to UI
 * @details Takes values from the settings object and presents them on the UI
 */
static void data_to_controls(struct receiver_settings const * p_settings, struct receiver_settings_dlg_controls * p_controls)
{
    StringCchPrintf(p_controls->text_buffer, TEXT_LIMIT+1, "%hu", p_settings->poll_sleep_time_);
    SetWindowText(p_controls->poll_sleep_time_edit_, p_controls->text_buffer);
    StringCchPrintf(p_controls->text_buffer, TEXT_LIMIT+1, "%hu", p_settings->play_settings_.play_buffer_size_);
    SetWindowText(p_controls->play_buffer_size_edit_, p_controls->text_buffer);
    StringCchPrintf(p_controls->text_buffer, TEXT_LIMIT+1, "%hu", p_settings->play_settings_.timer_delay_);
    SetWindowText(p_controls->mmtimer_edit_, p_controls->text_buffer);
}

/*!
 * @brief Transfer data from UI to the object.
 * @details Takes the values form the UI controls and saves them to the provided object.
 * @param[in] p_settings object to be written with UI data.
 * @return returns a non-zero value on success, 0 if failure has occured.
 */
static int controls_to_data(struct receiver_settings * p_settings, struct receiver_settings_dlg_controls * p_controls)
{
    int result;
    unsigned int poll_sleep_time, play_buffer_size, timer_delay;
    memset(p_controls->text_buffer, 0, sizeof(p_controls->text_buffer));
    *((WORD *)p_controls->text_buffer) = TEXT_LIMIT;
    SendMessage(p_controls->poll_sleep_time_edit_, EM_GETLINE, 0, (LPARAM)p_controls->text_buffer); 
    result = sscanf(p_controls->text_buffer, "%u", &poll_sleep_time);
    if (result<=0 || poll_sleep_time > USHRT_MAX)
        goto error;
    memset(p_controls->text_buffer, 0, sizeof(p_controls->text_buffer));
    *((WORD *)p_controls->text_buffer) = TEXT_LIMIT;
    SendMessage(p_controls->play_buffer_size_edit_, EM_GETLINE, 0, (LPARAM)p_controls->text_buffer); 
    result = sscanf(p_controls->text_buffer, "%u", &play_buffer_size);
    if (result<=0 || play_buffer_size > USHRT_MAX)
        goto error;
    memset(p_controls->text_buffer, 0, sizeof(p_controls->text_buffer));
    *((WORD *)p_controls->text_buffer) = TEXT_LIMIT;
    SendMessage(p_controls->mmtimer_edit_, EM_GETLINE, 0, (LPARAM)p_controls->text_buffer); 
    result = sscanf(p_controls->text_buffer, "%u", &timer_delay);
    if (result<=0 || timer_delay > USHRT_MAX)
        goto error;
    p_settings->play_settings_.timer_delay_ = timer_delay;
    p_settings->play_settings_.play_buffer_size_ = play_buffer_size;
    p_settings->poll_sleep_time_ = poll_sleep_time;
    return 1;
error:
    return 0;
}

/*!
 * @brief Handler for WM_INITDIALOG message.
 * @details This handler does as follows:
 * \li Initializes the control handles
 * \li Presents the settings on the UI
 */
static BOOL Handle_wm_initdialog(HWND hwnd, HWND hWndFocus, LPARAM lParam)
{
    struct receiver_settings * p_settings = &g_settings;
    g_controls.poll_sleep_time_edit_ = GetDlgItem(hwnd, IDC_POLL_SLEEP_TIME_EDIT);
    assert(g_controls.poll_sleep_time_edit_);
    g_controls.poll_sleep_time_spin_ = GetDlgItem(hwnd, IDC_POLL_SLEEP_TIME_SPIN);
    assert(g_controls.poll_sleep_time_spin_);
    g_controls.play_buffer_size_edit_ = GetDlgItem(hwnd, IDC_PLAY_BUFFER_SIZE_EDIT); 
    assert(g_controls.play_buffer_size_edit_);
    g_controls.play_buffer_size_spin_ = GetDlgItem(hwnd, IDC_PLAY_BUFFER_SIZE_SPIN);
    assert(g_controls.play_buffer_size_spin_);
    g_controls.mmtimer_edit_ = GetDlgItem(hwnd, IDC_MMTIMER_EDIT_CTRL);
    assert(g_controls.mmtimer_edit_);
    g_controls.mmtimer_spin_ = GetDlgItem(hwnd, IDC_MMTIMER_SPIN);
    assert(g_controls.mmtimer_spin_);
    g_controls.btok_ = GetDlgItem(hwnd, IDOK);
    assert(g_controls.btok_);

    SendMessage(g_controls.poll_sleep_time_spin_, UDM_SETBUDDY, (WPARAM)g_controls.poll_sleep_time_edit_, (LPARAM)0);
    SendMessage(g_controls.play_buffer_size_spin_, UDM_SETBUDDY, (WPARAM)g_controls.play_buffer_size_edit_, (LPARAM)0);
    SendMessage(g_controls.mmtimer_spin_, UDM_SETBUDDY, (WPARAM)g_controls.mmtimer_edit_, (LPARAM)0);
    SendMessage(g_controls.mmtimer_edit_, EM_SETLIMITTEXT, (WPARAM)TEXT_LIMIT, (LPARAM)0);
    SendMessage(g_controls.poll_sleep_time_edit_, EM_SETLIMITTEXT, (WPARAM)TEXT_LIMIT, (LPARAM)0);
    SendMessage(g_controls.play_buffer_size_edit_, EM_SETLIMITTEXT, (WPARAM)TEXT_LIMIT, (LPARAM)0);

    data_to_controls(&g_settings, &g_controls);
    return TRUE;
} 

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
    static struct receiver_settings copy_for_spins;
    static struct receiver_settings copy_for_edits;
    NMHDR * p_notify_header;
    NMUPDOWN * p_up_down;
    switch (uMessage)
    {
        case WM_INITDIALOG:
            return HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, Handle_wm_initdialog);
        case WM_NOTIFY:
            p_notify_header = (NMHDR*)lParam;
            switch (p_notify_header->code)
            {
                case UDN_DELTAPOS:
                    p_up_down = (NMUPDOWN *)p_notify_header;
                    receiver_settings_copy(&copy_for_spins, &g_settings);
                    switch (p_up_down->hdr.idFrom)
                    {
                        case IDC_POLL_SLEEP_TIME_SPIN:
                            copy_for_spins.poll_sleep_time_ -= p_up_down->iDelta;
                            break;
                        case IDC_PLAY_BUFFER_SIZE_SPIN:
                            copy_for_spins.play_settings_.play_buffer_size_ -= p_up_down->iDelta;
                            break;
                        case IDC_MMTIMER_SPIN:
                            copy_for_spins.play_settings_.timer_delay_ -= p_up_down->iDelta;
                            break;
                        default:
                            break;
                    }
                    /* If copy is different than the master settings - it must have been altered. Thus, there was a spin action.
                        Check if new settings validate OK, and if so, transfer those to control object. */
                    if (!receiver_settings_compare(&copy_for_spins, &g_settings) && receiver_settings_validate(&copy_for_spins))
                        data_to_controls(&copy_for_spins, &g_controls);
                    break;
                default:
                    break;
            }
            break;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                /* Process notificatoins from edit controls */
                case IDC_POLL_SLEEP_TIME_EDIT:
                case IDC_PLAY_BUFFER_SIZE_EDIT:
                case IDC_MMTIMER_EDIT_CTRL:
                    if (EN_CHANGE == HIWORD(wParam))
                    {
                        /* Make a copy of the master settings */
                        receiver_settings_copy(&copy_for_edits, &g_settings);
                        /* Alter the copy with what the user or our code has changed. Validate it */
                        if (controls_to_data(&copy_for_edits, &g_controls) && receiver_settings_validate(&copy_for_edits))
                        {
                            /* If entered settings correctly read and valid, they become our master settings. */
                            receiver_settings_copy(&g_settings, &copy_for_edits);
                            EnableWindow(g_controls.btok_, TRUE);
                        }
                        else
                        {
                            /* Either could not read settings or they are not valid. Either way - disable OK button */
                            EnableWindow(g_controls.btok_, FALSE);
                        }
                    }
                    break;
                case IDC_MCAST_SETTINGS: 
                    get_settings_from_dialog(hDlg, &g_settings.mcast_settings_);
                    break;
                case IDCANCEL:
                case IDOK:
                    EndDialog(hDlg, wParam);
                    break;
            }
            break;
        default:
            break;
    }
    return FALSE;
}

int receiver_settings_do_dialog(HWND hWndParent, struct receiver_settings * p_settings)
{
    receiver_settings_copy(&g_settings, p_settings);
    /* NULL hInst means = read dialog template from this application's resource file */
    if (IDOK == DialogBox(NULL, MAKEINTRESOURCE(IDD_RECEIVER_SETTINGS), hWndParent, McastSettingsProc))
    {
        receiver_settings_copy(p_settings, &g_settings);
        return 1;
    }
    return 0;
}

