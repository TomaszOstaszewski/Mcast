/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @brief
 * @file sender-settings-dlg.c
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
#include "resource.h"
#include "sender-settings-dlg.h"
#include "mcast-settings-dlg.h"
#include "sender-settings.h"
#include "mcast-settings.h"

extern HINSTANCE g_hInst;

/*!
 * @brief A copy of the sender settings object that this dialog operates on.
 * @details If the user blesses the dialog with an OK button, and all the data
 * validates OK, then this copy becomes the settings object returned to the caller.
 */
static struct sender_settings g_settings;

/*!
 * @brief Handle to the packet delay edit control.
 */
static HWND g_packet_delay_edit;

/*!
 * @brief Handle to the packet length edit control.
 * @details This control shows the amount of audio time contained in one packet. The unit is bytes.
 */
static HWND g_packet_length_edit;

/*!
 * @brief Handle to the packet delay spin control.
 */
static HWND g_packet_delay_spin;

/*!
 * @brief Handle to the packet length spin control.
 */
static HWND g_packet_length_spin;
/*!
 * @brief Handle to the packet length edit control.
 * @details This control shows the amount of audio time contained in one packet. The unit is milliseconds.
 */
static HWND g_packet_lenght_ms_edit;

/*!
 * @brief Handle to the OK button.
 * @details This control is enabled or disabled depending on the outcome of dialog data validation.
 */
static HWND g_btok;

/*!
 * @brief Maximum number of digits in the dialogs edit controls.
 */
#define TEXT_LIMIT (4)

/*!
 * @brief Buffer that holds a number typed into one of the edit controls.
 */
static TCHAR text_buffer[TEXT_LIMIT+1];

#define SAMPLES_PER_SEC (8000.0)
#define BYTES_PER_SAMPLE (2.0)
#define MILLISECOND_IN_SEC (1000.0)

static void update_calculated_controls(struct sender_settings const * p_settings)
{
    double length_in_ms = (MILLISECOND_IN_SEC * p_settings->chunk_size_/BYTES_PER_SAMPLE)/SAMPLES_PER_SEC;
    StringCchPrintf(text_buffer, TEXT_LIMIT+1, "%2.2f", length_in_ms);
    SetWindowText(g_packet_lenght_ms_edit, text_buffer);
}

/*!
 * @brief Transfer from data to UI
 * @details Takes values from the settings object and presents them on the UI
 */
static void data_to_controls(struct sender_settings const * p_settings)
{
    /*! \todo Remove this nasty magic numbers with variables, whose values are taken form the WAV file being send */
    update_calculated_controls(p_settings);
    StringCchPrintf(text_buffer, TEXT_LIMIT+1, "%u", p_settings->send_delay_);
    SetWindowText(g_packet_delay_edit, text_buffer);
    StringCchPrintf(text_buffer, TEXT_LIMIT+1, "%u", p_settings->chunk_size_);
    SetWindowText(g_packet_length_edit, text_buffer);
}

/*!
 * @brief Transfer data from UI to the object.
 * @details Takes the values form the UI controls and saves them to the provided object.
 * @param[in] p_settings object to be written with UI data.
 * @return returns a non-zero value on success, 0 if failure has occured. The failure is usually attributed to the fact
 * that data from controls cannot fit into representation offered by the target object.
 */
