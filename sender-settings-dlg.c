/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file sender-settings-dlg.c
 * @brief Defines the interface needed to obtain sender settings from UI, via means of the modal dialog box.
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
 * @date Jan-2012
 */
#include "pcc.h"
#include "debug_helpers.h"
#include "sender-settings-dlg.h"
#include "mcast-settings-dlg.h"
#include "sender-settings.h"
#include "mcast-settings.h"
#include "sender-res.h"

/*!
 * @brief Maximum number of digits in the dialogs edit controls.
 */
#define TEXT_LIMIT (4)

/*!
 * @brief A container for UI controls and variables, which are displayed by those UI controls.
 */
struct ui_controls 
{
    /*!
     * @brief Handle to the dialog window holding the controls.
     */
    HWND hDlg_;

    /*!
     * @brief Handle to the packet length edit control.
     * @details This control shows the amount of audio time contained in one packet. The unit is bytes.
     */
    HWND packet_length_ms_edit_;

    /*!
     * @brief Handle to the packet length edit control.
     * @details This control shows the amount of audio time contained in one packet. The unit is bytes.
     */
    HWND packet_length_bytes_edit_;

    /*!
     * @brief Handle to the packet length spin control.
     */
    HWND packet_length_ms_spin_;

    /*!
     * @brief Handle to the OK button.
     * @details This control is enabled or disabled depending on the outcome of dialog data validation.
     */
    HWND btok_;

    /*!
     * @brief A master copy of the sender settings object that this dialog operates on.
     * @details If the user blesses the dialog with an OK button, and all the data
     * validates OK, then this copy becomes the settings object returned to the caller.
     */
    struct sender_settings g_settings;

    /*!
     * @brief Another copy of the master settings.
     * @details When the spin control changes the settings, we first make a copy of it prior
     * incorporating the changes from the control into the master copy.
     * This goes like that:
     * - the spin control notifies us that the settings will be changed;
     * - we make a copy of the master settings into this member variable;
     * - we than incorporate the changes form the spin control into this member variable;
     * - if the settings are valid, i.e. they are well within design bounds, we override the
     *   master copy with the contents of this variable;
     * - otherwise, we discard the copy
     */
    struct sender_settings copy_for_spins_;

    /*!
     * @brief Another copy of the master settings.
     * @details When the edit control changes the settings, we first make a copy of it prior
     * incorporating the changes from the control into the master copy.
     * This goes like that:
     * - the edit control notifies us that the settings will be changed;
     * - we make a copy of the master settings into this member variable;
     * - we than incorporate the changes form the spin control into this member variable;
     * - if the settings are valid, i.e. they are well within design bounds, we override the
     *   master copy with the contents of this variable;
     * - otherwise, we discard the copy
     */
    struct sender_settings copy_for_edits_;

    /*!
     * @brief Buffer that holds a number typed into one of the edit controls.
     */
    TCHAR text_buffer[TEXT_LIMIT+1];
};

static struct ui_controls * g_controls;

static void update_calculated_controls(struct ui_controls * p_controls, struct sender_settings const * p_settings)
{
    uint32_t length_in_bytes = sender_settings_get_chunk_size_bytes(p_settings);
    StringCchPrintf(p_controls->text_buffer, TEXT_LIMIT+1, "%4u", length_in_bytes);
    SetWindowText(p_controls->packet_length_bytes_edit_, p_controls->text_buffer);
}

/*!
 * @brief Transfer from data to UI
 * @details Takes values from the settings object and presents them on the UI
 * @param[in] p_settings points to data to be transferred to the controls.
 */
static void data_to_controls(struct ui_controls * p_controls, struct sender_settings const * p_settings)
{
    StringCchPrintf(p_controls->text_buffer, TEXT_LIMIT+1, "%u", p_settings->chunk_size_ms_);
    SetWindowText(p_controls->packet_length_ms_edit_, p_controls->text_buffer);
    update_calculated_controls(p_controls, p_settings);
}

/*!
 * @brief Transfer data from UI to the object.
 * @details Takes the values form the UI controls and saves them to the provided object. For numeric controls, 
 * it is just calling sscanf() a couple number of times, each time validating if the entry we fetched can fit into destination.
 * For instance, 5 digit decimal unsigned variable may range from 0 to 99999 - this won't fit 2 byte hex variable. We need to 
 * check for that specifically. 
 * @param[in] p_settings object to be written with UI data.
 * @return returns a non-zero value on success, 0 if failure has occured. The failure is usually attributed to the fact
 * that data from controls cannot fit into representation offered by the target object.
 */
