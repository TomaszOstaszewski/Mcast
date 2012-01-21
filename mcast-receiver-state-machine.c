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
#include "receiver-settings.h"
#include "mcast_setup.h"
#include "debug_helpers.h"
#include "dsoundplay.h"
#include "fifo-circular-buffer.h"
#include "wave_utils.h"

/*!
 * @brief The multicast receiver object.
 * @details This structure represents the multicast receiver object.
 */
struct mcast_receiver { 
    receiver_state_t state_; /*!< Receiver's current state. */
    DSOUNDPLAY player_; /*!< Pointer to the data player buffer */
    struct mcast_connection * conn_; /*!< Pointer to the multicast connection object */
    struct fifo_circular_buffer * fifo_; /*!< Pointer to the jitter buffer */
    HANDLE hStopEvent_;/*!< The receiver's stop event. When this event is signalled via SetEvent() call, the receiver thread exits. */
    HANDLE hStopEventThread_;/*!< The receiver's stop event. When this event is signalled via SetEvent() call, the receiver thread exits. */
    HANDLE hRcvThread_; /*!< Handle to the receiver's thread */
    struct receiver_settings settings_; /*!< Receiver settings for multicast connection. */
};

#define MAX_UDP_PACKET_LENGHT (2048)

/**
 * @brief Entry point of the Multicast receiver thread 
 * @details The PCM data is being received from the multicast group via this thread. The thread
 * received the data from multicast connected socket and feeds it to the jitter buffer (fifo queue).
 *
 */
static DWORD WINAPI ReceiverThreadProc(LPVOID param)
{
    uint32_t count;
    struct mcast_receiver * p_receiver;
    fd_set read_selectors;
    struct data_item item;
    DWORD dwWaitTimeout;
    int bytes_recevied;
    DWORD dwWaitResult;
    socklen_t sock_addr_size;
    p_receiver = (struct mcast_receiver*)param;
    assert(p_receiver);
    assert(p_receiver->conn_);
    assert(p_receiver->fifo_);
    sock_addr_size  = sizeof(struct sockaddr_in);
    item.data_      = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_UDP_PACKET_LENGHT);
    item.count_     = MAX_UDP_PACKET_LENGHT;
    dwWaitTimeout   = p_receiver->settings_.poll_sleep_time_;
    for (count = 0; ; ++count)
    {
        FD_ZERO(&read_selectors);
        dwWaitResult = WaitForSingleObject(p_receiver->hStopEventThread_, 0);
        if (WAIT_TIMEOUT == dwWaitResult) /* Timedout - hence, nobody wants us to finish yet */
        {
            do { 
                if (mcast_is_new_data(p_receiver->conn_, dwWaitTimeout))
                {
                    /* Receive data from the socket until you receiving this WSAMSGSIZE error
                     * It is a non-blocking socket, so this call may well yield WSAEWOULDBLOCK - indicating that there
                     * is nothing to receive. We ignore those errors.
                     */
                    bytes_recevied = mcast_recvfrom(p_receiver->conn_, item.data_, item.count_);
                    if (SOCKET_ERROR != bytes_recevied)
                    {
                        item.count_ = bytes_recevied;
                        fifo_circular_buffer_push_item(p_receiver->fifo_, &item);
                        break;
                    }
                    item.data_ = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, item.data_, item.count_ + MAX_UDP_PACKET_LENGHT);
                    item.count_ += DATA_ITEM_SIZE;
                }
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
    CloseHandle(p_receiver->hStopEventThread_);
    p_receiver->hStopEventThread_ = NULL;
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
    int result = 0;
    assert(p_receiver);
    assert(NULL == p_receiver->hStopEvent_);
    assert(NULL == p_receiver->hRcvThread_);
    if (p_receiver && NULL == p_receiver->hStopEvent_ && NULL == p_receiver->hRcvThread_)
    {
        p_receiver->hStopEvent_ = CreateEvent(NULL, TRUE, FALSE, "recv-event-0");
        if (NULL != p_receiver->hStopEvent_ && DuplicateHandle(GetCurrentProcess(), p_receiver->hStopEvent_, GetCurrentProcess(), &p_receiver->hStopEventThread_, 0, FALSE, DUPLICATE_SAME_ACCESS))
        {
            ResetEvent(p_receiver->hStopEvent_);
            p_receiver->hRcvThread_ = CreateThread(NULL, 0, ReceiverThreadProc, p_receiver, 0, NULL);
            assert(NULL != p_receiver->hRcvThread_);
            if (NULL != p_receiver->hRcvThread_);
            {
                result = 1;
            }
        }   
    }
    if (!result)
    {
        if (NULL != p_receiver->hStopEvent_)
            CloseHandle(p_receiver->hStopEvent_);
        p_receiver->hStopEvent_ = NULL;
        if (NULL != p_receiver->hStopEventThread_)
            CloseHandle(p_receiver->hStopEventThread_);
        p_receiver->hStopEventThread_ = NULL;
    }
    return result;
}

/**
 * @brief Implements the multicast data retrieval worker thread graceful termination.
 * @details Signals a worker thread, created via call to handle_rcvstop_internal, that it shall complete whatever it is doing and exit gracefully.
 * @return returns 0 if the data retrieval process has been successfully terminated, otherwise returns <>0. Most likely reason of failure is that the receiver object does not have the data retrieval worker thread has not been created.
 * @sa handle_rcvstart_internal
 */
static int handle_rcvstop_internal(struct mcast_receiver * p_receiver)
{
    int result = 0;
    assert(NULL != p_receiver->hStopEvent_);
    assert(NULL != p_receiver->hRcvThread_);
    if (NULL != p_receiver->hStopEvent_)
    {
        DWORD dwWaitResult;
        SetEvent(p_receiver->hStopEvent_);
        CloseHandle(p_receiver->hStopEvent_);
        p_receiver->hStopEvent_ = NULL;
        dwWaitResult = WaitForSingleObject(p_receiver->hRcvThread_, INFINITE); 
        assert(WAIT_OBJECT_0 == dwWaitResult);
        CloseHandle(p_receiver->hRcvThread_);
        p_receiver->hRcvThread_ = NULL;
        result = 1;
    }
    return result;
}

/**
 * @brief Implements joining a multicast group
 * @return returns 0 if a multicast group has been successfully joined, <>0 otherwise.
 * @sa handle_mcastleave_internal
 */
static int handle_mcastjoin_internal(struct mcast_receiver * p_receiver)
{
    int result = 0;
    int mcast_result = 0;
    assert(p_receiver->conn_ == NULL);
    p_receiver->conn_ = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct mcast_connection));
    assert(NULL != p_receiver->conn_);
    if (NULL != p_receiver->conn_)
    {
        mcast_result = setup_multicast_addr(FALSE, TRUE, NULL, NULL, 8, &p_receiver->settings_.mcast_settings_.mcast_addr_, p_receiver->conn_);
        assert(mcast_result);
        if (mcast_result)
        {
            /* Set the non-blocking mode on the socket */
            unsigned long non_block = 1;
            int ioctl_result = ioctlsocket(p_receiver->conn_->socket_, FIONBIO, &non_block);
            assert(0 == ioctl_result);
            if (0 == ioctl_result)
                result = 1;
        }
    }
    if (!result)
    {
        if (p_receiver->conn_ && mcast_result)
            close_multicast(p_receiver->conn_); 
        if (p_receiver->conn_)
            HeapFree(GetProcessHeap(), 0, p_receiver->conn_);
        p_receiver->conn_ = NULL;
    }
    return result;
}

