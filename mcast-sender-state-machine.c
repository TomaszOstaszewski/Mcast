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
#include "mcast-sender-state-machine.h"

#include "mcast_setup.h"
#include "debug_helpers.h"
#include "wave_utils.h"
#include "mcast-sender-state-machine.h"
#include "sender-settings.h"

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
    /** @brief Pointer to the first byte of data being send. */
    master_riff_chunk_t * chunk_;
	/** @brief Pointer to the structure that describes multicast connection */
    struct mcast_connection * conn_;   
	/** @brief Copy of the sender settings. */
    struct sender_settings settings_; 
    /*!
     * @brief Current offset from the beginning of the WAV file to the next byte being send.
     */
    uint32_t send_offset_;
    HANDLE hStopEvent_; /*!< Handle to the event used to stop the sending thread. */
    /*!
     * @brief Handle to the event used to stop the sending thread. 
     * @details This is a copy of the handle stored in hStopEvent_ member. To avoid
     * race conditions between the sender thread and the main thread, there are 2 handles 
     * that describe the very same event. Both the main thread and the sender thread operate
     * on their own copy, therefore avoid race conditions between CloseHandle() calls.
     * @sa hStopEvent_
     */
    HANDLE hStopEventThread_;
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
    struct mcast_sender * p_sender;
    struct master_riff_chunk * p_master_riff;
    uint32_t send_delay;
    p_sender = (struct mcast_sender *)param;
    assert(p_sender);
    send_delay = p_sender->settings_.send_delay_;
    p_master_riff = p_sender->settings_.chunk_;
    assert(p_master_riff);
    {   
        uint32_t max_offset = p_master_riff->format_chunk_.subchunk_.subchunk_size_;
        uint32_t max_chunk_size = p_sender->settings_.chunk_size_;
        int8_t const * const p_data_begin = p_master_riff->format_chunk_.subchunk_.samples8_;
        assert(max_chunk_size <= MAX_ETHER_PAYLOAD_SANS_UPD_IP);
        for (;;)
        {
            dwResult = WaitForSingleObject(p_sender->hStopEventThread_, send_delay);
            if (WAIT_TIMEOUT == dwResult)
            {
                int result;
                uint32_t chunk_size = max_chunk_size;
                if (p_sender->send_offset_ + chunk_size > max_offset)
                {
                    chunk_size = max_offset - p_sender->send_offset_;
                }
                assert(p_sender->send_offset_ + chunk_size <= max_offset);
                result = mcast_sendto(p_sender->conn_, p_data_begin + p_sender->send_offset_, chunk_size);
                if (SOCKET_ERROR != result)
                {
                    p_sender->send_offset_ += result;
                    assert (p_sender->send_offset_ <= max_offset);
                    if (p_sender->send_offset_ >= max_offset)
                    {
                        p_sender->send_offset_ = 0;
                    }
                }
            }
            else if (WAIT_OBJECT_0 == dwResult)
            {
                dwResult = 0;
                break;
            }
            else 
            {
                dwResult = -1;
                debug_outputln("%s %d : %d", __FILE__, __LINE__, dwResult);
                break;
            }
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
                    result = 1;
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
        p_sender->hStopEvent_ = NULL;
        p_sender->hSenderThread_ = NULL;
        return 1;
    }
    return 0;
}

struct mcast_sender * sender_create(struct sender_settings * p_settings)
{
    struct mcast_sender * p_sender = (struct mcast_sender *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct mcast_sender));
    p_sender->chunk_ = p_settings->chunk_;
    sender_settings_copy(&p_sender->settings_, p_settings);
    return p_sender;
}

void sender_destroy(struct mcast_sender * p_sender)
{
    HeapFree(GetProcessHeap(), 0, p_sender);
}

sender_state_t sender_get_current_state(struct mcast_sender * p_sender)
{
    return p_sender->state_;
}

void sender_handle_mcastjoin(struct mcast_sender * p_sender)
{
    assert(p_sender);
    if (SENDER_INITIAL == p_sender->state_ && sender_handle_mcastjoin_internal(p_sender))
    {
        p_sender->state_ = SENDER_MCAST_JOINED;
        return;
    }
    debug_outputln("%s %5.5d", __FILE__, __LINE__);
}

void sender_handle_mcastleave(struct mcast_sender * p_sender)
{
    assert(p_sender);
    if (SENDER_MCAST_JOINED == p_sender->state_ && sender_handle_mcastleave_internal(p_sender))
    {
        p_sender->state_ = SENDER_INITIAL;
        return;
    }
    debug_outputln("%s %5.5d", __FILE__, __LINE__);
}

void sender_handle_startsending(struct mcast_sender * p_sender)
{
    assert(p_sender);
    if (SENDER_MCAST_JOINED == p_sender->state_ && sender_handle_startsending_internal(p_sender))
    {
        p_sender->state_ = SENDER_SENDING;
        return;
    }
    debug_outputln("%s %5.5d", __FILE__, __LINE__);
}

void sender_handle_stopsending(struct mcast_sender * p_sender)
{
    assert(p_sender);
    if (SENDER_SENDING == p_sender->state_ && sender_handle_stopsending_internal(p_sender))
    {
        p_sender->state_ = SENDER_MCAST_JOINED;
    }
    debug_outputln("%s %5.5d", __FILE__, __LINE__);
}

