#include "pcc.h"
#include "message-loop.h"

/*!
 * @brief 
 */
static BOOL IsIdleMessage(MSG* pMsg)
{
    static POINT m_ptCursorLast = { 0, 0 };
    static UINT m_nMsgLast = 0x00; 
    // Return FALSE if the message just dispatched should _not_
    // cause OnIdle to be run.  Messages which do not usually
    // affect the state of the user interface and happen very
    // often are checked for.

    // redundant WM_MOUSEMOVE and WM_NCMOUSEMOVE
    if (pMsg->message == WM_MOUSEMOVE || pMsg->message == WM_NCMOUSEMOVE)
    {
        // mouse move at same position as last mouse move?
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
            if (!(*idle_func)(hWnd, lIdleCount++))
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

