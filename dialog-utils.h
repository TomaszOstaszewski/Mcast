/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @brief Interface for the common utility code used all over the place with dialogs.
 * @file dialog-utils.h
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
#if !defined DIALOG_UTILS_D9665468_89A4_4C66_8F1B_5F7B949EB7AB
#define DIALOG_UTILS_D9665468_89A4_4C66_8F1B_5F7B949EB7AB

#if defined __cplusplus
extern "C" {
#endif

#include <windows.h>
#include "std-int.h"

/*!
 * @brief Reads the contents of the edit control and converts it to a decimal number in range 0 to 2^32-1
 * @details The boilerplate code of this function was repeated all over the place. Therefore, the decision
 * was made to create a separate function instead.
 * @param[in] hControl handle to the control.
 * @param[in] p_target reference to the caller allocated memory location, which will be written with a number
 * resulting from the conversion.
 * @return returns non-zero on success, otherwise returns 0.
 */
int get_from_edit_uint32_dec(HWND hControl, uint32_t * p_target);

/*!
 * @brief Reads the contents of the edit control and converts it to a decimal number in range 0 to 2^16-1
 * @details The boilerplate code of this function was repeated all over the place. Therefore, the decision
 * was made to create a separate function instead.
 * @param[in] hControl handle to the control.
 * @param[in] p_target reference to the caller allocated memory location, which will be written with a number
 * resulting from the conversion.
 * @return returns non-zero on success, otherwise returns 0.
 */
int get_from_edit_uint16_dec(HWND hControl, uint16_t * p_target);

/*!
 * @brief Reads the contents of the edit control and converts it to a decimal number in range 0 to 2^8-1
 * @details The boilerplate code of this function was repeated all over the place. Therefore, the decision
 * was made to create a separate function instead.
 * @param[in] hControl handle to the control.
 * @param[in] p_target reference to the caller allocated memory location, which will be written with a number
 * resulting from the conversion.
 * @return returns non-zero on success, otherwise returns 0.
 */
int get_from_edit_uint8_dec(HWND hControl, uint8_t * p_target);

int put_in_edit_control_uint32(HWND hControl, uint32_t value);

int put_in_edit_control_uint16(HWND hControl, uint16_t value);

int put_in_edit_control_uint8(HWND hControl, uint8_t value);

#if defined __cplusplus
}
#endif

#endif /* DIALOG_UTILS_D9665468_89A4_4C66_8F1B_5F7B949EB7AB */

