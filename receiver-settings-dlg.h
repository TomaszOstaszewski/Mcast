/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file receiver-settings-dlg.h
 * @brief Declares the interface needed to obtain receiver's settings from UI, via means of the modal dialog box.
 * @author T.Ostaszewski
 * @date Jan-2012
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
#if !defined RECEIVER_SETTINGS_DLG_4C679ECA_2A7A_4CDF_9FE0_FB4BE43B52C2
#define RECEIVER_SETTINGS_DLG_4C679ECA_2A7A_4CDF_9FE0_FB4BE43B52C2

#if defined __cplusplus
extern "C" {
#endif
#include <windows.h>

struct receiver_settings;

/*!
 * @brief Displays a dialog from which settings are obtained.
 * @details Dialog controls allow for modification of the parameters.
 * @param[in] hWndParent handle to the parent window.
 * @param[in,out] p_settings points to the settings which will be altered by the dialog
 * @return returns <>0 if the settings were changed by the dialog (UI), returns 0 otherwise.
 */
int receiver_settings_do_dialog(HWND hWndParent, struct receiver_settings * p_settings);

#if defined __cplusplus
}
#endif

#endif /* RECEIVER_SETTINGS_DLG_4C679ECA_2A7A_4CDF_9FE0_FB4BE43B52C2 */

