/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file mcast-sender-state-machine.c
 * @brief Multicast sender state machine.
 * @details A sender operates using a state machine. This state machine 
 * is quite simple and rudimentary, but nevertheless it gives a fairly good
 * reliability and readability. Instead of tons of if...else on the various
 * variables, there's just one check for state and then, if the test yields OK,
 * an action is performed.
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
#include "sender-sm-states.h"
#include "mcast-sender-state-machine.h"
#include "mcast_setup.h"
#include "debug_helpers.h"
#include "wave_utils.h"
#include "mcast-sender-state-machine.h"
#include "sender-settings.h"
#include "abstract-tone.h"
#include "circular-buffer-uint8.h"
#include "dsound-recorder.h"
#include "recorder-settings.h"

/*!
 * @brief Maximum number of payload bytes that will fit a single 100BaseT Ethernet packet.
 * @details The macro assumes that the payload also contains the IP and UDP header. What is 
 * left is the user data and that amount of user data this macro represents.
 */
#define MAX_ETHER_PAYLOAD_SANS_UPD_IP (1500-20-8)

/**
 * @brief Description of the multicast sender state machine.
 */
struct mcast_sender { 
    /** @brief Current sender state. */
    sender_state_t state_; 
    /** @brief Pointer to the structure that describes multicast connection */
    struct mcast_connection * conn_;   
    /** @brief Copy of the sender settings. */
    struct sender_settings settings_; 
    /** @brief */
    struct dxaudio_recorder * recorder_;
    /** @brief */
    struct fifo_circular_buffer * p_circular_buffer_;
    /** @brief */
    recorder_settings_t rec_settings_;
    HANDLE hStopEvent_; /*!< Handle to the event used to stop the sending thread. */
    /*!
     * @brief Handle to the event used to stop the sending thread. 
     * @details This is a copy of the handle stored in hStopEvent_ member. To avoid
     * race conditions between the sender thread and the main thread, there are 2 handles 
     * that describe the very same event. Both the main thread and the sender thread operate
     * on their own copy, therefore avoid race conditions between CloseHandle() calls.
     * @sa hStopEvent_
     */
    HANDLE hStopEventThread_; /*!< */
    HANDLE hSenderThread_; /*!< Handle of the sender thread. */
};

/*!
 * @brief Sender thread function.
 * @details Spins a loop in which it checks whether it was signalled to exit. If so, exits.
 * Otherwise, sends out next chunk of data over the multicast connected socket. The data wraps
 * around the beginning if end of data is exhibited.
 * @param[in] param pointer to the mcast_sender structure
 * @return returns 0 if a thread has completed successfully, otherwise <>0.
 */
static DWORD WINAPI SendThreadProc(LPVOID param)
{
    DWORD dwResult;
    static uint8_t data_to_send[1024*sizeof(uint16_t)] = {0};
    struct mcast_sender * p_sender;

    p_sender = (struct mcast_sender *)param;
    assert(p_sender);
    for (;;)
    {
        int result;
        size_t data_retrived_size;
        uint32_t retrieved_data_size = sizeof(data_to_send);
        data_retrived_size = fifo_circular_buffer_fetch_item(p_sender->p_circular_buffer_, data_to_send, &retrieved_data_size);
        if (data_retrived_size > 0)
        {
            result = mcast_sendto(p_sender->conn_, &data_to_send[0], retrieved_data_size);
            if (SOCKET_ERROR == result)
            {
                debug_outputln("%4.4u %s : %p %u", __LINE__, __FILE__, &data_to_send[0], retrieved_data_size);
            }
            debug_outputln("%4.4u %s : %d %u %u", 
                __LINE__, __FILE__, 
                result, data_retrived_size, fifo_circular_buffer_get_items_count(p_sender->p_circular_buffer_));
        }
        dwResult = WaitForSingleObject(p_sender->hStopEventThread_, 0);
        if (WAIT_TIMEOUT == dwResult)
        {
            continue;
        }
        else if (WAIT_OBJECT_0 == dwResult)
        {
            dwResult = 0;
            break;
        }
        else 
        {
            dwResult = -1;
            debug_outputln("%s %4.4u : %d", __FILE__, __LINE__, dwResult);
            break;
        }
    }
    CloseHandle(p_sender->hStopEventThread_);
    p_sender->hStopEventThread_ = NULL;
    return dwResult;
}

