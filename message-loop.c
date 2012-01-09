/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file message-loop.c
 * @author T.Ostaszewski
 * @date 03-Jan-2012
 * @brief Message loop helper routines.
 * @details This file contains definitions of the message processing 
 * routines used all over the place. Instead of copying them back and forth
 * I decided to put them into a separate file for, excuse me, reuse. 
 */
#include "pcc.h"
#include "message-loop.h"

/*!
 * @brief Determines if the message is an idle message. 
 * @details In general, messages which do not usually affect the state of the user interface and
 * happen very often are checked for. For instance, those can be redundant WM_MOUSEMOVE and WM_NCMOUSEMOVE
 * messages, whose parameters indicate that mouse move is at same position as last mouse move.
 * @param pMsg a message to be tested for idleness.
 * @return Returns FALSE if the message just dispatched <b>should not</b> cause idle processing function to be called.
 */
static BOOL IsIdleMessage(MSG* pMsg)
{
    static POINT m_ptCursorLast = { 0, 0 };
    static UINT m_nMsgLast = 0x00; 

    if (pMsg->message == WM_MOUSEMOVE || pMsg->message == WM_NCMOUSEMOVE)
    {
        if (0 == memcmp( &m_ptCursorLast, &pMsg->pt, sizeof(POINT)) && pMsg->message == m_nMsgLast)
            return FALSE;
        memcpy(&m_ptCursorLast, &pMsg->pt, sizeof(POINT));// remember for next time
        m_nMsgLast = pMsg->message;
        return TRUE;
    }

    // WM_PAINT and WM_SYSTIMER (caret blink)
    return pMsg->message != WM_PAINT && pMsg->message != 0x0118;
}

WPARAM message_loop(HWND hWnd, P_ON_IDLE idle_func)
{
    LONG lIdleCount = 0;
    MSG  msg;
    BOOL bIdle = TRUE;
    /* Message loop with idle time processing */
    for (;;)
    {
        // phase1: check to see if we can do idle work
        while (bIdle && !PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            // call OnIdle while in bIdle state
            if (idle_func && !(*idle_func)(hWnd, lIdleCount++))
                break;
        }
        // phase2: pump messages while available
        do
        {
            // pump message, but quit on WM_QUIT
            if (!GetMessage(&msg, NULL, 0, 0))
                return (int)msg.wParam;
            if (!IsDialogMessage(hWnd, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);  
            }
            // reset "no idle" state after pumping "normal" message
            if (IsIdleMessage(&msg))
            {
                bIdle = TRUE;
                lIdleCount = 0;
            }
        } while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE));
    }
    return (int)msg.wParam;
}

