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
#include "receiver.h" /* resources */
#include "debug_helpers.h"
#include "receiver-settings.h"
#include "receiver-settings-dlg.h"
#include "mcast-settings-dlg.h"
#include "dialog-utils.h"
#include "receiver-res.h"
#include "wave_utils.h"
#include "receiver.h"

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
     * @brief A copy of the receiver settings object that this dialog operates on.
     * @details If the user blesses the dialog with an OK button, and all the data
     * validates OK, then this copy becomes the settings object returned to the caller.
     */
    struct receiver_settings settings_;

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
    struct receiver_settings spins_copy_;

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
    struct receiver_settings other_copy_;

    /*!
     * @brief Handle to the poll sleep time delay edit control.
     */
    HWND poll_sleep_time_edit_;
    /*!
     * @brief Handle to the poll sleep time delay spin control.
     */
    HWND poll_sleep_time_spin_;
    /*!
     * @brief Handle to the play buffer size combo control.
     */
    HWND chunk_size_combo_;
    /*!
     * @brief Handle to the # of chunks edit control.
     */
    HWND num_of_chunks_edit_;
    /*!
     * @brief Handle to the # of chunks spin control.
     */
    HWND num_of_chunks_spin_;
    /*!
     * @brief Handle to the WAV sample rate combo box.
     */
    HWND sample_rate_combo_;
    /*!
     * @brief Handle to the WAV bits per sample rate combo box.
     */
    HWND bits_per_sample_combo_;
    /*!
     * @brief Handle to the 'WAV channels' combo box.
     */
    HWND channels_combo_;
    /*!
     * @brief Handle to the 'Circular buffer size' combo box.
     */
    HWND circularbuffer_combo_;
    /*!
     * @brief Handle to the OK button.
     */
    HWND btok_;
};

/*!
 * @brief Structure used to associate a number with a combo box item.
 */
struct val_2_combo {
    unsigned int val_; /*!< Value to be associated with combo box. */
    int combo_idx_; /*!< Combo box item number */
};
 
static struct val_2_combo sample_rate_values[] = {
    { 8000 },
    { 11025 },
    { 16000 },
    { 22050 },
    { 32000 },
    { 44100 },
    { 48000 },
    { 96000 },
};

static struct val_2_combo chunk_size_combo_values [] = {
    { 512 },
    { 1024 },
    { 2048 },
    { 4096 },
    { 8192 },
    { 16384 },
    { 32768 },
};

static struct val_2_combo circularbuffer_size_values[] = {
    { 32 },
    { 64 },
    { 128 },
    { 256 },
    { 512 },
    { 1024 },
    { 2048 },
    { 4096 },
    { 8192 },
    { 16384 },
    { 32768 },
    { 65536 },
};

static struct val_2_combo single_chunk_size_values[] = {
    { 512 },
    { 1024 },
    { 2048 },
    { 4096 },
    { 8192 },
    { 16384 },
};

static struct val_2_combo bits_per_sample_values[] = {
    { 8 },
    { 16 },
};

static struct val_2_combo channel_values[] = {
    { 1 },
    { 2 },
};

/*!
 * @brief The object that has all the controls.
 */
static struct receiver_settings_dlg_controls * g_controls;

static unsigned int log2ofInteger(unsigned int v)
{
    unsigned int r = 0; // r will be lg(v)
    while (v >>= 1) // unroll for more speed...
    {
        r++;
    }
    return r;
}

static void fill_combo(HWND hCombo, struct val_2_combo * values, unsigned int count)
{
    static TCHAR text_buffer[8] = {0};
    size_t index;
    for (index = 0; index < count; ++index)
    {
        int combo_idx, set_item_ptr_result;
        StringCchPrintf(text_buffer, 8, "%u", values[index].val_);
        combo_idx = ComboBox_InsertString(hCombo, -1, text_buffer);
        assert(CB_ERR != combo_idx);
        set_item_ptr_result = ComboBox_SetItemData(hCombo, combo_idx, index);
        assert(CB_ERR != set_item_ptr_result);
        values[index].combo_idx_ = combo_idx;
    }
}

/*!
 * @brief For given combo and given value, selects the combo box item so it matches the value.
 */
static void select_combo_value(struct val_2_combo const * val_table, size_t table_size, HWND combo, const unsigned int value)
{
    size_t idx;
    /* Find the sample rate that matches the combo box items */
    for (idx = 0; idx < table_size; ++idx)
    {
        if (value == val_table[idx].val_)
        {
            ComboBox_SetCurSel(combo, val_table[idx].combo_idx_);
            break;
        }
    }
    assert(idx < table_size);
}

/*!
 * @brief Transfer from data to UI
 * @details Takes values from the settings object and presents them on the UI
 */