/**
 * @brief Joins the multicast group for which sender is configured to join.
 * @param[in] p_sender pointer to the sender description structure.
 * @return returns non-zero on success, 0 otherwise.
 */
static int sender_handle_mcastjoin_internal(struct mcast_sender * p_sender)
{
    int result = 0;
    assert(NULL == p_sender->conn_);
    if (NULL == p_sender->conn_)
    {
        p_sender->conn_ = HeapAlloc(GetProcessHeap(), 0, sizeof(struct mcast_connection));
        assert(NULL != p_sender->conn_);
        if (NULL != p_sender->conn_)
        {
            result = setup_multicast_indirect(&p_sender->settings_.mcast_settings_, p_sender->conn_);
            assert(result);
        }
    }
    if (!result)
    {
        HeapFree(GetProcessHeap(), 0, p_sender->conn_);
        p_sender->conn_ = NULL;
    }
    return result;
}

/**
 * @brief Leaves the multicast group.
 * @param[in] p_sender pointer to the sender description structure.
 * @return returns non-zero on success, 0 otherwise.
 */
static int sender_handle_mcastleave_internal(struct mcast_sender * p_sender)
{
    int result = 0;
    assert(NULL != p_sender->conn_);
    if (NULL != p_sender->conn_);
    {
        result = close_multicast(p_sender->conn_);
        if (result)
        {
            HeapFree(GetProcessHeap(), 0, p_sender->conn_);
            p_sender->conn_ = NULL;
        }
    }
    return result;
}

/**
 * @brief Starts sending data over the multicast connection.
 * @param[in] p_sender pointer to the sender description structure.
 * @return returns non-zero on success, 0 otherwise.
 */
static int sender_handle_startsending_internal(struct mcast_sender * p_sender)
{
    int result = 0;
    assert(NULL == p_sender->hStopEventThread_);
    assert(NULL == p_sender->hStopEvent_);
    if (NULL == p_sender->hStopEventThread_ && NULL == p_sender->hStopEvent_)
    {
        p_sender->hStopEvent_ = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (NULL != p_sender->hStopEvent_)
        {
            if (DuplicateHandle(GetCurrentProcess(), p_sender->hStopEvent_, GetCurrentProcess(), &p_sender->hStopEventThread_, 0, FALSE, DUPLICATE_SAME_ACCESS))
            {
                p_sender->hSenderThread_ = CreateThread(NULL, 0, SendThreadProc, p_sender, 0, NULL);
                assert(NULL != p_sender->hSenderThread_);
                if (NULL != p_sender->hSenderThread_)
                {
                    result = 1;
                }
            }
        }
    }
    if (!result)
    {
        if (NULL != p_sender->hStopEvent_)
        {
            CloseHandle(p_sender->hStopEvent_);
            p_sender->hStopEvent_ = NULL;
        }
        if (NULL != p_sender->hStopEventThread_)
        {
            CloseHandle(p_sender->hStopEventThread_);
            p_sender->hStopEventThread_ = NULL;
        }
    }
    assert(result);
    return result;
}

/**
 * @brief Stops sending data over the multicast connection.
 * @param[in] p_sender pointer to the sender description structure.
 * @return returns non-zero on success, 0 otherwise.
 */
