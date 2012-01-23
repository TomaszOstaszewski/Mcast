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
 * @brief Copy of the mcast_settings object passed by the caller.  
 * @details It is this copy which is being altered by dialog controls. After the dialog is done, 
 * we copy it back.
 */
static struct mcast_settings g_settings;

/*!
 * @brief Handle to the IP address control object.
 */ 
static HWND g_ipaddr_ctrl;

/*!
 * @brief Handle to the IP port edit control.
 */
static HWND g_ipport_edit_ctrl;

static HWND port_spin_;

/*!
 * @brief Handle to the IP port edit control.
 */
static HWND g_btok;

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

/*!
 * @brief Maximum number of characters to be typed in the port control.
 */
#define TEXT_LIMIT (5)

/*!
 * @brief Buffer for characters typed in the port control.
 */
static TCHAR port_buffer[TEXT_LIMIT+1];

/*!
 * @brief Transfer from data to UI
 * @details Takes values from the settings object and presents them on the UI
 * @param[in] p_settings pointer to the settings object whose contents are to be presented on the screen.
 */
static void data_to_controls(struct mcast_settings const * p_settings)
{
    int result;
    unsigned long ipaddr_host_order;
    uint8_t ip_addr[4];
    StringCchPrintf(port_buffer, 8, "%hu", ntohs(p_settings->mcast_addr_.sin_port));
    debug_outputln("%s %4.4u : %s %u", __FILE__, __LINE__, port_buffer, ntohs(p_settings->mcast_addr_.sin_port));
    SetWindowText(g_ipport_edit_ctrl, port_buffer);
    ipaddr_host_order = ntohl(g_settings.mcast_addr_.sin_addr.s_addr);
    ip_addr[0] = (uint8_t)(0xff & (ipaddr_host_order >> 24));
    ip_addr[1] = (uint8_t)(0xff & (ipaddr_host_order >> 16));
    ip_addr[2] = (uint8_t)(0xff & (ipaddr_host_order >> 8));
    ip_addr[3] = (uint8_t)(0xff & (ipaddr_host_order >> 0));
    result = SendMessage(g_ipaddr_ctrl, IPM_SETADDRESS, (WPARAM)0, (LPARAM)MAKEIPADDRESS(ip_addr[0],ip_addr[1],ip_addr[2],ip_addr[3])); 
    /* Notiy oursevels */
}

/*!
 * @brief Transfer from data to UI
 * @details Takes values from the settings object and presents them on the UI
 * @param[in] p_settings pointer to the settings object whose contents are to be presented on the screen.
 */
