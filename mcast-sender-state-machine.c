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
    P_MASTER_RIFF_CONST chunk_;
    /** @brief Pointer to the structure that describes multicast connection */
    struct mcast_connection * conn_;   
    /** @brief Copy of the sender settings. */
    struct sender_settings settings_; 
    /** @brief The tone that is to be played. */
    struct abstract_tone * tone_;
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
    uint32_t send_offset;
    uint32_t send_delay;
    size_t max_offset;
    uint32_t max_chunk_size;
    int8_t const * p_data_begin;

    p_sender = (struct mcast_sender *)param;
    assert(p_sender);
    assert(p_sender->tone_);
    max_chunk_size = sender_settings_get_chunk_size_bytes(&p_sender->settings_);
    assert(max_chunk_size <= MAX_ETHER_PAYLOAD_SANS_UPD_IP);
    send_offset = 0;
    p_data_begin = (int8_t const *)abstract_tone_get_wave_data(p_sender->tone_, &max_offset);
    assert(p_data_begin);
    debug_outputln("%4.4u %s : %p %u %u", __LINE__, __FILE__, p_data_begin, max_chunk_size, max_offset);
    for (;;)
    {
        int result;
        uint32_t chunk_size = max_chunk_size;
        if (send_offset + chunk_size > max_offset)
        {
            chunk_size = max_offset - send_offset;
        }
        assert(send_offset + chunk_size <= max_offset);
        /* We wait here to emulate the time it takes to gather the samples. 
         * Time to wait is proportional to the send chunk length.
         */
        send_delay = sender_settings_convert_bytes_to_ms(&p_sender->settings_, chunk_size);
        dwResult = WaitForSingleObject(p_sender->hStopEventThread_, send_delay);
        if (WAIT_TIMEOUT == dwResult)
        {
            result = mcast_sendto(p_sender->conn_, p_data_begin + send_offset, chunk_size);
            if (SOCKET_ERROR != result)
            {
                send_offset += result;
                assert (send_offset <= max_offset);
                if (send_offset >= max_offset)
                {
                    send_offset = 0;
                }
                //debug_outputln("%4.4u %s : %p %u", __LINE__, __FILE__, p_data_begin+send_offset, chunk_size);
            }
            else
            {
                debug_outputln("%4.4u %s : %p %u", __LINE__, __FILE__, p_data_begin+send_offset, chunk_size);
            }
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

static int sender_handle_selecttone_internal(struct mcast_sender * p_sender, struct abstract_tone * p_tone)
{
    assert(NULL == p_sender->tone_);
    p_sender->tone_ = p_tone;
    return 1;
}

static int sender_handle_deselecttone_internal(struct mcast_sender * p_sender)
{
    assert(p_sender->tone_);
    p_sender->tone_ = NULL;
    return 1;
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
        p_sender->hStopEvent_ = NULL;
        p_sender->hSenderThread_ = NULL;
        return 1;
    }
    return 0;
}

struct mcast_sender * sender_create(struct sender_settings * p_settings)
{
    struct mcast_sender * p_sender = (struct mcast_sender *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct mcast_sender));
    sender_settings_copy(&p_sender->settings_, p_settings);
    return p_sender;
}

void sender_destroy(struct mcast_sender * p_sender)
{
    HeapFree(GetProcessHeap(), 0, p_sender);
}

#if defined DEBUG
sender_state_t sender_get_current_state(struct mcast_sender * p_sender)
{
    return p_sender->state_;
}
#endif

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
        case SENDER_TONE_SELECTED: 
            if (sender_handle_mcastjoin_internal(p_sender))
            {
                p_sender->state_ = SENDER_MCASTJOINED_TONESELECTED;
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
        case SENDER_MCASTJOINED_TONESELECTED: 
            if (sender_handle_mcastleave_internal(p_sender))
            {
                p_sender->state_ = SENDER_TONE_SELECTED;
                return 1;
            }
        default:
            assert(0);
            break;
    }
    debug_outputln("%s %4.4u", __FILE__, __LINE__);
    return 0;
}

int sender_handle_selecttone(struct mcast_sender * p_sender, struct abstract_tone * p_tone)
{
    assert(p_sender);
    switch (p_sender->state_)
    {
        case SENDER_INITIAL: 
            if (sender_handle_selecttone_internal(p_sender, p_tone))
            {
                p_sender->state_ = SENDER_TONE_SELECTED;
                return 1;
            }
            break;
        case SENDER_MCAST_JOINED: 
            if (sender_handle_selecttone_internal(p_sender, p_tone))
            {
                p_sender->state_ = SENDER_MCASTJOINED_TONESELECTED;
                return 1;
            }
        default:
            assert(0);
            break;
    }
    debug_outputln("%s %4.4u", __FILE__, __LINE__);
    return 0;
}

int sender_handle_deselecttone(struct mcast_sender * p_sender)
{
    assert(p_sender);
    switch (p_sender->state_)
    {
        case SENDER_TONE_SELECTED: 
            if (sender_handle_deselecttone_internal(p_sender))
            {
                p_sender->state_ = SENDER_INITIAL;
                return 1;
            }
            break;
        case SENDER_MCASTJOINED_TONESELECTED: 
            if (sender_handle_deselecttone_internal(p_sender))
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
    assert(0);
    return 0;
}

int sender_handle_startsending(struct mcast_sender * p_sender)
{
    assert(p_sender);
    switch (p_sender->state_)
    {
        case SENDER_MCASTJOINED_TONESELECTED: 
            if (sender_handle_startsending_internal(p_sender))
            {
                p_sender->state_ = SENDER_SENDING;
                return 1;
            }
            break;
        default:
            assert(0);
            break;
    }
    debug_outputln("%s %4.4u : %d", __FILE__, __LINE__, p_sender->state_);
    assert(0);
    return 0;
}

int sender_handle_stopsending(struct mcast_sender * p_sender)
{
    assert(p_sender);
    switch (p_sender->state_)
    {
        case SENDER_SENDING: 
            if (sender_handle_stopsending_internal(p_sender))
            {
                p_sender->state_ = SENDER_MCASTJOINED_TONESELECTED;
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

