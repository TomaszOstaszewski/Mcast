/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file mcast-sender-state-machine.c
 * @author T. Ostaszewski 
 */ 
#include "pcc.h"
#include "mcast-sender-state-machine.h"
#include "mcastui.h"
#include "conn_data.h"
#include "mcast_setup.h"
#include "resource.h"
#include "debug_helpers.h"
#include "wave_utils.h"
#include "winsock_adapter.h"
#include "mcast-sender-state-machine.h"

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

/*!
 * 
 */
#define DEFAULT_CHUNK_SEND_TIMEOUT (85)

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

/*!
 * @brief 
 */
static sender_state_t g_state;

extern master_riff_chunk_t * g_pWavChunk;

/*!
 * @brief
 */
uint8_t const * g_p_current_pos_;

/*!
 * @brief
 */
DWORD g_dw_chunk_send_timeout;

/*!
 * @brief
 */
HANDLE g_hStopEvent;

/*!
 * @brief
 */
struct mcast_connection * g_conn;

/*!
 * @brief
 */
static DWORD WINAPI SendThreadProc(LPVOID param)
{
    uint32_t max_offset;
    struct master_riff_chunk * p_master_riff;
    int8_t const * p_data_begin;
    HANDLE h_stop_event;
    DWORD dwResult;
    h_stop_event = (HANDLE)param;
    p_master_riff = g_pWavChunk;
    p_data_begin = &p_master_riff->format_chunk_.subchunk_.samples8_[0];
    max_offset = p_master_riff->format_chunk_.subchunk_.subchunk_size_;
    for (;;)
    {
        dwResult = WaitForSingleObject(h_stop_event, DEFAULT_CHUNK_SEND_TIMEOUT);
        if (WAIT_TIMEOUT == dwResult)
        {
            int result;
            uint32_t offset = g_p_current_pos_ - p_data_begin;
            uint16_t chunk_size = DEFAULT_WAV_CHUNK_SIZE;
            if (offset + chunk_size > max_offset)
            {
                chunk_size = max_offset - offset;
            }
            result = sendto(g_conn->socket_, 
                    (const char *)g_p_current_pos_, 
                    chunk_size,
                    0,
                    g_conn->multiAddr_->ai_addr,
                    (int) g_conn->multiAddr_->ai_addrlen
                    );
            g_p_current_pos_+= chunk_size;
            if ((uint32_t)(g_p_current_pos_ - p_data_begin) + chunk_size >= max_offset)
            {
                g_p_current_pos_ = p_data_begin;
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
 * @param
 */
static int sender_handle_mcastjoin_internal(void)
{
    assert(NULL == g_conn);
    g_conn = HeapAlloc(GetProcessHeap(), 0, sizeof(struct mcast_connection));
    assert(NULL != g_conn);
    if (NULL != g_conn)
    {
        int rc;
        rc = setup_multicast_3(DEFAULT_MCASTADDRV4, DEFAULT_MCASTPORT, g_conn);
        if (0 == rc)
        {
            return 0;
        }
        HeapFree(GetProcessHeap(), 0, g_conn);
        g_conn = NULL;
    }
    return (-1);
}

/**
 * @brief
 * @param
 */
static int sender_handle_mcastleave_internal(void)
{
    int result;
    result = close_multicast(g_conn);
    HeapFree(GetProcessHeap(), 0, g_conn);
    g_conn = NULL;
    return 0;
}

/**
 * @brief
 * @param
 */
static int sender_handle_startsending_internal(void)
{
    HANDLE h_stop_event;
    assert(NULL == g_hStopEvent);
    h_stop_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL != h_stop_event)
    {
        BOOL bDupResult; 
        bDupResult = DuplicateHandle(GetCurrentProcess(), h_stop_event, GetCurrentProcess(), &g_hStopEvent, 0, FALSE, DUPLICATE_SAME_ACCESS);
        if (bDupResult)
        {
            HANDLE hSenderThread;
            hSenderThread = CreateThread(NULL, 0, SendThreadProc, h_stop_event, 0, NULL);
            CloseHandle(hSenderThread);
            return 0;
        }
        CloseHandle(h_stop_event);
    }
    return -1;
}

/**
 * @brief
 * @param
 */
static int sender_handle_stopsending_internal(void)
{
    SetEvent(g_hStopEvent);   
    CloseHandle(g_hStopEvent);
    g_hStopEvent = NULL;
    return 0;
}

sender_state_t sender_get_current_state(void)
{
	return g_state;
}

void sender_initialize(void)
{
}

void sender_handle_mcastjoin(void)
{
	if (SENDER_INITIAL == g_state)
	{
        sender_handle_mcastjoin_internal();
        g_state = SENDER_MCAST_JOINED;
	}
	else
	{
		debug_outputln("%s %5.5d", __FILE__, __LINE__);
	}
}

void sender_handle_mcastleave(void)
{
	if (SENDER_MCAST_JOINED == g_state)
	{
        sender_handle_mcastleave_internal();
		g_state = SENDER_INITIAL;
	}
	else
	{
		debug_outputln("%s %5.5d", __FILE__, __LINE__);
	}
}

void sender_handle_startsending(void)
{
	if (SENDER_MCAST_JOINED == g_state)
	{
        sender_handle_startsending_internal();
    	g_state = SENDER_SENDING;
	}
	else
	{
		debug_outputln("%s %5.5d", __FILE__, __LINE__);
	}
}

void sender_handle_stopsending(void)
{
	if (SENDER_SENDING == g_state)
	{
        sender_handle_stopsending_internal();
		g_state = SENDER_MCAST_JOINED;
	}
	else
	{
		debug_outputln("%s %5.5d", __FILE__, __LINE__);
	}
}

