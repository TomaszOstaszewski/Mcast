/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file message-loop.h
 * @author T.Ostaszewski
 * @date 03-Jan-2011
 * @brief Message loop helper routines header.
 */
#if !defined MESSAGE_LOOP_CAA4787E_790C_417E_BD87_00C23412921E
#define MESSAGE_LOOP_CAA4787E_790C_417E_BD87_00C23412921E

#include <windows.h>

/*!
 * @brief Typedef for the idle loop processing function, 
 * that is to be called whenever application is in idle. 
 * @details The semantics of the idle processing function is as follows:
 * When it returns a non-zero value, it will be called again, unless a message
 * appears in the message queue. When it returns 0, it will not be called again
 * until application hits another idle state, i.e. a message gets into its queue,
 * and undergoes proper processing.
 * The @i count @i parameter indicates whether this is the first turn of the idle
 * processing - it is zero then, or another one (it is then non-zero). 
 * @param count - a counter that is incremented each time this function is called.
 * This counter will be zeroed when a message appears in the message queue.
 * @return Return 0 if no more idle processing is needed, return non-zero 
 * if you need more idle time processing. 
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
 * @param idle_func - function to be called when there are no messages
 * in the queue.
 * @return 
 */
WPARAM message_loop(HWND hWnd, P_ON_IDLE idle_func);

#endif /*!defined MESSAGE_LOOP_CAA4787E_790C_417E_BD87_00C23412921E */
