/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file mcast-sender-dlg.c
 * @author
 * @date
 * @brief
 * @details
 */
#include "pcc.h"
#include "mcastui.h"
#include "conn_data.h"
#include "mcast_setup.h"
#include "resource.h"
#include "debug_helpers.h"
#include "dsoundplay.h"
#include "wave_utils.h"
#include "winsock_adapter.h"

/*!
 * 
 */
#define DEFAULT_CHUNK_SEND_TIMEOUT (85)

/*!
 *
 */
#define DEFAULT_MCASTADDRV4    "234.5.6.7"

/*!
 *
 */
#define DEFAULT_MCASTADDRV6    "ff12::1"

/*!
 *
 */
#define DEFAULT_MCASTPORT      "25000"

/*!
 *
 */
#define DEFAULT_TTL    (8)

/*!
 * @brief 
 */
#define DEFAULT_WAV_CHUNK_SIZE    (1024+256+128)

typedef enum sender_state { 
    SENDER_INITIAL = 0,
    SENDER_MCAST_JOINED = 1,
    SENDER_SENDING = 2,
    SENDER_LAST = 3
} sender_state_t;

/**
 * @brief
 * @details 
 */
typedef struct data_send_descriptor { 
    uint8_t const * p_current_pos_;
    DWORD           dw_chunk_send_timeout_;
    HANDLE          h_stop_event_;
    HANDLE          h_thread_;
    struct mcast_connection *       p_conn_;
    struct master_riff_chunk *    p_master_riff_;
} data_send_descriptor_t;

/*!
 * @brief
 * @details
 */
typedef struct mcast_settings {
    const char * ip4_addr_;
    const char * ip6_addr_;
    const char * port_;
    uint8_t     ttl_;
} mcast_settings_t;

struct sender_intial_data {
    struct mcast_settings * settings_;
};

struct sender_mcastjoined_data {
    struct mcast_settings * settings_;
    struct mcast_connection * conn_;
};

struct sender_sending_data {
    struct mcast_settings * settings_;
    struct mcast_connection * conn_;
    struct data_send_descriptor * sender_;
};

/**
 * @brief 
 */
HINSTANCE   g_hInst;

/**
 * 
 */
master_riff_chunk_t *   g_pWavChunk;

/**
 * 
 */
sender_state_t g_state;

/**
 * 
 */
void * g_state_data[SENDER_LAST];

/*!
 *
 */
static DWORD WINAPI SendThreadProc(LPVOID param)
{
    uint32_t max_offset;
    struct data_send_descriptor * p_desc;
    struct mcast_connection * p_conn;
    struct master_riff_chunk * p_master_riff;
    int8_t const * p_data_begin;
    HANDLE h_stop_event;
    DWORD dwResult;
    BOOL bDupResult;
    p_desc = (struct data_send_descriptor *)param;
    if (NULL == p_desc || NULL == p_desc->p_conn_ || NULL == p_desc->p_master_riff_)
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
        return -1;
    }   
    p_conn = p_desc->p_conn_;
    p_master_riff = p_desc->p_master_riff_;
    bDupResult = DuplicateHandle(GetCurrentProcess(), p_desc->h_stop_event_, GetCurrentProcess(), &h_stop_event, 0, FALSE, DUPLICATE_SAME_ACCESS);
    p_data_begin = &p_master_riff->format_chunk_.subchunk_.samples8_[0];
    max_offset = p_master_riff->format_chunk_.subchunk_.subchunk_size_;
    for (;;)
    {
        dwResult = WaitForSingleObject(h_stop_event, p_desc->dw_chunk_send_timeout_);
        if (WAIT_TIMEOUT == dwResult)
        {
            int result;
            uint32_t offset = p_desc->p_current_pos_ - p_data_begin;
            uint16_t chunk_size = DEFAULT_WAV_CHUNK_SIZE;
            if (offset + chunk_size > max_offset)
            {
                chunk_size = max_offset - offset;
            }
            result = sendto(p_desc->p_conn_->socket_, 
                    (const char *)p_desc->p_current_pos_, 
                    chunk_size,
                    0,
                    p_conn->multiAddr_->ai_addr,
                    (int) p_conn->multiAddr_->ai_addrlen
                  );
            p_desc->p_current_pos_ += chunk_size;
            if ((uint32_t)(p_desc->p_current_pos_ - p_data_begin) + chunk_size >= max_offset)
            {
                p_desc->p_current_pos_ = p_data_begin;
            }
        }
        else if (WAIT_OBJECT_0 == dwResult)
        {
            dwResult = 0;
            debug_outputln("%s %d", __FILE__, __LINE__);
            break;
        }
        else 
        {
            dwResult = -1;
            debug_outputln("%s %d : %d", __FILE__, __LINE__, dwResult);
            break;
        }
    }
    CloseHandle(h_stop_event);
    return dwResult;
}

