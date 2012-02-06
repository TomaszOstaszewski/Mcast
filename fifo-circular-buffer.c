/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file fifo-circular-buffer.c
 * @brief A circular buffer implementation.
 * @details 
 * @date 04-Jan-2012
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
 */
#include "pcc.h"
#include "fifo-circular-buffer.h"

/*! 
 * @brief
 */
struct fifo_circular_buffer_header
{
    uint32_t max_items_; /*!< Maximum number of items the queue can hold.*/
    volatile uint32_t read_idx_; /*!< Current read index. */
    volatile uint32_t write_idx_; /*!< Current read index.*/
};

/*!
 * @brief The circular buffer data structure
 */
struct fifo_circular_buffer
{
    struct fifo_circular_buffer_header hdr_;
    uint8_t data_buffer_[1];    /*!< Data buffer from which bytes are read/to which will be written. */
};

struct fifo_circular_buffer *  fifo_circular_buffer_create_with_level(uint8_t level)
{
    if (level >= 2 && level <= 16)
    {
        struct fifo_circular_buffer * p_buffer;
        p_buffer = malloc(sizeof(struct fifo_circular_buffer_header)+sizeof(uint8_t)*(1<<level));
        assert(NULL != p_buffer);
        p_buffer->hdr_.max_items_ = 1 << level;
        p_buffer->hdr_.read_idx_ = 0;
        p_buffer->hdr_.write_idx_ = 0;
        return p_buffer;
    }
    return NULL;
}

struct fifo_circular_buffer * fifo_circular_buffer_create()
{
    return fifo_circular_buffer_create_with_level(CIRCULAR_BUFFER_DEFAULT_LEVEL);
}

void fifo_circular_buffer_delete(struct fifo_circular_buffer * p_circular_buffer)
{
    free(p_circular_buffer);
}

uint32_t fifo_circular_buffer_get_capacity(struct fifo_circular_buffer const * p_circular_buffer)
{
    return p_circular_buffer->hdr_.max_items_;
}

uint32_t fifo_circular_buffer_get_items_count(struct fifo_circular_buffer const * p_circular_buffer)
{
    return p_circular_buffer->hdr_.write_idx_ - p_circular_buffer->hdr_.read_idx_;
}

int fifo_circular_buffer_is_free_space(struct fifo_circular_buffer * p_circular_buffer)
{
    return (p_circular_buffer->hdr_.write_idx_ - p_circular_buffer->hdr_.read_idx_) < p_circular_buffer->hdr_.max_items_;
}

int fifo_circular_buffer_push_item(struct fifo_circular_buffer * p_circular_buffer, struct data_item const * p_item)
{
    uint16_t buffer_index;
    uint16_t idx;
    for (idx = 0 
         ;idx != p_item->count_
         ;++idx, ++p_circular_buffer->hdr_.write_idx_) 
    {
        buffer_index = (p_circular_buffer->hdr_.max_items_ -1) & p_circular_buffer->hdr_.write_idx_;
        assert(buffer_index < p_circular_buffer->hdr_.max_items_);
        p_circular_buffer->data_buffer_[buffer_index] = p_item->data_[idx];
        if (p_circular_buffer->hdr_.write_idx_ - p_circular_buffer->hdr_.read_idx_ == p_circular_buffer->hdr_.max_items_)
        {
            ++p_circular_buffer->hdr_.read_idx_;
        }
    }
    return 0;
}

int fifo_circular_buffer_fetch_item(struct fifo_circular_buffer * p_circular_buffer, struct data_item * p_item)
{
    uint16_t idx;
    for (idx = 0
        ; idx != p_item->count_ && (p_circular_buffer->hdr_.write_idx_ - p_circular_buffer->hdr_.read_idx_) != 0
        ; ++idx, ++p_circular_buffer->hdr_.read_idx_)
    {
        p_item->data_[idx] = p_circular_buffer->data_buffer_[((p_circular_buffer->hdr_.max_items_-1) & p_circular_buffer->hdr_.read_idx_)];
    }
    p_item->count_ = idx;
    return 0;
}

unsigned int fifo_circular_buffer_is_full(struct fifo_circular_buffer * p_fifo)
{
    return p_fifo->hdr_.max_items_ == fifo_circular_buffer_get_items_count(p_fifo);
}

