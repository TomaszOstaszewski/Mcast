/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file mcast-receiver-state-machine.c
 * @brief Multicast receiver state machine.
 * @details A reciever operats using a state machine. This state machine 
 * is quite simple and rudimentary, but nevertheless it gives a fairly good
 * reliability and readibility. Instead of tons of if...else on the various
 * variables, there's just one check for state and then, if the test yields ok,
 * an action is perfomed.
 * @author T. Ostaszewski
 * @par License
 * @code Copyright 2012 Tomasz Ostaszewski. All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * 	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation 
 * 	and/or other materials provided with the distribution.
  * THIS SOFTWARE IS PROVIDED BY Tomasz Ostaszewski AS IS AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL Tomasz Ostaszewski OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE.
  * The views and conclusions contained in the software and documentation are those of the 
 * authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of Tomasz Ostaszewski.
 * @endcode 
 * @date 04-Jan-2012
 */ 
#include "pcc.h"
#include "mcast-receiver-state-machine.h"
#include "mcast_setup.h"
#include "debug_helpers.h"
#include "dsoundplay.h"
#include "fifo-circular-buffer.h"
#include "wave_utils.h"

/**
 * @brief IPv4 multicast group address.
 */
#define DEFAULT_MCASTADDRV4    "234.5.6.7"

/**
 * @brief Default port on which multicast communication is performed.
 */
#define DEFAULT_MCASTPORT      "25000"

/**
 * @brief IPv6 multicast group address.
 */
#define DEFAULT_MCASTADDRV6    "ff12::1"

static receiver_state_t g_state; /*!< Receiver's current state. */

static struct mcast_connection * g_conn; /*!< Pointer to the multicast connection object */

static struct fifo_circular_buffer * g_fifo; /*!< Pointer to the jitter buffer */

static DSOUNDPLAY g_player; /*!< Pointer to the data player buffer */

static WAVEFORMATEX * g_wfex; /*!< Pointer to the object describing the PCM data being received */

static HANDLE g_hStopEvent; /*!< The receiver's stop event. When this event is signalled via SetEvent() call, the receiver thread exits. */

struct mcast_receiver { 
    receiver_state_t g_state; /*!< Receiver's current state. */
    DSOUNDPLAY player_; /*!< Pointer to the data player buffer */
    WAVEFORMATEX * wfex_; /*!< Pointer to the object describing the PCM data being received */
    struct mcast_connection * conn_; /*!< Pointer to the multicast connection object */
    struct fifo_circular_buffer * fifo_; /*!< Pointer to the jitter buffer */
    HANDLE hStopEvent_;/*!< The receiver's stop event. When this event is signalled via SetEvent() call, the receiver thread exits. */
    HANDLE hStopEventThread_;/*!< The receiver's stop event. When this event is signalled via SetEvent() call, the receiver thread exits. */
};

struct mcast_receiver * p_receiver = NULL;
 
/**
 * @brief Entry point of the Multicast receiver thread 
 * @details The PCM data is being received from the multicast group via this thread. The thread
 * received the data from multicast connected socket and feeds it to the jitter buffer (fifo queue).
 *
 */
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
        else if (WAIT_OBJECT_0 == dwWaitResult) /* Signaled - hence, the external environment has just requested termination */
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

/**
 * @brief Implements the multicast data retrieval worker thread startup.
 * @details Creates a worker thread which receives data from the multicast socket and feeds it to the jitter buffer.
 * @return returns 0 if the data retrieval process has been successfully started, otherwise returns <>0.
 */
static int handle_rcvstart_internal(struct mcast_receiver * p_receiver)
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

/**
 * @brief Implements the multicast data retrieval worker thread graceful termination.
 * @details Signals a worker thread, created via call to handle_rcvstop_internal, that it shall complete whatever it is doing and exit gracefully.
 * @return returns 0 if the data retrieval process has been successfully terminated, otherwise returns <>0. Most likely reason of failure is that the receiver object does not have the data retrieval worker thread has not been created.
 * @sa handle_rcvstart_internal
 */
static int handle_rcvstop_internal(struct mcast_receiver * p_receiver)
{
    assert (NULL != g_hStopEvent);
    if (NULL != g_hStopEvent)
    {
        SetEvent(g_hStopEvent);
        CloseHandle(g_hStopEvent);
        g_hStopEvent = NULL;
        return 0;
    }
    return -1;
}

/**
 * @brief Implements joining a multicast group
 * @return returns 0 if a multicast group has been successfully joined, <>0 otherwise.
 * @sa handle_mcastleave_internal
 */
static int handle_mcastjoin_internal(struct mcast_receiver * p_receiver)
{
	int result;
	assert(g_conn == NULL);
	g_conn = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct mcast_connection));
	assert(NULL != g_conn);
    if (NULL != g_conn)
    {
        result = setup_multicast_default(DEFAULT_MCASTADDRV4, DEFAULT_MCASTPORT, g_conn);
        assert(0 == result);
        if (0 == result)
        {
            return 0;
        }
        HeapFree(GetProcessHeap(), 0, g_conn);
        g_conn = NULL;
    }
    return -1;
}