static void data_to_controls(struct receiver_settings const * p_settings, struct receiver_settings_dlg_controls * p_controls)
{
    UINT nCircularBufferSize = 1 << p_settings->circular_buffer_level_;
    put_in_edit_control_uint16(p_controls->poll_sleep_time_edit_, p_settings->poll_sleep_time_);
    put_in_edit_control_uint16(p_controls->num_of_chunks_edit_, p_settings->play_settings_.play_chunks_count_);
    /* Find the sample rate that matches the combo box items */
    debug_outputln("%4.4u %s : %u", __LINE__, __FILE__, p_settings->play_settings_.play_chunk_size_in_bytes_);
    select_combo_value(chunk_size_combo_values, COUNTOF_ARRAY(chunk_size_combo_values), p_controls->chunk_size_combo_, p_settings->play_settings_.play_chunk_size_in_bytes_);
    select_combo_value(sample_rate_values, sizeof(sample_rate_values)/sizeof(sample_rate_values[0]), p_controls->sample_rate_combo_, p_settings->wfex_.nSamplesPerSec);
    select_combo_value(bits_per_sample_values, sizeof(bits_per_sample_values)/sizeof(bits_per_sample_values[0]), p_controls->bits_per_sample_combo_, p_settings->wfex_.wBitsPerSample);
    select_combo_value(channel_values, sizeof(channel_values)/sizeof(channel_values[0]), p_controls->channels_combo_, p_settings->wfex_.nChannels);
    select_combo_value(circularbuffer_size_values, sizeof(circularbuffer_size_values)/sizeof(circularbuffer_size_values[0]), p_controls->circularbuffer_combo_, nCircularBufferSize);
}

/*!
 * @brief Transfer data from edit UI to the object.
 * @details Takes the values form the UI controls and saves them to the provided object.
 * @param[in] p_settings object to be written with UI data.
 * @return returns a non-zero value on success, 0 if failure has occured.
 */
static int edit_controls_to_data(struct receiver_settings * p_settings, struct receiver_settings_dlg_controls * p_controls)
{
    int result;
    uint16_t poll_sleep_time, play_chunks_count;
    /* Get values from edit controls */
    result = get_from_edit_uint16_dec(p_controls->poll_sleep_time_edit_, &poll_sleep_time);
    if (result<=0)
        goto error;
    result = get_from_edit_uint16_dec(p_controls->num_of_chunks_edit_, &play_chunks_count);
    if (result<=0)
        goto error;
    p_settings->poll_sleep_time_ = poll_sleep_time;
    p_settings->play_settings_.play_chunks_count_ = play_chunks_count;
    return 1;
error:
    return 0;
}

/*!
 * @brief Transfer data from combo UI to the object.
 * @details Takes the values form the UI controls and saves them to the provided object.
 * @param[in] p_settings object to be written with UI data.
 * @return returns a non-zero value on success, 0 if failure has occured.
 */
