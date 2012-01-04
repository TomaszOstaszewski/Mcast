/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file message-loop.h
 * @author T.Ostaszewski
 * @date 03-Jan-2011
 * @brief Message loop helper routines headers.
 * @details This file contains headers for some message processing 
 * routines used all over the place. Instead of copying them back and forth
 * I decided to put them into a separate file for, excuse me, reuse. 
 */
#if !defined MESSAGE_LOOP_CAA4787E_790C_417E_BD87_00C23412921E
#define MESSAGE_LOOP_CAA4787E_790C_417E_BD87_00C23412921E

#include <windows.h>

/*!
 * @brief Typedef for the idle loop processing function, 
 * that is to be called whenever application is in idle. 
 */
typedef long int (*P_ON_IDLE)(HWND hWnd, long int count);

/*!
 * @brief A message loop routine with idle processing. 
 * @details A more sophisticated wrap-around on the usual
 * @code
 * while (GetMessage(&msg, NULL, 0, 0))
 * {
 *  TranslateMessage(&msg);
 *  DispatchMessage(&msg);
 * }
 * @endcode
 * By saing 'more sophisticated' we mean that when there will be
 * no messages in the queue, the idle_func function will be called.
 * @param hWnd
 * @param idle_func 
 * @return 
 */
WPARAM message_loop(HWND hWnd, P_ON_IDLE idle_func);

#endif /*!defined MESSAGE_LOOP_CAA4787E_790C_417E_BD87_00C23412921E */
