/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file mcastui.c
 * @author T. Ostaszewski
 * @date 04-Jan-2011
 * @brief 
 * @details 
 */

#include "pcc.h"
#include "mcastui.h"
#include "conn_data.h"
#include "mcast_setup.h"
#include "resource.h"
#include "debug_helpers.h"
#include "conn_entry.h"
#include "conn_table.h"
#include "dsoundplay.h"
#include "fifo-circular-buffer.h"
#include "wave_utils.h"

#define ErrorHandler() ErrorHandlerEx(__LINE__, __FILE__)

struct fifo_circular_buffer * g_fifo;

struct conn_data g_connection_data;
HINSTANCE   g_hInst;
static HWND g_hWndListView;
TCHAR       g_szClassName[] = TEXT("VListVwClass");

#define TIMER_1_TIMEOUT_MS (1000)
#define TIMEOUT_MS (80)

#define DIALOG_INFO_ITEM_COUNT (256)

const char g_pszWavFile[] = "play.wav";

UINT g_items_count;
master_riff_chunk_t * g_pWavChunk;
UINT_PTR g_timer_1;

DSOUNDPLAY g_play;
BOOL in_play = FALSE;
HANDLE g_hWavFile;
HANDLE g_receiverThread = INVALID_HANDLE_VALUE;

long int on_idle(long int count)
{
    return 0;
}

BOOL InitApplication(HINSTANCE hInstance, WNDPROC aWndProc)
{
    WNDCLASSEX  wcex;
    ATOM        aReturn;

    wcex.cbSize          = sizeof(WNDCLASSEX);
    wcex.style           = 0;
    wcex.lpfnWndProc     = (WNDPROC)aWndProc;
    wcex.cbClsExtra      = 0;
    wcex.cbWndExtra      = 0;
    wcex.hInstance       = hInstance;
    wcex.hCursor         = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground   = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName    = MAKEINTRESOURCE(IDM_MAIN_MENU);
    wcex.lpszClassName   = g_szClassName;
    wcex.hIcon           = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_MAINICON));
    wcex.hIconSm         = LoadImage(g_hInst, MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, 16, 16, 0);

    aReturn = RegisterClassEx(&wcex);

    if(0 == aReturn)
    {
        WNDCLASS wc;
        wc.style          = 0;
        wc.lpfnWndProc    = (WNDPROC)aWndProc;
        wc.cbClsExtra     = 0;
        wc.cbWndExtra     = 0;
        wc.hInstance      = hInstance;
        wc.hIcon          = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_MAINICON));
        wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszMenuName   = MAKEINTRESOURCE(IDM_MAIN_MENU);
        wc.lpszClassName  = g_szClassName;
        aReturn = RegisterClass(&wc);
    }
    return aReturn;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND     hWnd;
    TCHAR    szTitle[MAX_PATH] = TEXT("");

    g_hInst = hInstance;
    LoadString(g_hInst, IDS_APPTITLE, szTitle, sizeof(szTitle)/sizeof(szTitle[0]));
    /* Create a main window for this application instance.  */
    hWnd = CreateWindowEx(  0,
            g_szClassName,
            szTitle,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            NULL,
            NULL,
            hInstance,
            NULL);

    /* If window could not be created, return "failure" */
    if (!hWnd)
        return FALSE;
    /* Make the window visible; update its client area; and return "success" */
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

#define MAX_TEI_LEN (128) 

int update_controls_with_dialog_info(HWND hDlg, struct conn_entry * p_info)
{
    HWND hRadioBrick = GetDlgItem(hDlg, IDC_BRICK);
    HWND hRadioControlHead = GetDlgItem(hDlg, IDC_CONTROL_HEAD);
    HWND hTEI = GetDlgItem(hDlg, IDC_TEI); 
    HWND hId = GetDlgItem(hDlg, IDC_ID); 
    TCHAR psz_tei[MAX_TEI_LEN];
    if (TETRA_BRICK == p_info->type_)
    {
        SendMessage(hRadioBrick, BM_SETCHECK, BST_CHECKED, 0);
        SendMessage(hRadioControlHead, BM_SETCHECK, BST_UNCHECKED, 0);
    }
    else
    {
        SendMessage(hRadioBrick, BM_SETCHECK, BST_UNCHECKED, 0);
        SendMessage(hRadioControlHead, BM_SETCHECK, BST_CHECKED, 0);
    }
    tei_2_string(&p_info->tei_, psz_tei, MAX_TEI_LEN);
    SetWindowText(hTEI, psz_tei);
    return 0;
}