static int controls_to_data(struct mcast_settings * p_settings)
{
    int result;
    unsigned int port_host_order;
    DWORD address;
    memset(port_buffer, 0, sizeof(port_buffer));
    *((WORD *)port_buffer) = TEXT_LIMIT;
    SendMessage(g_ipport_edit_ctrl, EM_GETLINE, 0, (LPARAM)port_buffer);
    result = sscanf(port_buffer, "%u", &port_host_order);
    /* The 5 digit figures in decimal don't fit into 2 bytes of hex.
     * Therefore we may need to make some exceptions for values above 65535 - we enter the 65535 instead.
     */
    debug_outputln("%s %4.4u : %d %s %u", __FILE__, __LINE__, result, port_buffer, port_host_order);
    if (result<=0 || port_host_order > USHRT_MAX)
        goto error;
    p_settings->mcast_addr_.sin_port = htons((unsigned short)port_host_order);
    SendMessage(g_ipaddr_ctrl, IPM_GETADDRESS, (WPARAM)0, (LPARAM)&address);
    p_settings->mcast_addr_.sin_addr.s_addr = htonl(address);
    return 1;
error:
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
static BOOL Handle_wm_initdialog(HWND hwnd, HWND hWndFocus, LPARAM lParam)
{
    int result;
    g_ipaddr_ctrl = GetDlgItem(hwnd, IDC_IPADDRESS1);
    assert(g_ipaddr_ctrl);
    g_ipport_edit_ctrl = GetDlgItem(hwnd, IDC_EDIT1);
    assert(g_ipport_edit_ctrl);
    port_spin_ = GetDlgItem(hwnd, IDC_PORT_SPIN);
    assert(port_spin_);

    result = SendMessage(g_ipaddr_ctrl, IPM_SETRANGE, (WPARAM)0, (LPARAM)MAKEIPRANGE(valid_mcast_range_table[0].low_,valid_mcast_range_table[0].high_)); 
    assert(0 != result);
    result = SendMessage(g_ipaddr_ctrl, IPM_SETRANGE, (WPARAM)1, (LPARAM)MAKEIPRANGE(valid_mcast_range_table[1].low_,valid_mcast_range_table[1].high_)); 
    assert(0 != result);
    result = SendMessage(g_ipaddr_ctrl, IPM_SETRANGE, (WPARAM)2, (LPARAM)MAKEIPRANGE(valid_mcast_range_table[2].low_,valid_mcast_range_table[2].high_)); 
    assert(0 != result);
    result = SendMessage(g_ipaddr_ctrl, IPM_SETRANGE, (WPARAM)3, (LPARAM)MAKEIPRANGE(valid_mcast_range_table[3].low_,valid_mcast_range_table[3].high_)); 
    assert(0 != result);
    SendMessage(g_ipport_edit_ctrl, EM_SETLIMITTEXT, (WPARAM)TEXT_LIMIT, (LPARAM)0);
    
    g_btok = GetDlgItem(hwnd, IDOK);
    assert(g_btok);
    data_to_controls(&g_settings);
    EnableWindow(g_btok, TRUE);
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
    static struct mcast_settings settings_copy;
    static struct mcast_settings settings_copy_for_spins;
    static struct mcast_settings settings_copy_for_ip;
    NMHDR * p_nmhdr;
    NMUPDOWN * p_nm_updown;
    NMIPADDRESS * p_nm_ipaddr;
    unsigned short port;
    unsigned long ipaddr;
    switch (uMessage)
    {
        case WM_INITDIALOG:
            return HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, Handle_wm_initdialog);
        case WM_NOTIFY:
            p_nmhdr = (NMHDR*)lParam;
            switch (p_nmhdr->code)
            {
                case IPN_FIELDCHANGED:
                    p_nm_ipaddr = (NMIPADDRESS*)p_nmhdr; 
                    mcast_settings_copy(&settings_copy_for_ip, &g_settings);
                    ipaddr = ntohl(settings_copy_for_ip.mcast_addr_.sin_addr.s_addr);
                    debug_outputln("%s %4.4u : %8.8x", __FILE__, __LINE__, ipaddr);
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
                    debug_outputln("%s %4.4u : %8.8x", __FILE__, __LINE__, ipaddr);
                    break;
                case UDN_DELTAPOS:
                    p_nm_updown = (NMUPDOWN *)p_nmhdr;
                    mcast_settings_copy(&settings_copy_for_spins, &g_settings);
                    debug_outputln("%s %4.4u", __FILE__, __LINE__);
                    switch (p_nm_updown->hdr.idFrom)
                    {
                        case IDC_PORT_SPIN:
                            debug_outputln("%s %4.4u", __FILE__, __LINE__);
                            port = ntohs(settings_copy_for_spins.mcast_addr_.sin_port);
                            port -= p_nm_updown->iDelta;
                            settings_copy_for_spins.mcast_addr_.sin_port = htons(port);
                            debug_outputln("%s %4.4u : %5.5u", __FILE__, __LINE__, port);
                            break;
                        default:
                            debug_outputln("%s %4.4u", __FILE__, __LINE__);
                            break;
                    }
                    if (!mcast_settings_compare(&settings_copy_for_spins, &g_settings))
                    {
                        //if (!mcast_settings_compare(&settings_copy_for_spins, &g_settings) && mcast_settings_validate(&settings_copy_for_spins))
                        debug_outputln("%s %4.4u", __FILE__, __LINE__);
                        //if (mcast_settings_validate(&settings_copy_for_spins))
                        {
                            //debug_outputln("%s %4.4u : %5.5hu %5.5hu", __FILE__, __LINE__, );
                            data_to_controls(&settings_copy_for_spins);
                        }
                    }
                default:
                    break;
            }
            break;
        case WM_COMMAND:
            /* Handle notifications that come in form of WM_COMMAND messages */
            switch (LOWORD(wParam))
            {
                case IDC_EDIT1:
                    debug_outputln("%s %4.4u", __FILE__, __LINE__);
                    mcast_settings_copy(&settings_copy, &g_settings);
                    debug_outputln("%s %4.4u", __FILE__, __LINE__);
                    if (controls_to_data(&settings_copy) && mcast_settings_validate(&settings_copy))
                    {
                        debug_outputln("%s %4.4u", __FILE__, __LINE__);
                        mcast_settings_copy(&g_settings, &settings_copy);
                        EnableWindow(g_btok, TRUE);  
                    }
                    else
                    {
                        EnableWindow(g_btok, FALSE);  
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
    mcast_settings_copy(&g_settings, p_settings);
    /* NULL hInst means = read template from this application's resource file. */
    if (IDOK == DialogBox(NULL, MAKEINTRESOURCE(IDD_MCAST_SETTINGS), hParent, McastSettingsProc))
    {
        mcast_settings_copy(p_settings, &g_settings);
        return 1;
    }
    return 0;
}