/**
 * @brief Implements leaving a multicast group
 * @return returns 0 if a multicast group has been successfully left, <>0 otherwise. Most likely reason of failure is lack of multicast group membership to begin with, i.e. handle_mcastjoin_internal() has not been called.
 * @sa handle_mcastjoin_internal
 */
static int handle_mcastleave_internal(struct mcast_receiver * p_receiver)
{
    assert(NULL != g_conn);
    if (NULL != g_conn)
    {
        close_multicast(g_conn);
        HeapFree(GetProcessHeap(), 0, g_conn);
        g_conn = NULL;
        return 0;
    }
    return -1;
}

/**
 * @brief Stops retrieved data playback.
 * @return returns 0 if a player has stopped playing data, <>0 otherwise. Most likely reason of failure is the fact that there is no data playback to begin with, i.e. handle_play_internal() has not been called.
 * @sa handle_play_internal
 */
static int handle_stop_internal(struct mcast_receiver * p_receiver)
{
    assert(NULL != g_player);
    if (NULL != g_player)
    {
        dsoundplayer_stop(g_player);    
        dsoundplayer_destroy(g_player);
        g_player = NULL;
        return 0;
    }
    return -1;
}

/**
 * @brief Starts retrieved data playback.
 * @return returns 0 if a player has started playing data, <>0 otherwise.
 * @sa handle_play_internal
 */
static int handle_play_internal(HWND hMainWnd, struct mcast_receiver * p_receiver)
{
    assert(NULL == g_player);
    assert(NULL != g_wfex);
    assert(NULL != g_fifo);
    if (NULL == g_player && NULL != g_wfex && NULL != g_fifo)
    {
        g_player = dsoundplayer_create(hMainWnd, g_wfex, g_fifo);
        assert(NULL != g_player);
        if (NULL != g_player)
        {
            dsoundplayer_play(g_player);    
            return 0;
        }
    }
    return -1;
}

void receiver_init(WAVEFORMATEX * p_wfex)
{
	g_wfex = p_wfex;
	g_fifo = fifo_circular_buffer_create();
}

void handle_play(HWND hMainWnd)
{
    if (RECEIVER_MCASTJOINED == g_state)
    {
        if (0 ==handle_play_internal(hMainWnd, p_receiver))
            g_state = RECEIVER_MCASTJOINED_PLAYING;
    }
    else if (RECEIVER_INITIAL == g_state)
    {
        if (0 == handle_play_internal(hMainWnd, p_receiver))
            g_state = RECEIVER_PLAYING;
    }
    else if (RECEIVER_RECEIVING == g_state)
    {
        if (0 == handle_play_internal(hMainWnd, p_receiver))
            g_state = RECEIVER_RECEIVING_PLAYING;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

void handle_stop(void)
{
    if (RECEIVER_PLAYING == g_state)
    {
        if  (0 == handle_stop_internal(p_receiver))
            g_state = RECEIVER_INITIAL;
    }
    else if (RECEIVER_RECEIVING_PLAYING == g_state)
    {
        if  (0 == handle_stop_internal(p_receiver))
            g_state = RECEIVER_RECEIVING;
    }
    else if (RECEIVER_MCASTJOINED_PLAYING == g_state)
    {
        if  (0 == handle_stop_internal(p_receiver))
            g_state = RECEIVER_MCASTJOINED;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

void handle_rcvstart(void)
{
    if (RECEIVER_MCASTJOINED == g_state)
    {
        if (0 == handle_rcvstart_internal(p_receiver))
            g_state = RECEIVER_RECEIVING;
    } 
    else if (RECEIVER_MCASTJOINED_PLAYING == g_state)
    {
        if (0 == handle_rcvstart_internal(p_receiver))
            g_state = RECEIVER_RECEIVING_PLAYING;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

void handle_rcvstop(void)
{
    if (RECEIVER_RECEIVING_PLAYING == g_state)
    {
        if (0 == handle_rcvstop_internal(p_receiver))
            g_state = RECEIVER_MCASTJOINED_PLAYING;
    }
    else if (RECEIVER_RECEIVING == g_state)
    {
        if (0 == handle_rcvstop_internal(p_receiver))
            g_state = RECEIVER_MCASTJOINED;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

void handle_mcastjoin(void)
{
    if (RECEIVER_INITIAL == g_state)
    {
        if (0 == handle_mcastjoin_internal(p_receiver))
            g_state = RECEIVER_MCASTJOINED;
    }
    else if (RECEIVER_PLAYING == g_state)
    {
        if (0 == handle_mcastjoin_internal(p_receiver))
            g_state = RECEIVER_MCASTJOINED_PLAYING;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

void handle_mcastleave(void)
{
    if (RECEIVER_MCASTJOINED == g_state)
    {
        if (0 == handle_mcastleave_internal(p_receiver))
            g_state = RECEIVER_INITIAL; 
    }
    else if (RECEIVER_MCASTJOINED_PLAYING == g_state)
    {
        if (0 == handle_mcastleave_internal(p_receiver))
            g_state = RECEIVER_PLAYING;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

receiver_state_t receiver_get_state(void)
{
    return g_state;
}

