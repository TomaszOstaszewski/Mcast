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
 * @date 04-Jan-2012
 */ 
#include "pcc.h"
#include "mcast-sender-state-machine.h"

#include "mcast_setup.h"
#include "resource.h"
#include "debug_helpers.h"
#include "wave_utils.h"
#include "mcast-sender-state-machine.h"
#include "mcast-sender-settings.h"

/**
 * @brief Description of the multicast sender state machine.
 */
struct mcast_sender { 
    sender_state_t state_; /*!< Current sender state. */
    master_riff_chunk_t * chunk_; /*<! Pointer to the first byte of data being send. */
    struct mcast_connection * conn_;   /*!< Pointer to the structure that describes multicast connection */
    struct sender_settings * settings_; /*!< Pointer to the structure that describes sender settings. */
    /*!
     * @brief Pointer to the next byte being send 
     */
    uint8_t const * p_current_pos_;
    HANDLE hStopEvent_; /*!< Handle to the event used to stop the sending thread. */
    /*!
     * @brief Handle to the event used to stop the sending thread. 
     * @details This is a copy of the handle stored in hStopEvent_ member. To avoid
     * race conditions between the sender thread and the main thread, there are 2 handles 
     * that describe the very same event. Both the main thread and the sender thread operate
     * on their own copy, therefore avoid race conditions between CloseHandle() calls.
     * @sa hStopEvent_
     */
    HANDLE hStopEvent_thread_;
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
    uint32_t max_offset;
    struct mcast_sender * p_sender = (struct mcast_sender *)param;
    struct master_riff_chunk * p_master_riff;
    int8_t const * p_data_begin;
    DWORD dwResult;
    p_sender = (struct mcast_sender *)param;
    assert(p_sender);
    assert(p_sender->settings_);
    p_master_riff = p_sender->settings_->chunk_;
    assert(p_master_riff);
    p_data_begin = &p_master_riff->format_chunk_.subchunk_.samples8_[0];
    max_offset = p_master_riff->format_chunk_.subchunk_.subchunk_size_;
    for (;;)
    {
        dwResult = WaitForSingleObject(p_sender->hStopEvent_thread_, p_sender->settings_->send_delay_);
        if (WAIT_TIMEOUT == dwResult)
        {
            int result;
            uint32_t offset = p_sender->p_current_pos_ - p_data_begin;
            uint16_t chunk_size = p_sender->settings_->chunk_size_;
            if (offset + chunk_size > max_offset)
            {
                chunk_size = max_offset - offset;
            }
            result = sendto(p_sender->conn_->socket_, 
                    (const char *)p_sender->p_current_pos_, 
                    chunk_size,
                    0,
                    p_sender->conn_->multiAddr_->ai_addr,
                    (int) p_sender->conn_->multiAddr_->ai_addrlen
                    );
            p_sender->p_current_pos_+= chunk_size;
            if ((uint32_t)(p_sender->p_current_pos_ - p_data_begin) + chunk_size >= max_offset)
            {
                p_sender->p_current_pos_ = p_data_begin;
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
    CloseHandle(p_sender->hStopEvent_thread_);
    p_sender->hStopEvent_thread_ = NULL;
    return dwResult;
}

/**
 * @brief Joins the multicast group for which sender is configured to join.
 * @param[in] p_sender pointer to the sender description structure.
 * @return returns 0 on success, <>0 otherwise.
 */
static int sender_handle_mcastjoin_internal(struct mcast_sender * p_sender)
{
    assert(NULL == p_sender->conn_);
    p_sender->conn_ = HeapAlloc(GetProcessHeap(), 0, sizeof(struct mcast_connection));
    assert(NULL != p_sender->conn_);
    if (NULL != p_sender->conn_)
    {
        int rc;
        char * psz_addr;
        char psz_port[8] = { 0 };
        struct in_addr * p_in_addr;
        HRESULT hr;

        p_in_addr =(struct in_addr *)&p_sender->settings_->ipv4_mcast_group_addr_;
        psz_addr = inet_ntoa(*p_in_addr);
        assert(psz_addr);
        if (psz_addr)
        {
            hr = StringCchPrintf(psz_port, 8, "%5.5u", p_sender->settings_->mcast_port_);
            if (SUCCEEDED(hr))
            { 
                debug_outputln("%s %d : %s:%s", __FILE__, __LINE__, psz_addr, psz_port);
                rc = setup_multicast_3(psz_addr, psz_port, p_sender->conn_);
                if (0 == rc)
                {
                    return 0;
                }
            }   
        }
        HeapFree(GetProcessHeap(), 0, p_sender->conn_);
        p_sender->conn_ = NULL;
    }
    return (-1);
}

/**
 * @brief Leaves the multicast group.
 * @param[in] p_sender pointer to the sender description structure.
 * @return returns 0 on success, <>0 otherwise.
 */
static int sender_handle_mcastleave_internal(struct mcast_sender * p_sender)
{
    int result;
    result = close_multicast(p_sender->conn_);
    HeapFree(GetProcessHeap(), 0, p_sender->conn_);
    p_sender->conn_ = NULL;
    return 0;
}

/**
 * @brief Starts sending data over the multicast connection.
 * @param[in] p_sender pointer to the sender description structure.
 * @return returns 0 on success, <>0 otherwise.
 */
static int sender_handle_startsending_internal(struct mcast_sender * p_sender)
{
    assert(NULL == p_sender->hStopEvent_);
    p_sender->hStopEvent_ = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL != p_sender->hStopEvent_)
    {
        BOOL bDupResult; 
        bDupResult = DuplicateHandle(GetCurrentProcess(), p_sender->hStopEvent_, GetCurrentProcess(), &p_sender->hStopEvent_thread_, 0, FALSE, DUPLICATE_SAME_ACCESS);
        if (bDupResult)
        {
            HANDLE hSenderThread;
            hSenderThread = CreateThread(NULL, 0, SendThreadProc, p_sender, 0, NULL);
            CloseHandle(hSenderThread);
            return 0;
        }
        CloseHandle(p_sender->hStopEvent_);
        p_sender->hStopEvent_ = NULL;
    }
    return -1;
}

/**
 * @brief Stops sending data over the multicast connection.
 * @param[in] p_sender pointer to the sender description structure.
 * @return returns 0 on success, <>0 otherwise.
 */
static int sender_handle_stopsending_internal(struct mcast_sender * p_sender)
{
    SetEvent(p_sender->hStopEvent_);   
    CloseHandle(p_sender->hStopEvent_);
    p_sender->hStopEvent_ = NULL;
    return 0;
}

struct mcast_sender * sender_create(struct sender_settings * p_settings)
{
    struct mcast_sender * p_sender = (struct mcast_sender *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct mcast_sender));
    p_sender->chunk_ = p_settings->chunk_;
    p_sender->settings_ = p_settings;
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
    if (SENDER_INITIAL == p_sender->state_)
    {
        if (0 == sender_handle_mcastjoin_internal(p_sender))
            p_sender->state_ = SENDER_MCAST_JOINED;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

void sender_handle_mcastleave(struct mcast_sender * p_sender)
{
    if (SENDER_MCAST_JOINED == p_sender->state_)
    {
        if (0 == sender_handle_mcastleave_internal(p_sender))
            p_sender->state_ = SENDER_INITIAL;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

void sender_handle_startsending(struct mcast_sender * p_sender)
{
    assert(p_sender);
    if (SENDER_MCAST_JOINED == p_sender->state_)
    {
        if (0 == sender_handle_startsending_internal(p_sender))
            p_sender->state_ = SENDER_SENDING;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

void sender_handle_stopsending(struct mcast_sender * p_sender)
{
    if (SENDER_SENDING == p_sender->state_)
    {
        if (0 == sender_handle_stopsending_internal(p_sender))
            p_sender->state_ = SENDER_MCAST_JOINED;
    }
    else
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
    }
}