/**
 * @brief Implements leaving a multicast group
 * @return returns 0 if a multicast group has been successfully left, <>0 otherwise. Most likely reason of failure is lack of multicast group membership to begin with, i.e. handle_mcastjoin_internal() has not been called.
 * @sa handle_mcastjoin_internal
 */
static int handle_mcastleave_internal(struct mcast_receiver * p_receiver)
{
    int result;
    assert(NULL != p_receiver->conn_);
    if (NULL != p_receiver->conn_)
    {
        result = close_multicast(p_receiver->conn_);
        HeapFree(GetProcessHeap(), 0, p_receiver->conn_);
        p_receiver->conn_ = NULL;
    }
    return result;
}

/**
 * @brief Stops retrieved data playback.
 * @return returns non-zero if a player has stopped playing data, 0 when failure to stop player occures. Most likely reason of failure is the fact that there is no data playback to begin with, i.e. handle_play_internal() has not been called.
 * @sa handle_play_internal
 */
static int handle_stop_internal(struct mcast_receiver * p_receiver)
{
    int result = 0;
    assert(NULL != p_receiver->player_);
    if (NULL != p_receiver->player_)
    {
        result = dsoundplayer_stop(p_receiver->player_);    
        dsoundplayer_destroy(p_receiver->player_);
        p_receiver->player_ = NULL;
    }
    return result;
}

/**
 * @brief Starts retrieved data playback.
 * @return returns 1 if a player has started playing data, 0 otherwise.
 * @sa handle_play_internal
 */
static int handle_play_internal(HWND hMainWnd, struct mcast_receiver * p_receiver)
{
    int result = 0;
    assert(NULL == p_receiver->player_);
    assert(NULL != p_receiver->fifo_);
    if (NULL == p_receiver->player_ && NULL != p_receiver->fifo_)
    {
        p_receiver->player_ = dsoundplayer_create(hMainWnd, &p_receiver->settings_.wfex_, p_receiver->fifo_, &p_receiver->settings_.play_settings_);
        assert(NULL != p_receiver->player_);
        if (NULL != p_receiver->player_)
        {
            result = dsoundplayer_play(p_receiver->player_);    
            assert(result);
            return result;
        }
    }
    return result;
}

