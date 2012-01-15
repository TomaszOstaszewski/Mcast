/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @brief
 * @file sender-settings-dlg.h
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
 * @date Jan-2012
 */
#if !defined MCAST_SENDER_SETTINGS_DLG_BDDF0787_F748_479B_BB79_F5C26F739FB4
#define MCAST_SENDER_SETTINGS_DLG_BDDF0787_F748_479B_BB79_F5C26F739FB4

#if defined __cplusplus
extern "C" {
#endif
#include <windows.h>

struct sender_settings;

/*!
 * @brief Displays a dialog from which settings are obtained.
 * @details Displays a dialog from which settings are obtained.
 * @param[in] hWndParent handle to the parent window.
 * @param[in,out] p_settings points to the settings which will be altered by the dialog
 * @return returns <>0 if the settings were changed by the dialog (UI), returns 0 otherwise.
 */
int sender_settings_from_dialog(HWND hWndParent, struct sender_settings * p_settings);

#if defined __cplusplus
}
#endif

#endif /* !defined MCAST_SENDER_SETTINGS_DLG_BDDF0787_F748_479B_BB79_F5C26F739FB4 */