static int combo_controls_to_data(struct receiver_settings * p_settings, struct receiver_settings_dlg_controls * p_controls)
{
    int result;
    WORD wBitsPerSample;
    WORD nChannels;
    DWORD nSamplesPerSec;
    UINT nCircularBufferLevel;
    uint32_t play_chunk_size;
    unsigned int combo_data_item;
    /* Get a value from chunk size combo control */
    result = ComboBox_GetCurSel(p_controls->chunk_size_combo_);
    assert(CB_ERR != result);
    if (CB_ERR == result)
    {
        debug_outputln("%s %d", __FILE__, __LINE__);
        goto error;
    }
    play_chunk_size = chunk_size_combo_values[ComboBox_GetItemData(p_controls->chunk_size_combo_, result)].val_;
    debug_outputln("%s %d : %u", __FILE__, __LINE__, play_chunk_size);

    /* Get a value from sample rate combo control */
    result = ComboBox_GetCurSel(p_controls->sample_rate_combo_);
    assert(CB_ERR != result);
    if (CB_ERR == result)
    {
        debug_outputln("%s %d", __FILE__, __LINE__);
        goto error;
    }
    combo_data_item = ComboBox_GetItemData(p_controls->sample_rate_combo_, result);
    assert(combo_data_item < sizeof(sample_rate_values)/sizeof(sample_rate_values[0]));
    if (combo_data_item >= sizeof(sample_rate_values)/sizeof(sample_rate_values[0]))
    {
        debug_outputln("%s %d : %d %u", __FILE__, __LINE__, combo_data_item, sizeof(sample_rate_values)/sizeof(sample_rate_values));
        goto error;
    }
    nSamplesPerSec = sample_rate_values[combo_data_item].val_;
    /* Get a value from bits per sample combo control */
    result = ComboBox_GetCurSel(p_controls->bits_per_sample_combo_);
    assert(CB_ERR != result);
    if (CB_ERR == result)
    {
        debug_outputln("%s %d", __FILE__, __LINE__);
        goto error;
    }
    combo_data_item = ComboBox_GetItemData(p_controls->bits_per_sample_combo_, result);
    assert(combo_data_item < sizeof(bits_per_sample_values)/sizeof(bits_per_sample_values[0]));
    if(combo_data_item >= sizeof(bits_per_sample_values)/sizeof(bits_per_sample_values[0]))
    {
        debug_outputln("%s %d : %d %u", __FILE__, __LINE__, combo_data_item, sizeof(bits_per_sample_values)/sizeof(bits_per_sample_values[0]));
        goto error;
    }
    wBitsPerSample = bits_per_sample_values[combo_data_item].val_;
    /* Get value form 'channels' combo */
    result = ComboBox_GetCurSel(p_controls->channels_combo_);
    assert(CB_ERR != result);
    if (CB_ERR == result)
    {
        debug_outputln("%s %d", __FILE__, __LINE__);
        goto error;
    }
    combo_data_item = ComboBox_GetItemData(p_controls->channels_combo_, result);
    assert(combo_data_item < sizeof(channel_values)/sizeof(channel_values[0]));
    if(combo_data_item >= sizeof(channel_values)/sizeof(channel_values[0]))
    {
        debug_outputln("%s %d : %d %u", __FILE__, __LINE__, combo_data_item, sizeof(channel_values)/sizeof(channel_values));
        goto error;
    }
    nChannels = channel_values[combo_data_item].val_;
    /* Get value form 'circular buffer size' combo */
    result = ComboBox_GetCurSel(p_controls->circularbuffer_combo_);
    assert(CB_ERR != result);
    if (CB_ERR == result)
    {
        debug_outputln("%s %d", __FILE__, __LINE__);
        goto error;
    }
    combo_data_item = ComboBox_GetItemData(p_controls->circularbuffer_combo_, result);
    assert(combo_data_item < sizeof(circularbuffer_size_values)/sizeof(circularbuffer_size_values[0]));
    if(combo_data_item >= sizeof(circularbuffer_size_values)/sizeof(circularbuffer_size_values[0]))
    {
        debug_outputln("%s %d : %d %u", __FILE__, __LINE__, combo_data_item, sizeof(circularbuffer_size_values)/sizeof(circularbuffer_size_values));
        goto error;
    }
    nCircularBufferLevel = circularbuffer_size_values[combo_data_item].val_;
    p_settings->wfex_.nChannels      = nChannels;
    p_settings->wfex_.wBitsPerSample = wBitsPerSample;
    p_settings->wfex_.nSamplesPerSec = nSamplesPerSec;
    p_settings->circular_buffer_level_ = log2ofInteger(nCircularBufferLevel);
    p_settings->play_settings_.play_chunk_size_in_bytes_ = play_chunk_size;
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
    g_controls = (struct receiver_settings_dlg_controls *)lParam;
    g_controls->poll_sleep_time_edit_ = GetDlgItem(hwnd, IDC_POLL_SLEEP_TIME_EDIT);
    assert(g_controls->poll_sleep_time_edit_);
    g_controls->poll_sleep_time_spin_ = GetDlgItem(hwnd, IDC_POLL_SLEEP_TIME_SPIN);
    assert(g_controls->poll_sleep_time_spin_);
    g_controls->sample_rate_combo_ = GetDlgItem(hwnd, IDC_WAV_SAMPLE_RATE);
    assert(g_controls->sample_rate_combo_);
    g_controls->bits_per_sample_combo_ = GetDlgItem(hwnd, IDC_WAV_BITS_PER_SAMPLE);
    assert(g_controls->bits_per_sample_combo_);
    g_controls->btok_ = GetDlgItem(hwnd, IDOK);
    assert(g_controls->btok_);
    g_controls->num_of_chunks_edit_ = GetDlgItem(hwnd, IDC_PLAY_CHUNKS_EDIT);
    assert(g_controls->num_of_chunks_edit_);
    g_controls->num_of_chunks_spin_ = GetDlgItem(hwnd, IDC_PLAY_CHUNKS_SPIN);
    assert(g_controls->num_of_chunks_spin_);
    g_controls->channels_combo_ = GetDlgItem(hwnd, IDC_WAV_CHANNELS);
    assert(g_controls->channels_combo_);
    g_controls->circularbuffer_combo_ = GetDlgItem(hwnd, IDC_CIRCBUFFER_COMBO);
    assert(g_controls->circularbuffer_combo_);
    g_controls->chunk_size_combo_ = GetDlgItem(hwnd, IDC_COMBO1);
    assert(g_controls->chunk_size_combo_);
    SendMessage(g_controls->poll_sleep_time_spin_, UDM_SETBUDDY, (WPARAM)g_controls->poll_sleep_time_edit_, (LPARAM)0);
    SendMessage(g_controls->poll_sleep_time_edit_, EM_SETLIMITTEXT, (WPARAM)TEXT_LIMIT, (LPARAM)0);
    SendMessage(g_controls->num_of_chunks_edit_, EM_SETLIMITTEXT, (WPARAM)TEXT_LIMIT, (LPARAM)0);
    fill_combo(g_controls->chunk_size_combo_, chunk_size_combo_values, COUNTOF_ARRAY(chunk_size_combo_values));
    fill_combo(g_controls->sample_rate_combo_, sample_rate_values, COUNTOF_ARRAY(sample_rate_values));
    fill_combo(g_controls->bits_per_sample_combo_, bits_per_sample_values, COUNTOF_ARRAY(bits_per_sample_values));
    fill_combo(g_controls->channels_combo_, channel_values, COUNTOF_ARRAY(channel_values));
    fill_combo(g_controls->circularbuffer_combo_, circularbuffer_size_values, COUNTOF_ARRAY(circularbuffer_size_values));
    data_to_controls(&g_controls->settings_, g_controls);
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
    NMHDR * p_notify_header;
    NMUPDOWN * p_up_down;
    static int s_chunk_size_value = 0;
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
                    receiver_settings_copy(&g_controls->spins_copy_, &g_controls->settings_);
                    switch (p_up_down->hdr.idFrom)
                    {
                        case IDC_POLL_SLEEP_TIME_SPIN:
                            g_controls->spins_copy_.poll_sleep_time_ -= p_up_down->iDelta;
                            break;
                        case IDC_PLAY_CHUNKS_SPIN:
                            g_controls->spins_copy_.play_settings_.play_chunks_count_ -= p_up_down->iDelta;
                        default:
                            break;
                    }
                    /* If copy is different than the master settings - it must have been altered. Thus, there was a spin action.
                     *  Check if new settings validate OK, and if so, transfer those to control object. 
                     */
                    if (!receiver_settings_compare(&g_controls->spins_copy_, &g_controls->settings_) && receiver_settings_validate(&g_controls->spins_copy_))
                        data_to_controls(&g_controls->spins_copy_, g_controls);
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
                case IDC_PLAY_CHUNKS_EDIT:
                    switch (HIWORD(wParam))
                    {
                        case EN_CHANGE:
                            /* Make a copy of the master settings */
                            receiver_settings_copy(&g_controls->other_copy_, &g_controls->settings_);
                            /* Alter the copy with what the user or our code has changed. Validate it */
                            if (edit_controls_to_data(&g_controls->other_copy_, g_controls) && receiver_settings_validate(&g_controls->other_copy_))
                            {
                                /* If entered settings correctly read and valid, they become our master settings. */
                                receiver_settings_copy(&g_controls->settings_, &g_controls->other_copy_);
                                EnableWindow(g_controls->btok_, TRUE);
                            }
                            else
                            {
                                /* Either could not read settings or they are not valid. Either way - disable OK button */
                                EnableWindow(g_controls->btok_, FALSE);
                            }
                        default: 
                            break;
                    }
                    break;
                case IDC_WAV_SAMPLE_RATE:
                case IDC_WAV_BITS_PER_SAMPLE:
                case IDC_WAV_CHANNELS:
                case IDC_CIRCBUFFER_COMBO:
                case IDC_COMBO1:
                    switch (HIWORD(wParam))
                    {
                        case CBN_SELCHANGE:
                            /* Make a copy of the master settings */
                            receiver_settings_copy(&g_controls->other_copy_, &g_controls->settings_);
                            /* Fill the copy with what controls have */
                            if (combo_controls_to_data(&g_controls->other_copy_, g_controls) 
                                && receiver_settings_validate(&g_controls->other_copy_))
                            {
                                receiver_settings_copy(&g_controls->settings_, &g_controls->other_copy_);
                                EnableWindow(g_controls->btok_, TRUE);
                            }
                            else
                            {
                                EnableWindow(g_controls->btok_, FALSE);
                            }
                            break;
                        default: 
                            break;
                    }
                    break;
                case IDC_MCAST_SETTINGS: 
                    get_settings_from_dialog(hDlg, &g_controls->settings_.mcast_settings_);
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
    struct receiver_settings_dlg_controls controls;
    play_settings_get_default(&controls.settings_.play_settings_);
    receiver_settings_copy(&controls.settings_, p_settings);
    /* NULL hInst means = read dialog template from this application's resource file */
    if (IDOK == DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_RECEIVER_SETTINGS), hWndParent, McastSettingsProc, (LPARAM)&controls))
    {
        waveformat_normalize(&controls.settings_.wfex_);
        receiver_settings_copy(p_settings, &controls.settings_);
        return 1;
    }
    return 0;
}

