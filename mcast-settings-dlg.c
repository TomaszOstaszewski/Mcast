/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/*!
 * @file mcast-settings-dlg.c
 * @brief Multicast settings dialog implementation.
 * @details This file contains implementation of the functions that allow the client to get multicast settings from the UI, via modal dialog window.
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
#include "mcast-settings.h"
#include "mcast-settings-dlg.h"
#include "debug_helpers.h"
#include "common-dialogs-res.h"

/*!
 * @brief Maximum number of characters to be typed in the port control.
 */
#define TEXT_LIMIT (5)

struct mcast_settings_dlg {
    /*!
     * @brief Copy of the mcast_settings object passed by the caller.  
     * @details It is this copy which is being altered by dialog controls. After the dialog is done, 
     * we copy it back.
     */
    struct mcast_settings settings_;
    struct mcast_settings settings_copy_;
    struct mcast_settings settings_copy_for_spins_;
    struct mcast_settings settings_copy_for_ip_;
    /*!
     * @brief Handle to the main dialog.
     */
    HWND hwnd_;
    /*!
     * @brief Buffer for characters typed in the port control.
     */
    TCHAR port_buffer_[TEXT_LIMIT+1];

    /*!
     * @brief Handle to the IP address control object.
     */ 
    HWND ipaddr_ctrl_;

    /*!
     * @brief Handle to the IP port edit control.
     */
    HWND ipport_edit_ctrl_;

    /*!
     * @brief Handle to the spin control associated with IP port edit control
     */
    HWND port_spin_;

    /*!
     * @brief Handle to the IP port edit control.
     */
    HWND btok_;
};

/*!
 * @brief A table that describes valid multicast address ranges for each of 4 octets of the IPv4 address.
 * sa http://en.wikipedia.org/wiki/Multicast_address 
 */
static const struct mcast_range {
    uint8_t low_; /*!< Low range */
    uint8_t high_; /*!< High range */
} valid_mcast_range_table[4] = {
    { 224, 239 },
    { 0, 255 },
    { 0, 255 },
    { 0, 255 },
};

static int set_dlg_window(HWND hwnd, struct mcast_settings_dlg * p_dlg)
{
    LONG result;
    SetLastError(0);
    result = SetWindowLong(hwnd, GWL_USERDATA, (LONG)p_dlg);
    if (0 == result)
       return 0 == GetLastError();
    return result;
}

static struct mcast_settings_dlg * get_dlg_window(HWND hwnd)
{
    struct mcast_settings_dlg * p_retval; 
    p_retval = (struct mcast_settings_dlg *)GetWindowLong(hwnd, GWL_USERDATA);
    return p_retval;
}

/*!
 * @brief Transfer from data to UI
 * @details Takes values from the settings object and presents them on the UI
 * @param[in] p_settings pointer to the settings object whose contents are to be presented on the screen.
 */
static void data_to_controls(struct mcast_settings_dlg * p_dlg, struct mcast_settings const * p_settings)
{
    NMIPADDRESS ipnotify;
    int result;
    size_t index;
    unsigned long ipaddr_host_order;
    uint8_t ip_addr[4];

    StringCchPrintf(p_dlg->port_buffer_, 8, "%hu", ntohs(p_settings->mcast_addr_.sin_port));
    SetWindowText(p_dlg->ipport_edit_ctrl_, p_dlg->port_buffer_);
    ipaddr_host_order = ntohl(p_dlg->settings_.mcast_addr_.sin_addr.s_addr);
    ip_addr[0] = (uint8_t)(0xff & (ipaddr_host_order >> 24));
    ip_addr[1] = (uint8_t)(0xff & (ipaddr_host_order >> 16));
    ip_addr[2] = (uint8_t)(0xff & (ipaddr_host_order >> 8));
    ip_addr[3] = (uint8_t)(0xff & (ipaddr_host_order >> 0));
    result = SendMessage(p_dlg->ipaddr_ctrl_, IPM_SETADDRESS, (WPARAM)0, 
        (LPARAM)MAKEIPADDRESS(ip_addr[0],ip_addr[1],ip_addr[2],ip_addr[3])); 
    /* The IP control does not send notify messages         */
    /* when address is changed via IPM_SETADDRESS.          */
    /* Therefore, simulate those notifications ourselvers.  */
    ipnotify.hdr.hwndFrom = p_dlg->ipaddr_ctrl_;
    ipnotify.hdr.idFrom = IDC_IPADDRESS1;
    ipnotify.hdr.code = IPN_FIELDCHANGED;
    for (index = 0; index < COUNTOF_ARRAY(ip_addr); ++index)
    {
        ipnotify.iField = index;
        ipnotify.iValue = ip_addr[index];
        SendMessage(p_dlg->hwnd_, WM_NOTIFY, 0, (LPARAM)&ipnotify);
    }
}