int update_dialog_info_with_controls(HWND hDlg, struct conn_entry * p_entry)
{
    HWND hRadioBrick = GetDlgItem(hDlg, IDC_BRICK);
    HWND hRadioControlHead = GetDlgItem(hDlg, IDC_CONTROL_HEAD);
    HWND hTEI = GetDlgItem(hDlg, IDC_TEI); 
    HWND hId = GetDlgItem(hDlg, IDC_ID); 
    UINT state = Button_GetCheck(hRadioBrick);
    if (BST_CHECKED == state)
    {
        p_entry->type_ = TETRA_BRICK;
        debug_outputln("%s %d", __FILE__, __LINE__);
    }
    else
    {
        p_entry->type_ = TETRA_CONTROL_HEAD;
        debug_outputln("%s %d", __FILE__, __LINE__);
    }
    GetWindowText(hId, p_entry->id_, sizeof(p_entry->id_));
    return 0;
}

int remove_update_dialog_info_with_controls(HWND hDlg, struct conn_entry * p_entry)
{
    HWND hTEI = GetDlgItem(hDlg, IDC_TEI); 
    HWND hId = GetDlgItem(hDlg, IDC_ID); 
    GetWindowText(hId, p_entry->id_, sizeof(p_entry->id_));
    return 0;
}

INT_PTR CALLBACK RemoveDlgProc( HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    static struct conn_entry * p_info = NULL;
    switch (uMessage)
    {
        case WM_INITDIALOG:
            p_info = (struct conn_entry *)lParam;
            return TRUE;
        case WM_COMMAND:
            switch(wParam)
            {
                case IDOK:
                    remove_update_dialog_info_with_controls(hDlg, p_info);
                    EndDialog(hDlg, IDOK);
                    break;
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;
            }
            return TRUE;
    } 
    return FALSE;
}

INT_PTR CALLBACK AddDlgProc( HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    static struct conn_entry * p_info = NULL;
    switch (uMessage)
    {
        case WM_INITDIALOG:
            p_info = (struct conn_entry *)lParam;
            update_controls_with_dialog_info(hDlg, p_info);
            return TRUE;
        case WM_COMMAND:
            switch(wParam)
            {
                case IDOK:
                    update_dialog_info_with_controls(hDlg, p_info);
                    EndDialog(hDlg, IDOK);
                    break;
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;
            }
            return TRUE;
    } 

    return FALSE;
}

void add_item(HWND g_hWndListView, HWND hWnd)
{
    INT result;
    struct conn_entry entry;
    memset(&entry, 0, sizeof(entry));
    generate_random_tei(&entry.tei_);
    result = DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_ADD_ITEM), hWnd, AddDlgProc, (LPARAM)&entry);
    if (IDOK==result)
    {
        LVITEM lvI;
        int item_index;
        size_t idx;
        struct conn_entry * p_entry;
        idx = find_first_unused(g_connection_data.p_conn_table_);
        if ((size_t)-1 == idx)
        {
            debug_outputln("%s %d", __FILE__, __LINE__);
            return;
        }
        p_entry = get_item(g_connection_data.p_conn_table_, idx);
        memcpy(p_entry, &entry, sizeof(entry)); 
        // Initialize LVITEM members that are common to all items.
        memset(&lvI, 0, sizeof(lvI));
        lvI.mask        = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_PARAM;
        lvI.pszText     = LPSTR_TEXTCALLBACK; 
        lvI.iItem       = g_items_count;
        lvI.lParam      = (LPARAM)p_entry;
        lvI.iImage      = I_IMAGECALLBACK;
        item_index      = ListView_InsertItem(g_hWndListView, &lvI);
        if (-1 != item_index)
        {
            ++g_items_count;
            mark_used(g_connection_data.p_conn_table_, idx);
        }
        else
        {
            debug_outputln("%s %d : %p %d", __FILE__, __LINE__, p_entry, GetLastError());
        }
    }
}

void remove_item(HWND g_hWndListView, HWND hWnd)
{
    INT result;
    size_t idx;
    struct conn_entry entry;
    memset(&entry, 0, sizeof(struct conn_entry));
    result = DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_REMOVE_ITEM), hWnd, RemoveDlgProc, (LPARAM)&entry);
    idx = find_by_id(g_connection_data.p_conn_table_, entry.id_); 
    if ((size_t)-1 != idx)
    {
        LVFINDINFO lvfi;
        int result;
        struct conn_entry * p_entry = get_item(g_connection_data.p_conn_table_, idx);
        memset(&lvfi, 0, sizeof(lvfi));
        lvfi.flags = LVFI_PARAM;
        lvfi.lParam = (LPARAM)p_entry;
        result = ListView_FindItem(g_hWndListView, (-1), &lvfi);    
        if (-1 != result)
        {
            ListView_DeleteItem(g_hWndListView, result);
        }
        else
        {
            debug_outputln("%s %d", __FILE__, __LINE__);
        }
    }
    else
    {
        debug_outputln("%s %d", __FILE__, __LINE__);
    }
}

INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage)
    {
        case WM_INITDIALOG:
            return TRUE;
        case WM_COMMAND:
            switch(wParam)
            {
                case IDOK:
                    EndDialog(hDlg, IDOK);
                    break;
                case IDCANCEL:
                    EndDialog(hDlg, IDOK);
                    break;
            }
            return TRUE;
    } 

    return FALSE;
}

/*!
 * \brief 
 * this needs to be called when the ListView is created, resized, the view is 
 * changed or a WM_SYSPARAMETERCHANGE message is received
 */
void PositionHeader(HWND g_hWndListView)
{
    HWND  hwndHeader = GetWindow(g_hWndListView, GW_CHILD);
    DWORD dwStyle = GetWindowLong(g_hWndListView, GWL_STYLE);

    /*To ensure that the first item will be visible, create the control without 
      the LVS_NOSCROLL style and then add it here*/
    dwStyle |= LVS_NOSCROLL;
    SetWindowLong(g_hWndListView, GWL_STYLE, dwStyle);

    //only do this if we are in report view and were able to get the header hWnd
    if(((dwStyle & LVS_TYPEMASK) == LVS_REPORT) && hwndHeader)
    {
        RECT        rc;
        HD_LAYOUT   hdLayout;
        WINDOWPOS   wpos;

        GetClientRect(g_hWndListView, &rc);
        hdLayout.prc = &rc;
        hdLayout.pwpos = &wpos;
        Header_Layout(hwndHeader, &hdLayout);
        SetWindowPos(hwndHeader, 
                wpos.hwndInsertAfter, 
                wpos.x, 
                wpos.y,
                wpos.cx, 
                wpos.cy, 
                wpos.flags | SWP_SHOWWINDOW);
        ListView_EnsureVisible(g_hWndListView, 0, FALSE);
    }
}

void ResizeListView(HWND g_hWndListView, HWND hwndParent)
{
    RECT  rc;
    GetClientRect(hwndParent, &rc);
    MoveWindow( g_hWndListView, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
    //only call this if we want the LVS_NOSCROLL style
#if 0
    PositionHeader(g_hWndListView);
#endif
}

HWND CreateListView(HINSTANCE hInstance, HWND hwndParent)
{
    DWORD       dwStyle;
    HWND        g_hWndListView;
    HIMAGELIST  himlSmall;
    HIMAGELIST  himlLarge;
    BOOL        bSuccess = TRUE;

    dwStyle =   WS_TABSTOP | 
        WS_CHILD | 
        WS_BORDER | 
        WS_VISIBLE |
        LVS_AUTOARRANGE |
        LVS_REPORT;

    g_hWndListView = CreateWindowEx(   WS_EX_CLIENTEDGE,          // ex style
            WC_LISTVIEW,               // class name - defined in commctrl.h
            TEXT(""),                        // dummy text
            dwStyle,                   // style
            0,                         // x position
            0,                         // y position
            0,                         // width
            0,                         // height
            hwndParent,                // parent
            (HMENU)ID_LISTVIEW,        // ID
            g_hInst,                   // instance
            NULL);                     // no extra data

    if(!g_hWndListView)
        return NULL;

    ResizeListView(g_hWndListView, hwndParent);

    //set the image lists
    himlSmall = ImageList_Create(16, 16, ILC_COLORDDB | ILC_MASK, 1, 0);
    himlLarge = ImageList_Create(32, 32, ILC_COLORDDB | ILC_MASK, 1, 0);

    if (himlSmall && himlLarge)
    {
        HICON hIcon;

        //set up the small image list
        hIcon = LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BRICK), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
        ImageList_AddIcon(himlSmall, hIcon);
        hIcon = LoadImage(g_hInst, MAKEINTRESOURCE(IDI_NGCH), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
        ImageList_AddIcon(himlSmall, hIcon);
        ListView_SetImageList(g_hWndListView, himlSmall, LVSIL_SMALL);

        //set up the large image list
        hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_BRICK));
        ImageList_AddIcon(himlLarge, hIcon);
        hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_NGCH));
        ImageList_AddIcon(himlLarge, hIcon);
        ListView_SetImageList(g_hWndListView, himlLarge, LVSIL_NORMAL);
    }
    return g_hWndListView;
}