static int sender_handle_stopsending_internal(struct mcast_sender * p_sender)
{
    DWORD dwWaitResult;
    SetEvent(p_sender->hStopEvent_);   
    CloseHandle(p_sender->hStopEvent_);
    dwWaitResult = WaitForSingleObject(p_sender->hSenderThread_, INFINITE); 
    if (WAIT_OBJECT_0 == dwWaitResult)
    {
        CloseHandle(p_sender->hSenderThread_);
    }
    p_sender->hStopEvent_ = NULL;
    p_sender->hSenderThread_ = NULL;
    dxaudio_recorder_stop(p_sender->recorder_);
    dxaudio_recorder_destroy(p_sender->recorder_);
    p_sender->recorder_ = NULL; 
    return 1;
}

int sender_handle_mcastjoin(struct mcast_sender * p_sender)
{
    assert(p_sender);
    switch (p_sender->state_)
    {
        case SENDER_INITIAL: 
            if (sender_handle_mcastjoin_internal(p_sender))
            {
                p_sender->state_ = SENDER_MCAST_JOINED;
                return 1;
            }
            break;
        default:
            assert(0);
            break;
    }
    debug_outputln("%s %4.4u", __FILE__, __LINE__);
    return 0;
}

int sender_handle_mcastleave(struct mcast_sender * p_sender)
{
    assert(p_sender);
    switch (p_sender->state_)
    {
        case SENDER_MCAST_JOINED: 
            if (sender_handle_mcastleave_internal(p_sender))
            {
                p_sender->state_ = SENDER_INITIAL;
                return 1;
            }
            break;
        default:
            assert(0);
            break;
    }
    debug_outputln("%s %4.4u", __FILE__, __LINE__);
    return 0;
}

int sender_handle_startrecording(struct mcast_sender * p_sender)
{
    debug_outputln("%4.4u %s", __LINE__, __FILE__);
    if (NULL == p_sender->p_circular_buffer_)
    {
       p_sender->p_circular_buffer_ = circular_buffer_create_with_size(16);
    }
    assert(NULL != p_sender->p_circular_buffer_);
    if (NULL == p_sender->rec_settings_)
    {
        p_sender->rec_settings_ = recorder_settings_get_default(); 
    }
    assert(NULL != p_sender->rec_settings_);
    if (NULL == p_sender->recorder_)
    {
        p_sender->recorder_ = dxaudio_recorder_create(
            p_sender->rec_settings_, 
            p_sender->p_circular_buffer_);     
    }
    if (sender_handle_startsending_internal(p_sender))
    {
        assert(NULL != p_sender->rec_settings_);
        dxaudio_recorder_start(p_sender->recorder_);
        p_sender->state_ = SENDER_SENDING;
        return 1;
    }
    return 0;
}

int sender_handle_stoprecording(struct mcast_sender * p_sender)
{
    debug_outputln("%4.4u %s", __LINE__, __FILE__);
    if (sender_handle_stopsending_internal(p_sender))
    {
        p_sender->state_= SENDER_MCAST_JOINED;
    }
    return 1;
}

#if defined DEBUG
sender_state_t sender_get_current_state(struct mcast_sender * p_sender)
{
    return p_sender->state_;
}
#endif

struct mcast_sender * sender_create(struct sender_settings * p_settings)
{
    struct mcast_sender * p_sender = (struct mcast_sender *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct mcast_sender));
    p_sender->state_= SENDER_INITIAL;
    sender_settings_copy(&p_sender->settings_, p_settings);
    return p_sender;
}

void sender_destroy(struct mcast_sender * p_sender)
{
    if (p_sender)
    {
        switch (p_sender->state_)
        {
            /* case fall through (no 'break') is intentional. */
            case SENDER_SENDING:
                debug_outputln("%4.4u %s", __LINE__, __FILE__);
                sender_handle_stoprecording(p_sender);
            case SENDER_MCAST_JOINED:
                debug_outputln("%4.4u %s", __LINE__, __FILE__);
                sender_handle_mcastleave_internal(p_sender);
            case SENDER_INITIAL:
                debug_outputln("%4.4u %s", __LINE__, __FILE__);
                HeapFree(GetProcessHeap(), 0, p_sender);
                break;
            default:
                break;
        }
    }
}