/*!
 * @brief Transfer from data to UI
 * @details Takes values from the settings object and presents them on the UI
 * @param[in] p_settings pointer to the settings object whose contents are to be presented on the screen.
 */
static int controls_to_data(struct mcast_settings_dlg * p_dlg, struct mcast_settings * p_settings)
{
    int result;
    unsigned int port_host_order;
    DWORD address;
    memset(p_dlg->port_buffer_, 0, sizeof(p_dlg->port_buffer_));
    *((WORD *)p_dlg->port_buffer_) = TEXT_LIMIT;
    SendMessage(p_dlg->ipport_edit_ctrl_, EM_GETLINE, 0, (LPARAM)p_dlg->port_buffer_);
    result = sscanf(p_dlg->port_buffer_, "%u", &port_host_order);
    /* The 5 digit figures in decimal don't fit into 2 bytes of hex.
     * Therefore we may need to make some exceptions for values above 65535 - we enter the 65535 instead.
     */
    if (result>0 && port_host_order <= USHRT_MAX)
    {
        p_settings->mcast_addr_.sin_port = htons((unsigned short)port_host_order);
        /* Internet address is not validated - the control itself validates it */
        SendMessage(p_dlg->ipaddr_ctrl_, IPM_GETADDRESS, (WPARAM)0, (LPARAM)&address);
        p_settings->mcast_addr_.sin_addr.s_addr = htonl(address);
        return 1;
    }
    return 0;
}

/*!
 * @brief Handler for WM_INITDIALOG message.
 * @details This handler does as follows:
 * \li Initializes the control handles
 * \li Presents the settings on the UI
 * @param[in] hwnd handle to the window that received WM_INITDIALOG message
 * @param[in] hwndFocus handle to the Window that is to be got the keyboard focus upon dialog initializing. 
 * @param[in] lParam client specific parameter passed to DialogBoxParam function. This is a way to pass to the
 * handler some client specific data.
 * @param returns TRUE if the window indicated as hWndFocus is to get keyboard focus. Returns FALSE otherwise.
 */