BOOL InsertListViewItems(HWND g_hWndListView)
{
    //empty the list
    ListView_DeleteAllItems(g_hWndListView);
    return TRUE;
}

#define COLUMNS_COUNT (3)

BOOL InitListView(HWND hwndListView)
{
    LV_COLUMN   lvColumn;
    int         i;
    TCHAR       szString[COLUMNS_COUNT][20] = {TEXT("Device "), TEXT("TEI"), TEXT("Last update time")};

    //empty the list
    ListView_DeleteAllItems(hwndListView);

    //initialize the columns
    lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvColumn.fmt = LVCFMT_LEFT;
    lvColumn.cx = 120;
    for(i = 0; i < COLUMNS_COUNT; i++)
    {
        lvColumn.pszText = szString[i];
        ListView_InsertColumn(hwndListView, i, &lvColumn);
    }

    InsertListViewItems(hwndListView);
    ListView_SetCallbackMask(hwndListView, LVIS_STATEIMAGEMASK);
    return TRUE;
}

LRESULT ListViewNotify(HWND hWnd, LPARAM lParam)
{
    LPNMHDR  lpnmh = (LPNMHDR) lParam;
    HWND     g_hWndListView = GetDlgItem(hWnd, ID_LISTVIEW);
    switch(lpnmh->code)
    {
        case LVN_GETDISPINFO:
            {
                LV_DISPINFO *lpdi = (LV_DISPINFO *)lParam;
                struct conn_entry * p_entry = (struct conn_entry *)lpdi->item.lParam;
                if (NULL != p_entry)
                {
                    if (lpdi->item.mask & LVIF_TEXT)
                    {
                        switch (lpdi->item.iSubItem)
                        {
                            case 0:
                                lstrcpyn(lpdi->item.pszText, p_entry->id_, lpdi->item.cchTextMax);
                                break;
                            case 1:
                                tei_2_string(&p_entry->tei_, lpdi->item.pszText, lpdi->item.cchTextMax);
                                break;
                            case 2:
                            default:
                                break;
                        }
                    }
                    if(lpdi->item.mask & LVIF_IMAGE)
                    {
                        debug_outputln("%s %d : %s %d", __FILE__, __LINE__, p_entry->id_, p_entry->type_);
                        if (p_entry->type_ == TETRA_BRICK)
                        {
                            lpdi->item.iImage = 0;
                        }
                        else
                        {
                            lpdi->item.iImage = 1;
                        }
                    }
                }
                else
                {
                    debug_outputln("%s %d : %d", __FILE__, __LINE__, lpdi->item.iItem);
                }
            }
            return 0;
        case LVN_ODCACHEHINT:
            {
                LPNMLVCACHEHINT   lpCacheHint = (LPNMLVCACHEHINT)lParam;
                /*
                   This sample doesn't use this notification, but this is sent when the 
                   ListView is about to ask for a range of items. On this notification, 
                   you should load the specified items into your local cache. It is still 
                   possible to get an LVN_GETDISPINFO for an item that has not been cached, 
                   therefore, your application must take into account the chance of this 
                   occurring.
                 */
            }
            return 0;

        case LVN_ODFINDITEM:
            {
                LPNMLVFINDITEM lpFindItem = (LPNMLVFINDITEM)lParam;
                /*
                   This sample doesn't use this notification, but this is sent when the 
                   ListView needs a particular item. Return -1 if the item is not found.
                 */
            }
            return 0;
    }

    return 0;
}

void SwitchView(HWND g_hWndListView, DWORD dwView)
{
    DWORD dwStyle = GetWindowLong(g_hWndListView, GWL_STYLE);
    SetWindowLong(g_hWndListView, GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | dwView);
    ResizeListView(g_hWndListView, GetParent(g_hWndListView));
}

