#if !defined MESSAGE_LOOP_CAA4787E_790C_417E_BD87_00C23412921E
#define MESSAGE_LOOP_CAA4787E_790C_417E_BD87_00C23412921E

#include <windows.h>

/*!
 * @brief 
 */
typedef long int (*P_ON_IDLE)(HWND hWnd, long int count);

/*!
 * @brief 
 */
WPARAM message_loop(HWND hWnd, P_ON_IDLE idle_func);

#endif /*!defined MESSAGE_LOOP_CAA4787E_790C_417E_BD87_00C23412921E */