/**
 * @brief
 *
 */
static void UpdateUI(HWND hDlg)
{
    static sender_state_t prev_state = -1;
    if (prev_state != g_state)
    {
        static HWND hSettingsBtn = NULL, hJoinMcastBtn = NULL, hLeaveMcast = NULL, hStartSendingBtn = NULL, hStopSendingBtn = NULL;
        static HMENU hMenu = NULL;
        if (NULL == hSettingsBtn)
            hSettingsBtn = GetDlgItem(hDlg, ID_SENDER_SETTINGS);    
        if (NULL == hJoinMcastBtn)
            hJoinMcastBtn = GetDlgItem(hDlg, ID_SENDER_JOINMCAST);  
        if (NULL == hStartSendingBtn)
            hStartSendingBtn = GetDlgItem(hDlg, ID_SENDER_STARTSENDING);    
        if (NULL == hStopSendingBtn)
            hStopSendingBtn = GetDlgItem(hDlg, ID_SENDER_STOPSENDING);  
        if (NULL == hLeaveMcast)
            hLeaveMcast = GetDlgItem(hDlg, ID_SENDER_LEAVEMCAST);
        if (NULL == hMenu)
            hMenu = GetMenu(hDlg);
        switch (g_state)
        {
            case SENDER_INITIAL:
                EnableMenuItem(hMenu, ID_SENDER_SETTINGS, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMenu, ID_SENDER_JOINMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMenu, ID_SENDER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, ID_SENDER_STARTSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, ID_SENDER_STOPSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(hSettingsBtn, TRUE);
                EnableWindow(hJoinMcastBtn, TRUE);
                EnableWindow(hLeaveMcast, FALSE);
                EnableWindow(hStartSendingBtn, FALSE);
                EnableWindow(hStopSendingBtn, FALSE);
                break;
            case SENDER_MCAST_JOINED:
                EnableMenuItem(hMenu, ID_SENDER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, ID_SENDER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, ID_SENDER_LEAVEMCAST, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMenu, ID_SENDER_STARTSENDING, MF_BYCOMMAND | MF_ENABLED);
                EnableMenuItem(hMenu, ID_SENDER_STOPSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableWindow(hSettingsBtn, FALSE);
                EnableWindow(hJoinMcastBtn, FALSE);
                EnableWindow(hLeaveMcast, TRUE);
                EnableWindow(hStartSendingBtn, TRUE);
                EnableWindow(hStopSendingBtn, FALSE);
                break;
            case SENDER_SENDING:
                EnableMenuItem(hMenu, ID_SENDER_SETTINGS, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, ID_SENDER_JOINMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, ID_SENDER_LEAVEMCAST, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, ID_SENDER_STARTSENDING, MF_BYCOMMAND | MF_GRAYED);
                EnableMenuItem(hMenu, ID_SENDER_STOPSENDING, MF_BYCOMMAND | MF_ENABLED);
                EnableWindow(hSettingsBtn, FALSE);
                EnableWindow(hJoinMcastBtn, FALSE);
                EnableWindow(hLeaveMcast, FALSE);
                EnableWindow(hStartSendingBtn, FALSE);
                EnableWindow(hStopSendingBtn, TRUE);
                break;
            default:
                break;
        }
        prev_state = g_state;
    }
}

/**
 * @brief
 * @return
 */
static data_send_descriptor_t * start_sending(struct mcast_connection * p_connection)
{
    struct data_send_descriptor * p_send_data;

    p_send_data = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(data_send_descriptor_t));
    if (NULL != p_send_data)
    {
        p_send_data->p_conn_ = p_connection;
        p_send_data->dw_chunk_send_timeout_ = DEFAULT_CHUNK_SEND_TIMEOUT;
        p_send_data->h_stop_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
        p_send_data->p_master_riff_ = g_pWavChunk;
        if (NULL != p_send_data->h_stop_event_)
        {
            ResetEvent(p_send_data->h_stop_event_);
            p_send_data->h_thread_ = CreateThread(NULL, 0, SendThreadProc, p_send_data, 0, NULL);
            debug_outputln("%s %d : %p", __FILE__, __LINE__, p_send_data);
            return p_send_data;
        }
        HeapFree(GetProcessHeap(), 0, p_send_data);
    }
    debug_outputln("%s %d : %p", __FILE__, __LINE__, p_send_data);
    return p_send_data;
}

/**
 * @brief
 * @param
 */
static void stop_sending(struct sender_sending_data * p_sending)
{
    debug_outputln("%s %d : %p", __FILE__, __LINE__, p_sending);
    SetEvent(p_sending->sender_->h_stop_event_);   
    CloseHandle(p_sending->sender_->h_thread_);
    CloseHandle(p_sending->sender_->h_stop_event_);
    HeapFree(GetProcessHeap(), 0, p_sending->sender_);
    p_sending->sender_ = NULL;
    debug_outputln("%s %d", __FILE__, __LINE__);
}

/**
 * @brief
 * @param
 */
static struct mcast_settings * get_mcast_settings(void)
{
    static struct mcast_settings  g_settings = {
        DEFAULT_MCASTADDRV4,
        DEFAULT_MCASTADDRV6,
        DEFAULT_MCASTPORT,
        DEFAULT_TTL
    };
    return &g_settings;
}

/**
 * @brief
 */
static struct sender_intial_data * create_initial_0(struct mcast_settings * p_settings)
{
    struct sender_intial_data * p_initial;
    p_initial = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct sender_intial_data));
    if (NULL != p_initial)
    {
        p_initial->settings_ = p_settings;
    }
    return p_initial;
}