static int controls_to_data(struct sender_settings * p_settings)
{
    int result;
    unsigned int packet_delay, packet_length;
    memset(text_buffer, 0, sizeof(text_buffer));
    *((WORD *)text_buffer) = TEXT_LIMIT;
    SendMessage(g_packet_delay_edit, EM_GETLINE, 0, (LPARAM)text_buffer); 
    result = sscanf(text_buffer, "%u", &packet_delay);
    if (!result)
        goto error;
    if (packet_delay > USHRT_MAX)
        goto error;
    memset(text_buffer, 0, sizeof(text_buffer));
    *((WORD *)text_buffer) = TEXT_LIMIT;
    SendMessage(g_packet_length_edit, EM_GETLINE, 0, (LPARAM)text_buffer); 
    result = sscanf(text_buffer, "%u", &packet_length);
    if (!result)
        goto error;
    if (packet_length > USHRT_MAX)
        goto error;
    p_settings->send_delay_ = packet_delay;
    p_settings->chunk_size_ = packet_length;
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
    g_packet_delay_edit = GetDlgItem(hwnd, IDC_PACKET_DELAY_EDIT);
    assert(g_packet_delay_edit);
    g_packet_length_edit = GetDlgItem(hwnd, IDC_PACKET_LENGTH_EDIT);
    assert(g_packet_length_edit);
    g_packet_delay_spin = GetDlgItem(hwnd, IDC_PACKET_DELAY_SPIN);
    assert(g_packet_delay_spin);
    g_packet_length_spin = GetDlgItem(hwnd, IDC_PACKET_LENGTH_SPIN);
    assert(g_packet_length_spin);
    g_packet_lenght_ms_edit = GetDlgItem(hwnd, IDC_PACKET_LENGTH_MS_EDIT);
    assert(g_packet_lenght_ms_edit);
    g_btok = GetDlgItem(hwnd, IDOK);
    assert(g_btok);
    SendMessage(g_packet_delay_spin, UDM_SETBUDDY, (WPARAM)g_packet_delay_edit, (LPARAM)0);
    SendMessage(g_packet_length_spin, UDM_SETBUDDY, (WPARAM)g_packet_length_edit, (LPARAM)0);
    SendMessage(g_packet_delay_spin, UDM_SETPOS, (WPARAM)0, (LPARAM)0);
    SendMessage(g_packet_length_spin, UDM_SETPOS, (WPARAM)0, (LPARAM)0);
    SendMessage(g_packet_delay_edit, EM_SETLIMITTEXT, (WPARAM)TEXT_LIMIT, (LPARAM)0);
    SendMessage(g_packet_length_edit, EM_SETLIMITTEXT, (WPARAM)TEXT_LIMIT, (LPARAM)0);
    data_to_controls(&g_settings);
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
    static struct sender_settings copy_for_spins;
    static struct sender_settings copy_for_edits;
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
                    CopyMemory(&copy_for_spins, &g_settings, sizeof(struct sender_settings));
                    switch (p_notify_header->idFrom)
                    {
                        case IDC_PACKET_LENGTH_SPIN:
                            copy_for_spins.chunk_size_ -= p_up_down->iDelta;
                            if (sender_settings_validate(&copy_for_spins))
                                data_to_controls(&copy_for_spins);
                            break;
                        case IDC_PACKET_DELAY_SPIN:
                            copy_for_spins.send_delay_ -= p_up_down->iDelta;
                            if (sender_settings_validate(&copy_for_spins))
                                data_to_controls(&copy_for_spins);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_MCAST_SETTINGS:
                    get_settings_from_dialog(hDlg, &g_settings.mcast_settings_);
                    break;
                case IDC_PACKET_DELAY_EDIT:
                case IDC_PACKET_LENGTH_EDIT:
                    if (EN_CHANGE == HIWORD(wParam))
                    {
                        CopyMemory(&copy_for_edits, &g_settings, sizeof(struct sender_settings));
                        if (controls_to_data(&copy_for_edits) && sender_settings_validate(&copy_for_edits)) 
                        {
                            CopyMemory(&g_settings, &copy_for_edits, sizeof(struct sender_settings));
                            EnableWindow(g_btok, TRUE);
                            update_calculated_controls(&copy_for_edits);
                        }
                        else
                        {
                            EnableWindow(g_btok, FALSE);
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
    CopyMemory(&g_settings, p_settings, sizeof(struct sender_settings));
    if (IDOK == DialogBox(g_hInst, MAKEINTRESOURCE(IDD_SENDER_SETTINGS), hWndParent, McastSettingsProc))
    {
        CopyMemory(p_settings, &g_settings, sizeof(struct sender_settings));
        return 1;
    }
    return 0;
}

