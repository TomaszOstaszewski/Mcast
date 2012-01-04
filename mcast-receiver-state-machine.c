/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file mcast-receiver-state-machine.c
 * @author T. Ostaszewski 
 */ 
#include "pcc.h"
#include "mcast-receiver-state-machine.h"
#include "mcast_setup.h"
#include "debug_helpers.h"
#include "dsoundplay.h"
#include "fifo-circular-buffer.h"
#include "wave_utils.h"

/**
 * @brief
 */
#define DEFAULT_MCASTADDRV4    "234.5.6.7"

/**
 * @brief
 */
#define DEFAULT_MCASTPORT      "25000"

/**
 * @brief
 */
#define DEFAULT_MCASTADDRV6    "ff12::1"

/**
 * @brief
 */
static receiver_state_t g_state;

/**
 * @brief
 */
static struct mcast_connection * g_conn;

/**
 * @brief
 */
static struct fifo_circular_buffer * g_fifo;

/**
 * @brief
 */
static DSOUNDPLAY g_player;

/**
 * @brief
 */
static WAVEFORMATEX * g_wfex;

/**
 * @brief
 */
static HANDLE g_hStopEvent;
 
static DWORD WINAPI ReceiverThreadProc(LPVOID param)
{
    uint16_t count;
    struct data_item item;
    struct sockaddr_in sock_addr;
    HANDLE h_stop_event;
    socklen_t sock_addr_size;
    sock_addr_size  = sizeof(struct sockaddr_in);
    item.data_      = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DATA_ITEM_SIZE);
    item.count_     = DATA_ITEM_SIZE;
    assert(NULL != g_conn);
    assert(NULL != g_fifo);
	h_stop_event = (HANDLE)param;
    for (count = 0; ; ++count)
    {
        int bytes_recevied;
        DWORD dwWaitResult;
        dwWaitResult = WaitForSingleObject(h_stop_event, 50);
        if (WAIT_TIMEOUT == dwWaitResult) /* Timedout - hence, nobody wants us to finish yet */
        {
            /* Receive data from the socket until you receiving this WSAMSGSIZE error */
            do { 
                bytes_recevied = recvfrom(
                        g_conn->socket_,
                        item.data_, 
                        item.count_, 
                        0,
                        (struct sockaddr*)&sock_addr,
                        &sock_addr_size
                        );
                if (SOCKET_ERROR != bytes_recevied)
                {
                    item.count_ = bytes_recevied;
                    fifo_circular_buffer_push_item(g_fifo, &item);
                    break;
                }
                item.data_ = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, item.data_, item.count_ + DATA_ITEM_SIZE);
                item.count_ += DATA_ITEM_SIZE;
            } while (WSAGetLastError() == WSAEMSGSIZE);
        }
        else if (WAIT_OBJECT_0 == dwWaitResult) /* Signaled - hence, the external enviroment has just requested termination */
        {
            break;
        }
        else
        {
            debug_outputln("%s %d : %8.8x", __FILE__, __LINE__, dwWaitResult);
            break;
        }
    }
	CloseHandle(h_stop_event);
	HeapFree(GetProcessHeap(), 0, item.data_);
    return 0;
}

static int handle_rcvstart_internal(void)
{
    HANDLE h_rcv_thread;
	HANDLE h_stop_event;
	BOOL bDupResult;

	assert(NULL == g_hStopEvent);
	h_stop_event = CreateEvent(NULL, TRUE, FALSE, "recv-event-0");
	if (NULL == h_stop_event)
	{
		return -1;
	}
    bDupResult = DuplicateHandle(GetCurrentProcess(), h_stop_event, GetCurrentProcess(), &g_hStopEvent, 0, FALSE, DUPLICATE_SAME_ACCESS);
	if (bDupResult)
	{
		ResetEvent(g_hStopEvent);
		/* Pass event copy to the receiver thread */
		h_rcv_thread = CreateThread(NULL, 0, ReceiverThreadProc, h_stop_event, 0, NULL);
		assert(NULL != h_rcv_thread);
		CloseHandle(h_rcv_thread);
		return 0;
	}
    return -1;
}

static int handle_rcvstop_internal(void)
{
    assert (NULL != g_hStopEvent);
    SetEvent(g_hStopEvent);
	CloseHandle(g_hStopEvent);
	g_hStopEvent = NULL;
    return 0;
}