/**
 * @brief
 */
static struct sender_sending_data * create_sending_0(struct sender_mcastjoined_data * p_mcastjoined)
{
    struct sender_sending_data * p_sending;
    p_sending = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct sender_sending_data));
    if (NULL != p_sending)
    {
        p_sending->settings_ = p_mcastjoined->settings_;
        p_sending->conn_ = p_mcastjoined->conn_; 
    }
    return p_sending;
}

/**
 * @brief
 */
static struct sender_mcastjoined_data * create_mcastjoined_0(struct sender_intial_data * p_initial)
{
    struct sender_mcastjoined_data * p_mcastjoined;
    p_mcastjoined = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct sender_mcastjoined_data));
    p_mcastjoined->settings_ = p_initial->settings_;
    return p_mcastjoined;
}

/**
 * @brief
 */
static int join_mcast(struct sender_mcastjoined_data * p_mcastjoined)
{
    p_mcastjoined->conn_ = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct mcast_connection));
    if (NULL != p_mcastjoined->conn_)
    {
        int rc;
        rc = setup_multicast_3(DEFAULT_MCASTADDRV4, DEFAULT_MCASTPORT, p_mcastjoined->conn_);
        if (0 == rc)
        {
            return 0;
        }
    }
    return (-1);
}

/**
 * @brief
 */
static int leave_mcast_0(struct sender_mcastjoined_data * p_mcastjoined)
{
    int result = close_multicast(p_mcastjoined->conn_);
    HeapFree(GetProcessHeap(), 0, p_mcastjoined->conn_);
    p_mcastjoined->conn_ = NULL;
    return result;
}

/**
 * @brief
 * 
 */
