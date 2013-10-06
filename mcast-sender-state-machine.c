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
#include "circular-buffer-uint16.h"
#include "dsound-recorder.h"
#include "recorder-settings.h"
#include "soxr-lsr.h"

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
    struct dxaudio_recorder_block * recorder_;
    /** @brief */
    struct circular_buffer_uint16 * p_circular_buffer_;
    /** @brief */
    recorder_settings_t rec_settings_;
};

/**
 * @brief Simple resampling from 11025 Hz to 8000 Hz
 * @details This works exactly in a way that Bresenham line drawing algorithm works
 * @param[in]
 * @param[in]
 * @param[in, out]
 * @param[in]
 */ 
size_t resample_16_bit_11025_to_8000(uint16_t const * input, size_t input_size, uint16_t * output, size_t output_size)
{
    static int error = 0;
#define DELTA_X 441
#define DELTA_Y 320
    size_t in_idx;
    size_t out_idx;
    for (in_idx = 0, out_idx = 0; in_idx < input_size && out_idx < output_size; ++in_idx)
    {
        output[out_idx] = input[in_idx];
        if (2 * (error + DELTA_Y) < DELTA_X)
        {
            error = error + DELTA_Y;
        }
        else
        {
            error = error + DELTA_Y - DELTA_X;
            out_idx = out_idx + 1;
        }
    }
    return out_idx;
}

static void mcast_send_data_packet(void * p_context, void * data, size_t data_size)
{
#if 1
#define INPUT_SAMPLING_FREQ (11025.0)
#define OUTPUT_SAMPLING_FREQ (8000.0)

    static float f_temp_input_samples[1024];
    static float f_temp_output_samples[1024];
    static uint16_t output_samples[1024];
    static struct SRC_DATA conversion_params;

    size_t idx;
    struct mcast_sender * p_sender;
    int16_t const * input_samples;
    int error;

    p_sender = (struct mcast_sender *)p_context;
    input_samples = (int16_t const *)data;

    for (idx = 0; idx < COUNTOF_ARRAY(f_temp_input_samples) && idx < data_size/sizeof(input_samples[0]); ++idx)
        f_temp_input_samples[idx] = input_samples[idx];
    /* Do resampling here */
    conversion_params.data_in = f_temp_input_samples;
    conversion_params.data_out = f_temp_output_samples;
    conversion_params.input_frames = idx;
    conversion_params.output_frames = COUNTOF_ARRAY(f_temp_output_samples);
    conversion_params.src_ratio = OUTPUT_SAMPLING_FREQ/INPUT_SAMPLING_FREQ;
    error = src_simple(&conversion_params, SRC_SINC_FASTEST, 1);
    if (0 == error)
    {
        /* Get back the samples */
        for (idx = 0; idx < COUNTOF_ARRAY(output_samples) && idx < (size_t)conversion_params.output_frames_gen; ++idx)
            output_samples[idx] = (int16_t)f_temp_output_samples[idx];
        mcast_sendto(p_sender->conn_, output_samples, (size_t)conversion_params.output_frames_gen*sizeof(int16_t));
    }
#else
    struct mcast_sender * p_sender;
    p_sender = (struct mcast_sender *)p_context;
    mcast_sendto(p_sender->conn_, data, data_size);
#endif
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
 * @brief Stops sending data over the multicast connection.
 * @param[in] p_sender pointer to the sender description structure.
 * @return returns non-zero on success, 0 otherwise.
 */
static int sender_handle_stopsending_internal(struct mcast_sender * p_sender)
{
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
       p_sender->p_circular_buffer_ = circular_buffer_uint16_create_with_size(16);
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
            p_sender,
            mcast_send_data_packet);     
    }
    assert(NULL != p_sender->rec_settings_);
    dxaudio_recorder_start(p_sender->recorder_);
    p_sender->state_ = SENDER_SENDING;
    return 1;
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