static int controls_to_data(struct ui_controls * p_controls, struct sender_settings * p_settings)
{
    int result;
    unsigned int packet_length_ms;
    memset(p_controls->text_buffer, 0, sizeof(p_controls->text_buffer));
    *((WORD *)p_controls->text_buffer) = TEXT_LIMIT;
    SendMessage(p_controls->packet_length_ms_edit_, EM_GETLINE, 0, (LPARAM)p_controls->text_buffer); 
    result = sscanf(p_controls->text_buffer, "%u", &packet_length_ms);
    if (result<=0 || packet_length_ms > USHRT_MAX)
        goto error;
    p_settings->chunk_size_ms_ = packet_length_ms;
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
    struct ui_controls * p_controls = (struct ui_controls *)lParam;
    g_controls = p_controls;
    p_controls->packet_length_ms_edit_ = GetDlgItem(hwnd, IDC_PACKET_LENGTH_MS_EDIT);
    assert(p_controls->packet_length_ms_edit_);
    p_controls->packet_length_bytes_edit_ = GetDlgItem(hwnd, IDC_PACKET_LENGTH_BYTES_EDIT);
    assert(p_controls->packet_length_bytes_edit_);
    p_controls->packet_length_ms_spin_ = GetDlgItem(hwnd, IDC_PACKET_LENGTH_MS_SPIN);
    assert(p_controls->packet_length_ms_spin_);
    p_controls->btok_ = GetDlgItem(hwnd, IDOK);
    assert(p_controls->btok_);
    SendMessage(p_controls->packet_length_ms_spin_, UDM_SETBUDDY, (WPARAM)p_controls->packet_length_ms_edit_, (LPARAM)0);
    SendMessage(p_controls->packet_length_ms_spin_, UDM_SETPOS, (WPARAM)0, (LPARAM)0);
    SendMessage(p_controls->packet_length_ms_edit_, EM_SETLIMITTEXT, (WPARAM)TEXT_LIMIT, (LPARAM)0);
    data_to_controls(p_controls, &p_controls->g_settings);
    return FALSE;
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
    NMHDR * p_notify_header;
    NMUPDOWN * p_up_down;
    switch (uMessage)
    {
        case WM_INITDIALOG:
            return HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, Handle_wm_initdialog);
        case WM_NOTIFY:
            p_notify_header = (NMHDR*)lParam;
            p_up_down = (NMUPDOWN *)p_notify_header;
            switch (p_notify_header->code)
            {
                case UDN_DELTAPOS:
                    sender_settings_copy(&g_controls->copy_for_spins_, &g_controls->g_settings);
                    switch (p_notify_header->idFrom)
                    {
                        case IDC_PACKET_LENGTH_MS_SPIN:
                            g_controls->copy_for_spins_.chunk_size_ms_ -= p_up_down->iDelta;
                            break;
                        default:
                            break;
                    }
                    /* If copy and master settings are different, and a copy fits the bounds, update the controls with copy contents */
                    if (!sender_settings_compare(&g_controls->copy_for_spins_, &g_controls->g_settings) &&  sender_settings_validate(&g_controls->copy_for_spins_))
                        data_to_controls(g_controls, &g_controls->copy_for_spins_);
                    break;
                default:
                    break;
            }
            break;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_MCAST_SETTINGS:
                    get_settings_from_dialog(hDlg, &g_controls->g_settings.mcast_settings_);
                    break;
                case IDC_PACKET_LENGTH_MS_EDIT:
                    if (EN_CHANGE == HIWORD(wParam))
                    {
                        sender_settings_copy(&g_controls->copy_for_edits_, &g_controls->g_settings);
                        if (controls_to_data(g_controls, &g_controls->copy_for_edits_) && sender_settings_validate(&g_controls->copy_for_edits_)) 
                        {
                            sender_settings_copy(&g_controls->g_settings, &g_controls->copy_for_edits_);
                            EnableWindow(g_controls->btok_, TRUE);
                            update_calculated_controls(g_controls, &g_controls->copy_for_edits_);
                        }
                        else
                        {
                            EnableWindow(g_controls->btok_, FALSE);
                        }
                    }
                    break;
                case IDCANCEL:
                case IDOK:
                    EndDialog(hDlg, wParam);
                    return TRUE;
            }
        default:
            break;
    }
    return FALSE;
}

int sender_settings_from_dialog(HWND hWndParent, struct sender_settings * p_settings)
{
    struct ui_controls controls;
    sender_settings_copy(&controls.g_settings, p_settings);
    /* NULL hInst means = read dialog template from this application's resource file */
    if (IDOK == DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_SENDER_SETTINGS), hWndParent, McastSettingsProc, (LPARAM)&controls))
    {
        sender_settings_copy(p_settings, &controls.g_settings);
        return 1;
    }
    return 0;
}