void UpdateMenu(HWND g_hWndListView, HMENU hMenu)
{
    UINT  uID = IDM_LIST;
    DWORD dwStyle;

    //uncheck all of these guys
    CheckMenuItem(hMenu, IDM_LARGE_ICONS,  MF_BYCOMMAND | MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_SMALL_ICONS,  MF_BYCOMMAND | MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_LIST,  MF_BYCOMMAND | MF_UNCHECKED);
    CheckMenuItem(hMenu, IDM_REPORT,  MF_BYCOMMAND | MF_UNCHECKED);
    if (in_play)
    {
        EnableMenuItem(hMenu, ID_SOUND_PLAY, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, ID_SOUND_PAUSE, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hMenu, ID_SOUND_STOP, MF_BYCOMMAND | MF_ENABLED);
    }
    else
    {
        EnableMenuItem(hMenu, ID_SOUND_PLAY, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(hMenu, ID_SOUND_PAUSE, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, ID_SOUND_STOP, MF_BYCOMMAND | MF_GRAYED);
    }

    //check the appropriate view menu item
    dwStyle = GetWindowLong(g_hWndListView, GWL_STYLE);
    switch(dwStyle & LVS_TYPEMASK)
    {
        case LVS_ICON:
            uID = IDM_LARGE_ICONS;
            break;

        case LVS_SMALLICON:
            uID = IDM_SMALL_ICONS;
            break;

        case LVS_LIST:
            uID = IDM_LIST;
            break;

        case LVS_REPORT:
            uID = IDM_REPORT;
            break;
    }
    CheckMenuRadioItem(hMenu, IDM_LARGE_ICONS, IDM_REPORT, uID,  MF_BYCOMMAND | MF_CHECKED);
}

BOOL DoContextMenu(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HWND  g_hWndListView = (HWND)wParam;
    HMENU hMenuLoad,
          hMenu;

    if(g_hWndListView != GetDlgItem(hWnd, ID_LISTVIEW))
        return FALSE;

    hMenuLoad = LoadMenu(g_hInst, MAKEINTRESOURCE(IDM_CONTEXT_MENU));
    hMenu = GetSubMenu(hMenuLoad, 0);

    UpdateMenu(g_hWndListView, hMenu);

    TrackPopupMenu(   hMenu,
            TPM_LEFTALIGN | TPM_RIGHTBUTTON,
            LOWORD(lParam),
            HIWORD(lParam),
            0,
            hWnd,
            NULL);

    DestroyMenu(hMenuLoad);

    return TRUE;
}

HANDLE g_fileMapping;
static WAVEFORMATEX g_waveformatex;

#define DEFAULT_WAV_CHUNK_SIZE (1024+256+128)

typedef struct data_send_descriptor { 
    uint8_t const * p_data_begin_;
    uint8_t const * p_current_pos_;
    uint32_t    max_offset_; 
    uint16_t    chunk_size_;
    HANDLE h_stop_event_;
} data_send_descriptor_t;

data_send_descriptor_t g_send_descriptor = { NULL, NULL, 0, 0, INVALID_HANDLE_VALUE};

/** 
 *
 */
static DWORD WINAPI SendThreadProc(LPVOID param)
{
    struct data_send_descriptor * p_desc;
    uint8_t const * p_data1;
    uint8_t const * p_data;

    p_desc = (struct data_send_descriptor *)param;
    p_data1 = (uint8_t const *)g_connection_data.connection_.multiAddr_;
    p_data = (uint8_t const *)g_connection_data.connection_.multiAddr_->ai_addr;
    p_desc->chunk_size_ = DEFAULT_WAV_CHUNK_SIZE;
    debug_outputln("%s %d : %u", __FILE__, __LINE__, g_connection_data.connection_.socket_);
    debug_outputln("%s %d : %p", __FILE__, __LINE__, p_data1);
    debug_outputln("%s %d : %p", __FILE__, __LINE__, p_data);
    debug_outputln("%s %d : %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x", __FILE__, __LINE__, 
            *p_data, *(p_data+1), *(p_data+2), *(p_data+3),*(p_data+4), *(p_data+5), *(p_data+6), *(p_data+7)
            );
    for (;;)
    {
        DWORD dwResult;
        dwResult = WaitForSingleObject(p_desc->h_stop_event_, TIMEOUT_MS);
        if (WAIT_TIMEOUT == dwResult)
        {
            uint32_t offset = p_desc->p_current_pos_ - p_desc->p_data_begin_;
            uint16_t chunk_size = p_desc->chunk_size_;
            if (offset + chunk_size > p_desc->max_offset_)
            {
                chunk_size = p_desc->max_offset_ - offset;
                debug_outputln("%s %d : %8.8u", __FILE__, __LINE__, chunk_size);
            }
            sendto(
                    (SOCKET)g_connection_data.connection_.socket_, 
                    (const char *)p_desc->p_current_pos_, 
                    chunk_size,
                    0,
                    g_connection_data.connection_.multiAddr_->ai_addr,
                    (int) g_connection_data.connection_.multiAddr_->ai_addrlen
                  );
            p_desc->p_current_pos_ += chunk_size;
            if ((uint32_t)(p_desc->p_current_pos_ - p_desc->p_data_begin_) + chunk_size >= p_desc->max_offset_)
            {
                debug_outputln("%s %d : %8.8x %8.8x", __FILE__, __LINE__, p_desc->p_current_pos_, p_desc->p_data_begin_ + p_desc->max_offset_);
                p_desc->p_current_pos_ = p_desc->p_data_begin_;
                p_desc->chunk_size_= DEFAULT_WAV_CHUNK_SIZE;
            }
        }
        else if (WAIT_OBJECT_0 == dwResult)
        {
            debug_outputln("%s %d", __FILE__, __LINE__);
            break;
        }
        else if (WAIT_FAILED == dwResult)
        {
            debug_outputln("%s %d", __FILE__, __LINE__);
            break;
        }
        else
        {
            debug_outputln("%s %d", __FILE__, __LINE__);
            break;
        }
    }
    return 0;
}

struct receiver_thread_params { 
    struct fifo_circular_buffer * p_fifo_;
    HANDLE h_stop_event_;
    SOCKET recv_socket_;
} g_recv_params = {
    NULL, 
    INVALID_HANDLE_VALUE,
    (SOCKET)INVALID_HANDLE_VALUE
};

static DWORD WINAPI ReceiverThreadProc(LPVOID param)
{
    struct receiver_thread_params * p_this = (struct receiver_thread_params *)param;
    uint16_t count = 1;
    struct data_item item;
    struct sockaddr_in sock_addr;
    socklen_t sock_addr_size;

    sock_addr_size  = sizeof(struct sockaddr_in);
    item.data_      = malloc(DATA_ITEM_SIZE);
    item.count_     = DATA_ITEM_SIZE;
    debug_outputln("%s %d : %8.8u %8.8x %8.8x %8.8x", __FILE__, __LINE__, g_connection_data.connection_.socket_, p_this->p_fifo_, p_this->h_stop_event_);
    for (; ; ++count)
    {
        int bytes_recevied;
        do { 
            bytes_recevied = recvfrom(
                    (SOCKET)g_connection_data.connection_.socket_, 
                    item.data_, 
                    item.count_, 
                    0,
                    (struct sockaddr*)&sock_addr,
                    &sock_addr_size
                    );
            if (SOCKET_ERROR != bytes_recevied)
            {
                //debug_outputln("%s %d : %8.8u", __FILE__, __LINE__, bytes_recevied);
                item.count_ = bytes_recevied;
                fifo_circular_buffer_push_item(p_this->p_fifo_, &item);
                break;
            }
            debug_outputln("%s %d : %8.8u", __FILE__, __LINE__, WSAGetLastError());
            item.data_ = realloc(item.data_, item.count_ + DATA_ITEM_SIZE);
            item.count_ += DATA_ITEM_SIZE;
        } while (WSAGetLastError() == WSAEMSGSIZE);
    }
    free(item.data_);
    return 0;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage)
    {
        case WM_CREATE:
            // create the TreeView control
            g_hWndListView = CreateListView(g_hInst, hWnd);
            g_hWavFile = CreateFile(g_pszWavFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
            g_timer_1 = SetTimer(hWnd, IDT_TIMER_1, TIMER_1_TIMEOUT_MS, NULL);
            g_fileMapping = CreateFileMapping(g_hWavFile, NULL, PAGE_READONLY,  0, 0, NULL);
            if (INVALID_HANDLE_VALUE != g_fileMapping)
            {   
                g_pWavChunk = (master_riff_chunk_t*)MapViewOfFile(g_fileMapping, FILE_MAP_READ, 0, 0, 0);
#if 1
                debug_outputln("%s %d : %x %x", __FILE__, __LINE__, (DWORD)g_hWavFile, (DWORD)g_fileMapping);
                debug_outputln("%s %d "
                        ": %c%c%c%c"
                        " %d",
                        __FILE__, __LINE__, 
                        g_pWavChunk->ckid_[0],
                        g_pWavChunk->ckid_[1],
                        g_pWavChunk->ckid_[2],
                        g_pWavChunk->ckid_[3],
                        g_pWavChunk->cksize_
                        );
                debug_outputln("%s %d "
                        ": %c%c%c%c"
                        " %c%c%c%c"
                        " %d",
                        __FILE__, __LINE__, 
                        g_pWavChunk->format_chunk_.waveid_[0],
                        g_pWavChunk->format_chunk_.waveid_[1],
                        g_pWavChunk->format_chunk_.waveid_[2],
                        g_pWavChunk->format_chunk_.waveid_[3],
                        g_pWavChunk->format_chunk_.ckid_[0],
                        g_pWavChunk->format_chunk_.ckid_[1],
                        g_pWavChunk->format_chunk_.ckid_[2],
                        g_pWavChunk->format_chunk_.ckid_[3],
                        g_pWavChunk->format_chunk_.cksize_
                        );
                debug_outputln("%s %d :"
                        " %4.4hx %4.4hx %8.8x %8.8x %4.4hx %4.4hx"
                        , __FILE__, __LINE__
                        ,g_pWavChunk->format_chunk_.format_.wFormatTag
                        ,g_pWavChunk->format_chunk_.format_.nChannels
                        ,g_pWavChunk->format_chunk_.format_.nSamplesPerSec
                        ,g_pWavChunk->format_chunk_.format_.nAvgBytesPerSec
                        ,g_pWavChunk->format_chunk_.format_.nBlockAlign
                        ,g_pWavChunk->format_chunk_.format_.wBitsPerSample
                        );
                debug_outputln("%s %d :"
                        " %c%c%c%c %8.8x"
                        , __FILE__, __LINE__
                        ,g_pWavChunk->format_chunk_.subchunk_.subchunkId_[0]
                        ,g_pWavChunk->format_chunk_.subchunk_.subchunkId_[1]
                        ,g_pWavChunk->format_chunk_.subchunk_.subchunkId_[2]
                        ,g_pWavChunk->format_chunk_.subchunk_.subchunkId_[3]
                        ,g_pWavChunk->format_chunk_.subchunk_.subchunk_size_
                        );
#endif
                copy_waveformatex_2_WAVEFORMATEX(&g_waveformatex, &g_pWavChunk->format_chunk_.format_);
                g_send_descriptor.p_data_begin_ = &g_pWavChunk->format_chunk_.subchunk_.samples8_[0];
                g_send_descriptor.p_current_pos_= &g_pWavChunk->format_chunk_.subchunk_.samples8_[0];
                g_send_descriptor.max_offset_ = g_pWavChunk->format_chunk_.subchunk_.subchunk_size_;
                g_fifo = fifo_circular_buffer_create();
                debug_outputln("%s %d : %8.8x", __FILE__, __LINE__, g_fifo);
                g_play = dsoundplayer_create(hWnd, &g_waveformatex, g_fifo);
                debug_outputln("%s %d : %8.8x", __FILE__, __LINE__, g_play);
            }
            else
            {
                debug_outputln("%s %d : %8.8x", __FILE__, __LINE__, GetLastError());
            }
            InitListView(g_hWndListView);
            break;
        case WM_NOTIFY:
            return ListViewNotify(hWnd, lParam);
        case WM_SIZE:
            ResizeListView(g_hWndListView, hWnd);
            break;
        case WM_INITMENUPOPUP:
            UpdateMenu(g_hWndListView, GetMenu(hWnd));
            break;
        case WM_CONTEXTMENU:
            if(DoContextMenu(hWnd, wParam, lParam))
                return FALSE;
            break;
        case WM_TIMER:
            if (g_timer_1 == (UINT_PTR)wParam)
            {
                if (g_connection_data.connection_.socket_ > 0)
                {
                    //act_on_timer(&g_connection_data);
                }
            }
            break;
        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDM_JOIN_MCAST:
                    {
                        int result = setup_mcast(&g_connection_data);
                        debug_outputln("%s %d : %d", __FILE__, __LINE__, result);
                    }
                    break;
                case ID_SOUND_PLAY:
                    debug_outputln("%s %d", __FILE__, __LINE__);
                    dsoundplayer_play(g_play);
                    in_play = TRUE;
                    break;
                case ID_SOUND_PAUSE:
                    debug_outputln("%s %d", __FILE__, __LINE__);
                    break;
                case ID_SOUND_STOP:
                    debug_outputln("%s %d", __FILE__, __LINE__);
                    dsoundplayer_stop(g_play);
                    in_play = FALSE;
                    break;
                case IDM_LARGE_ICONS:
                    SwitchView(g_hWndListView, LVS_ICON);
                    break;
                case IDM_SMALL_ICONS:
                    SwitchView(g_hWndListView, LVS_SMALLICON);
                    break;
                case IDM_LIST:
                    SwitchView(g_hWndListView, LVS_LIST);
                    break;
                case IDM_REPORT:
                    SwitchView(g_hWndListView, LVS_REPORT);
                    break;
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                case IDM_ABOUT:
                    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDlgProc);
                    break;   
                case IDM_ADD_ITEM:
                    add_item(g_hWndListView, hWnd);
                    break;
                case IDM_REMOVE_ITEM:
                    remove_item(g_hWndListView, hWnd);
                    break;
                case ID_ENTRIES_SENDDATA:
                    {
                        HANDLE hThread;
                        if (INVALID_HANDLE_VALUE == g_send_descriptor.h_stop_event_)
                        {
                            g_send_descriptor.h_stop_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
                            debug_outputln("%s %d : %8.8x", __FILE__, __LINE__, g_send_descriptor.h_stop_event_);
                        }
                        ResetEvent(g_send_descriptor.h_stop_event_);
                        hThread = CreateThread(NULL, 
                                0,
                                SendThreadProc,
                                &g_send_descriptor,
                                0,
                                NULL);
                        debug_outputln("%s %d : %8.8x", __FILE__, __LINE__, hThread);
                        CloseHandle(hThread);
                    }
                    break;
                case ID_ENTRIES_STOPSENDER:
                    if (INVALID_HANDLE_VALUE != g_send_descriptor.h_stop_event_)
                    {
                        debug_outputln("%s %d : %8.8x", __FILE__, __LINE__, g_send_descriptor.h_stop_event_);
                        SetEvent(g_send_descriptor.h_stop_event_);
                    }
                    else
                    {
                        debug_outputln("%s %d", __FILE__, __LINE__);
                    }
                    break;
                case ID_SOUND_STARTRECEIVER:
                    if (INVALID_HANDLE_VALUE == g_receiverThread)
                    {
                        if (INVALID_HANDLE_VALUE == g_recv_params.h_stop_event_)
                        {
                            g_recv_params.h_stop_event_ = CreateEvent(NULL, TRUE, FALSE, "recv-event-0");
                        }
                        ResetEvent(g_recv_params.h_stop_event_);
                        if (NULL == g_recv_params.p_fifo_)
                        {
                            g_recv_params.p_fifo_= g_fifo;
                        }
                        debug_outputln("%s %d : %8.8x %8.8x", __FILE__, __LINE__, g_recv_params.p_fifo_, g_recv_params.h_stop_event_);
                        g_receiverThread = CreateThread(NULL,
                                0, 
                                ReceiverThreadProc,
                                &g_recv_params,
                                0,
                                NULL);
                        debug_outputln("%s %d : %8.8x", __FILE__, __LINE__, g_receiverThread);
                    }
                    else
                    {
                        debug_outputln("%s %d : %8.8x", __FILE__, __LINE__, g_receiverThread);
                    }
                    break;
                case ID_SOUND_STOPRECEIVER:
                    if (INVALID_HANDLE_VALUE != g_receiverThread)
                    {
                        DWORD dwResult;
                        dwResult = SetEvent(g_recv_params.h_stop_event_);
                        debug_outputln("%s %d : %8.8x", __FILE__, __LINE__, dwResult);
                        dwResult = CloseHandle(g_receiverThread);
                        debug_outputln("%s %d : %8.8x", __FILE__, __LINE__, dwResult);
                        g_receiverThread = INVALID_HANDLE_VALUE;
                    }
                    break;
                default:
                    break;
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            break;
    }
    return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

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

int PASCAL WinMain(  HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
{
    LONG lIdleCount = 0;
    BOOL bIdle = TRUE;
    long int count;
    MSG  msg;
    HRESULT hr;
    g_hInst = hInstance;
    count = 0;

    if(!hPrevInstance)
        if(!InitApplication(hInstance, MainWndProc))
            return FALSE;

    hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        return FALSE;
    //required to use the common controls
    InitCommonControls();
    g_connection_data.p_conn_table_ = conn_table_create();

    /* Perform initializations that apply to a specific instance */
    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;
    /* Message loop with idle time processing */
    for (;;)
    {
        // phase1: check to see if we can do idle work
        while (bIdle && !PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            // call OnIdle while in bIdle state
            if (!on_idle(lIdleCount++))
                break;
        }
        // phase2: pump messages while available
        do
        {
            // pump message, but quit on WM_QUIT
            if (!GetMessage(&msg, NULL, 0, 0))
                return (int)msg.wParam;
            TranslateMessage(&msg);
            DispatchMessage(&msg);  
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