static int handle_mcastjoin_internal(void)
{
	int result;
	assert(g_conn == NULL);
	g_conn = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct mcast_connection));
	assert(NULL != g_conn);
	result = setup_multicast_3(DEFAULT_MCASTADDRV4, DEFAULT_MCASTPORT, g_conn);
	assert(0 == result);
	if (0 != result)
	{
		HeapFree(GetProcessHeap(), 0, g_conn);
		g_conn = NULL;
	}
	return result;
}

static int handle_mcastleave_internal(void)
{
	assert(NULL != g_conn);
	close_multicast(g_conn);
	HeapFree(GetProcessHeap(), 0, g_conn);
	g_conn = NULL;
    return 0;
}

static int handle_stop_internal(void)
{
    assert(NULL != g_player);
	dsoundplayer_stop(g_player);    
	dsoundplayer_destroy(g_player);
	g_player = NULL;
	return 0;
}

static int handle_play_internal(HWND hMainWnd)
{
	assert(NULL == g_player);
	assert(NULL != g_wfex);
	assert(NULL != g_fifo);
	g_player = dsoundplayer_create(hMainWnd, g_wfex, g_fifo);
	assert(NULL != g_player);
	dsoundplayer_play(g_player);    
	return 0;
}

void receiver_init(WAVEFORMATEX * p_wfex)
{
	g_wfex = p_wfex;
	g_fifo = fifo_circular_buffer_create();
}

/*!
 * @brief
 */
void handle_play(HWND hMainWnd)
{
    if (RECEIVER_MCASTJOINED == g_state)
    {
        handle_play_internal(hMainWnd);
        g_state = RECEIVER_MCASTJOINED_PLAYING;
    }
    else if (RECEIVER_INITIAL == g_state)
    {
        handle_play_internal(hMainWnd);
        g_state = RECEIVER_PLAYING;
    }
    else if (RECEIVER_RECEIVING == g_state)
    {
        handle_play_internal(hMainWnd);
        g_state = RECEIVER_RECEIVING_PLAYING;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

/*!
 * @brief
 */
void handle_stop(void)
{
    if (RECEIVER_PLAYING == g_state)
    {
        if  (0 == handle_stop_internal())
        {
            debug_outputln("%s %d", __FILE__, __LINE__);
            g_state = RECEIVER_INITIAL;
        }
    }
    else if (RECEIVER_RECEIVING_PLAYING == g_state)
    {
        if  (0 == handle_stop_internal())
        {
            debug_outputln("%s %d", __FILE__, __LINE__);
            g_state = RECEIVER_RECEIVING;
        }
    }
    else if (RECEIVER_MCASTJOINED_PLAYING == g_state)
    {
        if  (0 == handle_stop_internal())
        {
            debug_outputln("%s %d", __FILE__, __LINE__);
            g_state = RECEIVER_MCASTJOINED;
        }
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}
 
/*!
 * @brief
 */
void handle_rcvstart(void)
{
    if (RECEIVER_MCASTJOINED == g_state)
    {
        handle_rcvstart_internal();
        g_state = RECEIVER_RECEIVING;
    } 
    else if (RECEIVER_MCASTJOINED_PLAYING == g_state)
    {
        handle_rcvstart_internal();
        g_state = RECEIVER_RECEIVING_PLAYING;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

/*!
 * @brief
 */
void handle_rcvstop(void)
{
    if (RECEIVER_RECEIVING_PLAYING == g_state)
    {
        handle_rcvstop_internal();
        g_state = RECEIVER_MCASTJOINED_PLAYING;
    }
    else if (RECEIVER_RECEIVING == g_state)
    {
        handle_rcvstop_internal();
        g_state = RECEIVER_MCASTJOINED;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

/*!
 * @brief
 */
void handle_mcastjoin(void)
{
    if (RECEIVER_INITIAL == g_state)
    {
        handle_mcastjoin_internal();
        g_state = RECEIVER_MCASTJOINED;
    }
    else if (RECEIVER_PLAYING == g_state)
    {
        handle_mcastjoin_internal();
        g_state = RECEIVER_MCASTJOINED_PLAYING;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

/*!
 * @brief
 */
void handle_mcastleave(void)
{
    if (RECEIVER_MCASTJOINED == g_state)
    {
        handle_mcastleave_internal();
        g_state = RECEIVER_INITIAL; 
    }
    else if (RECEIVER_MCASTJOINED_PLAYING == g_state)
    {
        handle_mcastleave_internal();
        g_state = RECEIVER_PLAYING;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

/**
 * @brief
 */
receiver_state_t receiver_get_state(void)
{
    return g_state;
}