static BOOL Handle_wm_initdialog(HWND hwnd, HWND hWndFocus, struct mcast_settings_dlg * p_dlg)
{
    int result;
    p_dlg->hwnd_ = hwnd;
    p_dlg->ipaddr_ctrl_ = GetDlgItem(hwnd, IDC_IPADDRESS1);
    assert(p_dlg->ipaddr_ctrl_);
    p_dlg->ipport_edit_ctrl_ = GetDlgItem(hwnd, IDC_EDIT1);
    assert(p_dlg->ipport_edit_ctrl_);
    p_dlg->port_spin_ = GetDlgItem(hwnd, IDC_PORT_SPIN);
    assert(p_dlg->port_spin_);
    p_dlg->btok_ = GetDlgItem(hwnd, IDOK);
    assert(p_dlg->btok_);

    /* Set controls limits */
    result = SendMessage(p_dlg->ipaddr_ctrl_, IPM_SETRANGE, (WPARAM)0, 
        (LPARAM)MAKEIPRANGE(valid_mcast_range_table[0].low_,valid_mcast_range_table[0].high_)); 
    assert(0 != result);

    result = SendMessage(p_dlg->ipaddr_ctrl_, IPM_SETRANGE, (WPARAM)1, 
        (LPARAM)MAKEIPRANGE(valid_mcast_range_table[1].low_,valid_mcast_range_table[1].high_)); 
    assert(0 != result);

    result = SendMessage(p_dlg->ipaddr_ctrl_, IPM_SETRANGE, (WPARAM)2, 
        (LPARAM)MAKEIPRANGE(valid_mcast_range_table[2].low_,valid_mcast_range_table[2].high_)); 
    assert(0 != result);

    result = SendMessage(p_dlg->ipaddr_ctrl_, IPM_SETRANGE, (WPARAM)3, 
        (LPARAM)MAKEIPRANGE(valid_mcast_range_table[3].low_,valid_mcast_range_table[3].high_)); 
    assert(0 != result);

    SendMessage(p_dlg->ipport_edit_ctrl_, EM_SETLIMITTEXT, (WPARAM)TEXT_LIMIT, (LPARAM)0);
    
    /* Init controls with data provided by the client */
    data_to_controls(p_dlg, &p_dlg->settings_);
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
    struct mcast_settings_dlg * p_dlg;
    NMHDR * p_nmhdr;
    NMUPDOWN * p_nm_updown;
    NMIPADDRESS * p_nm_ipaddr;
    unsigned short port;
    unsigned long ipaddr;

    switch (uMessage)
    {
        case WM_INITDIALOG:
            set_dlg_window(hDlg, (struct mcast_settings_dlg*)lParam);
            p_dlg = get_dlg_window(hDlg);
            assert(p_dlg);
            return HANDLE_WM_INITDIALOG(hDlg, wParam, p_dlg, Handle_wm_initdialog);
        case WM_NOTIFY:
            p_dlg = get_dlg_window(hDlg);
            assert(p_dlg);
            
            p_nmhdr = (NMHDR*)lParam;
            switch (p_nmhdr->code)
            {
                case IPN_FIELDCHANGED:
                    p_nm_ipaddr = (NMIPADDRESS*)p_nmhdr; 
                    mcast_settings_copy(&p_dlg->settings_copy_for_ip_, &p_dlg->settings_);
                    ipaddr = ntohl(p_dlg->settings_copy_for_ip_.mcast_addr_.sin_addr.s_addr);
                    switch (p_nm_ipaddr->iField)
                    {
                        case 0:
                            ipaddr &= (ipaddr & 0x00ffffff) | ((p_nm_ipaddr->iValue & 0xff) << 24);
                            break;
                        case 1:
                            ipaddr &= (ipaddr & 0xff00ffff) | ((p_nm_ipaddr->iValue & 0xff) << 16);
                            break;
                        case 2:
                            ipaddr &= (ipaddr & 0xffff00ff) | ((p_nm_ipaddr->iValue & 0xff) << 8);
                            break;
                        case 3:
                            ipaddr &= (ipaddr & 0xffffff00) | (p_nm_ipaddr->iValue & 0xff);
                            break;
                        default:
                            break;
                    }
                    p_dlg->settings_copy_for_ip_.mcast_addr_.sin_addr.s_addr = htonl(ipaddr);
                    if (mcast_settings_validate(&p_dlg->settings_copy_for_ip_))
                    {
                        mcast_settings_copy(&p_dlg->settings_, &p_dlg->settings_copy_for_ip_);
                        EnableWindow(p_dlg->btok_, TRUE);
                    }
                    else
                    {
                        EnableWindow(p_dlg->btok_, FALSE);
                    }
                    break;
                case UDN_DELTAPOS:
                    p_nm_updown = (NMUPDOWN *)p_nmhdr;
                    mcast_settings_copy(&p_dlg->settings_copy_for_spins_, &p_dlg->settings_);
                    switch (p_nm_updown->hdr.idFrom)
                    {
                        case IDC_PORT_SPIN:
                            port = ntohs(p_dlg->settings_copy_for_spins_.mcast_addr_.sin_port);
                            port -= p_nm_updown->iDelta;
                            p_dlg->settings_copy_for_spins_.mcast_addr_.sin_port = htons(port);
                            break;
                        default:
                            break;
                    }
                    if (!mcast_settings_compare(&p_dlg->settings_copy_for_spins_, &p_dlg->settings_) 
                        && mcast_settings_validate(&p_dlg->settings_copy_for_spins_))
                    {
                        /* No need to enable or disable OK button here.                         */
                        /* the data_to_controls() triggers notificatinos from edit controls,    */
                        /* which result in appropriate validation code being run                */
                        data_to_controls(p_dlg, &p_dlg->settings_copy_for_spins_);
                    }
                default:
                    break;
            }
            break;
        case WM_COMMAND:
            p_dlg = get_dlg_window(hDlg);
            assert(p_dlg);
            /* Handle notifications that come in form of WM_COMMAND messages */
            switch (LOWORD(wParam))
            {
                case IDC_EDIT1:
                    mcast_settings_copy(&p_dlg->settings_copy_, &p_dlg->settings_);
                    if (controls_to_data(p_dlg, &p_dlg->settings_copy_) 
                        && mcast_settings_validate(&p_dlg->settings_copy_))
                    {
                        mcast_settings_copy(&p_dlg->settings_, &p_dlg->settings_copy_);
                        EnableWindow(p_dlg->btok_, TRUE);  
                    }
                    else
                    {
                        EnableWindow(p_dlg->btok_, FALSE);  
                    }
                    break;
                case IDCANCEL:
                case IDOK:
                    EndDialog(hDlg, wParam);
                    return TRUE;
                default:
                    break;
            }
        default:
            break;
    }
    return FALSE;
}

int get_settings_from_dialog(HWND hParent, struct mcast_settings * p_settings)
{
    struct mcast_settings_dlg dlg;
    ZeroMemory(&dlg, sizeof(struct mcast_settings_dlg));
    mcast_settings_copy(&dlg.settings_, p_settings);
    /* NULL hInst means = read template from this application's resource file. */
    if (IDOK == DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_MCAST_SETTINGS), hParent, McastSettingsProc, (LPARAM)&dlg))
    {
        mcast_settings_copy(p_settings, &dlg.settings_);
        return 1;
    }
    return 0;
}