struct mcast_receiver * receiver_create(struct receiver_settings const * p_settings)
{
    struct mcast_receiver * p_receiver = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct mcast_receiver)); 
    assert(p_receiver);
    assert(p_settings);
    receiver_settings_copy(&p_receiver->settings_, p_settings);
    p_receiver->fifo_ = fifo_circular_buffer_create();
    return p_receiver;
}

int receiver_destroy(struct mcast_receiver * p_receiver)
{
    assert(RECEIVER_INITIAL == p_receiver->state_);
    if (RECEIVER_INITIAL == p_receiver->state_);
    {
        HeapFree(GetProcessHeap(), 0, p_receiver);
        return 1;
    }
    return 0;
}

void handle_play(struct mcast_receiver * p_receiver, HWND hMainWnd)
{
    if (RECEIVER_MCASTJOINED == p_receiver->state_)
    {
        if (handle_play_internal(hMainWnd, p_receiver))
            p_receiver->state_ = RECEIVER_MCASTJOINED_PLAYING;
    }
    else if (RECEIVER_INITIAL == p_receiver->state_)
    {
        if (handle_play_internal(hMainWnd, p_receiver))
            p_receiver->state_ = RECEIVER_PLAYING;
    }
    else if (RECEIVER_RECEIVING == p_receiver->state_)
    {
        if (handle_play_internal(hMainWnd, p_receiver))
            p_receiver->state_ = RECEIVER_RECEIVING_PLAYING;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

void handle_stop(struct mcast_receiver * p_receiver)
{
    if (RECEIVER_PLAYING == p_receiver->state_)
    {
        if  (handle_stop_internal(p_receiver))
            p_receiver->state_ = RECEIVER_INITIAL;
    }
    else if (RECEIVER_RECEIVING_PLAYING == p_receiver->state_)
    {
        if  (handle_stop_internal(p_receiver))
            p_receiver->state_ = RECEIVER_RECEIVING;
    }
    else if (RECEIVER_MCASTJOINED_PLAYING == p_receiver->state_)
    {
        if  (handle_stop_internal(p_receiver))
            p_receiver->state_ = RECEIVER_MCASTJOINED;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

void handle_rcvstart(struct mcast_receiver * p_receiver)
{
    if (RECEIVER_MCASTJOINED == p_receiver->state_)
    {
        if (handle_rcvstart_internal(p_receiver))
            p_receiver->state_ = RECEIVER_RECEIVING;
    } 
    else if (RECEIVER_MCASTJOINED_PLAYING == p_receiver->state_)
    {
        if (handle_rcvstart_internal(p_receiver))
            p_receiver->state_ = RECEIVER_RECEIVING_PLAYING;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

void handle_rcvstop(struct mcast_receiver * p_receiver)
{
    if (RECEIVER_RECEIVING_PLAYING == p_receiver->state_)
    {
        if (handle_rcvstop_internal(p_receiver))
            p_receiver->state_ = RECEIVER_MCASTJOINED_PLAYING;
    }
    else if (RECEIVER_RECEIVING == p_receiver->state_)
    {
        if (handle_rcvstop_internal(p_receiver))
            p_receiver->state_ = RECEIVER_MCASTJOINED;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

void handle_mcastjoin(struct mcast_receiver * p_receiver)
{
    debug_outputln("%s %u", __FILE__, __LINE__);
    if (RECEIVER_INITIAL == p_receiver->state_)
    {
        if (handle_mcastjoin_internal(p_receiver))
            p_receiver->state_ = RECEIVER_MCASTJOINED;
    }
    else if (RECEIVER_PLAYING == p_receiver->state_)
    {
        if (handle_mcastjoin_internal(p_receiver))
            p_receiver->state_ = RECEIVER_MCASTJOINED_PLAYING;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

void handle_mcastleave(struct mcast_receiver * p_receiver)
{
    if (RECEIVER_MCASTJOINED == p_receiver->state_)
    {
        if (handle_mcastleave_internal(p_receiver))
            p_receiver->state_ = RECEIVER_INITIAL; 
    }
    else if (RECEIVER_MCASTJOINED_PLAYING == p_receiver->state_)
    {
        if (handle_mcastleave_internal(p_receiver))
            p_receiver->state_ = RECEIVER_PLAYING;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

receiver_state_t receiver_get_state(struct mcast_receiver * p_receiver)
{
    return p_receiver->state_;
}

struct fifo_circular_buffer * receiver_get_fifo(struct mcast_receiver * p_receiver)
{
    assert(p_receiver);
    assert(p_receiver->fifo_);
    return p_receiver->fifo_;
}