INT_PTR CALLBACK SenderDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    static struct conn_entry * p_info = NULL;
    switch (uMessage)
    {
        case WM_INITDIALOG:
            p_info = (struct conn_entry *)lParam;
            {
                int result = init_master_riff(&g_pWavChunk, g_hInst, MAKEINTRESOURCE(IDR_0_1));
                debug_outputln("%s %5.5d : %d %p", __FILE__, __LINE__, result, g_pWavChunk);
                g_state = SENDER_INITIAL;
                if (NULL == g_state_data[g_state])
                {
                    g_state_data[g_state] = create_initial_0(get_mcast_settings());
                }
                assert(NULL != g_state_data[g_state]);
            }
            UpdateUI(hDlg);
            return TRUE;
        case WM_INITMENUPOPUP:
            UpdateUI(hDlg);
            debug_outputln("%s %5.5d", __FILE__, __LINE__);
            break;
        case WM_COMMAND:
            switch(wParam)
            {
                case ID_SENDER_SETTINGS:
                    if (SENDER_INITIAL == g_state)
                    {
                        /* Open up the settings dialog with the MCAST settings parameters */
                    }
                    else
                    {
                        debug_outputln("%s %5.5d", __FILE__, __LINE__);
                    }
                    UpdateUI(hDlg);
                    break;
                case ID_SENDER_JOINMCAST:
                    if (SENDER_INITIAL == g_state)
                    {
                        struct sender_intial_data * p_initial;
                        struct sender_mcastjoined_data * p_mcastjoined;
                        p_initial = (struct sender_intial_data *)g_state_data[g_state];
                        assert(NULL != p_initial);
                        p_mcastjoined = (struct sender_mcastjoined_data*)g_state_data[SENDER_MCAST_JOINED];
                        if (NULL == p_mcastjoined)
                        {
                            debug_outputln("%s %5.5d", __FILE__, __LINE__);
                            p_mcastjoined = create_mcastjoined_0(p_initial);
                        }
                        if (0 == join_mcast(p_mcastjoined))
                        {
                            g_state = SENDER_MCAST_JOINED;
                            g_state_data[g_state] = p_mcastjoined;
                        }
                    }
                    else
                    {
                        debug_outputln("%s %5.5d", __FILE__, __LINE__);
                    }
                    UpdateUI(hDlg);
                    break;
                case ID_SENDER_LEAVEMCAST:
                    if (SENDER_MCAST_JOINED == g_state)
                    {
                        struct sender_intial_data * p_initial;
                        struct sender_mcastjoined_data * p_mcastjoined;
                        p_mcastjoined = (struct sender_mcastjoined_data*)g_state_data[g_state];
                        assert(NULL != p_mcastjoined);
                        p_initial = (struct sender_intial_data*)g_state_data[SENDER_INITIAL];
                        if (NULL != p_initial) 
                        {
                            p_initial = create_initial_0(p_mcastjoined->settings_);
                        }
                        leave_mcast_0(p_mcastjoined);
                        g_state = SENDER_INITIAL;
                        g_state_data[g_state] = p_initial;
                    }
                    else
                    {
                        debug_outputln("%s %5.5d", __FILE__, __LINE__);
                    }
                    UpdateUI(hDlg);
                    break;
                case ID_SENDER_STARTSENDING:
                    if (SENDER_MCAST_JOINED == g_state)
                    {
                        struct sender_mcastjoined_data * p_mcastjoined;
                        struct sender_sending_data * p_sending;
                        p_mcastjoined = (struct sender_mcastjoined_data*)g_state_data[g_state];
                        assert(NULL != p_mcastjoined);
                        p_sending = (struct sender_sending_data*)g_state_data[SENDER_SENDING]; 
                        if (NULL == p_sending)
                        {
                            p_sending = create_sending_0(p_mcastjoined);
                        }
                        p_sending->sender_ = start_sending(p_sending->conn_); 
                        if (NULL != p_sending->sender_)
                        {
                            g_state = SENDER_SENDING;
                            g_state_data[g_state] = p_sending;
                        }
                    }
                    else
                    {
                        debug_outputln("%s %5.5d", __FILE__, __LINE__);
                    }
                    UpdateUI(hDlg);
                    break;
                case ID_SENDER_STOPSENDING:
                    if (SENDER_SENDING == g_state)
                    {
                        struct sender_mcastjoined_data * p_mcastjoined;
                        struct sender_sending_data * p_sending;
                        p_sending = (struct sender_sending_data*)g_state_data[g_state]; 
                        p_mcastjoined = (struct sender_mcastjoined_data*)g_state_data[SENDER_MCAST_JOINED];
                        assert(NULL != p_sending);
                        assert(NULL != p_mcastjoined);
                        stop_sending(p_sending);
                        g_state = SENDER_MCAST_JOINED;
                    }
                    else
                    {
                        debug_outputln("%s %5.5d", __FILE__, __LINE__);
                    }
                    UpdateUI(hDlg);
                    break;
                case IDOK:
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

int PASCAL WinMain(  HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
{
    INT_PTR dialog_result;
    HRESULT hr;
    WSADATA             wsd;
    int	rc;

    /* Init Winsock */
    if ((rc = WSAStartup(MAKEWORD(1, 1), &wsd)) != 0)
        return FALSE;

    /* Init COM */
    hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        return FALSE;

    g_hInst = hInstance;
    //required to use the common controls
    InitCommonControls();
    /* Perform initializations that apply to a specific instance */
    dialog_result = DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN_SENDER), NULL, SenderDlgProc);
    return (int)dialog_result;
}

