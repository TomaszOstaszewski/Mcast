/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @brief
 * @file sender-settings-dlg.h
 * @author T.Ostaszewski
 * @date Jan-2012
 */
#if !defined RECEIVER_SETTINGS_DLG_4C679ECA_2A7A_4CDF_9FE0_FB4BE43B52C2
#define RECEIVER_SETTINGS_DLG_4C679ECA_2A7A_4CDF_9FE0_FB4BE43B52C2

#if defined __cplusplus
extern "C" {
#endif
#include <windows.h>

struct receiver_settings;

int receiver_settings_do_dialog(HWND hWndParent, struct receiver_settings * p_settings);

#if defined __cplusplus
}
#endif

#endif /* RECEIVER_SETTINGS_DLG_4C679ECA_2A7A_4CDF_9FE0_FB4BE43B52C2 */

