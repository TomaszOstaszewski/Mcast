/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file fifo-circular-buffer.c
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
 * @brief 
 * @details 
 */
#include "pcc.h"
#include "fifo-circular-buffer.h"

#define MAX_ITEMS (0x10000)

/*!
 * @brief
 */
struct fifo_circular_buffer
{
    uint32_t    read_idx_;                  /*!< */
    uint32_t    write_idx_;                 /*!< */
    uint8_t     data_buffer_[MAX_ITEMS];    /*!< */
};

struct fifo_circular_buffer *  fifo_circular_buffer_create(void)
{
    struct fifo_circular_buffer * p_buffer;
    p_buffer = malloc(sizeof(struct fifo_circular_buffer));
    memset(p_buffer, 0, sizeof(struct fifo_circular_buffer));
    assert(NULL != p_buffer);
    return p_buffer;
}

void fifo_circular_buffer_delete(struct fifo_circular_buffer * p_circular_buffer)
{
    free(p_circular_buffer);
}

uint32_t fifo_circular_buffer_get_capacity(struct fifo_circular_buffer * p_circular_buffer)
{
    return MAX_ITEMS;
}

uint32_t fifo_circular_buffer_get_items_count(struct fifo_circular_buffer * p_circular_buffer)
{
    return p_circular_buffer->write_idx_ - p_circular_buffer->read_idx_;
}

int fifo_circular_buffer_is_free_space(struct fifo_circular_buffer * p_circular_buffer)
{
    return (p_circular_buffer->write_idx_ - p_circular_buffer->read_idx_) < MAX_ITEMS;
}

int fifo_circular_buffer_push_item(struct fifo_circular_buffer * p_circular_buffer, struct data_item const * p_item)
{
    uint32_t idx;
    for (idx = 0; 
         idx != p_item->count_; 
         ++idx, ++p_circular_buffer->write_idx_) 
    {
        p_circular_buffer->data_buffer_[((0x0000ffff) & p_circular_buffer->write_idx_)] = p_item->data_[idx];
        if ((p_circular_buffer->write_idx_ - p_circular_buffer->read_idx_) == MAX_ITEMS)
        {
            ++p_circular_buffer->read_idx_;
        }
    }
    return 0;
}

int fifo_circular_buffer_fetch_item(struct fifo_circular_buffer * p_circular_buffer, struct data_item * p_item)
{
    uint32_t    idx;
    for ( idx = 0
        ; idx != p_item->count_ && (p_circular_buffer->write_idx_ - p_circular_buffer->read_idx_) != 0
        ; ++idx, ++p_circular_buffer->read_idx_)
    {
        p_item->data_[idx] = p_circular_buffer->data_buffer_[((0x0000ffff) & p_circular_buffer->read_idx_)];
    }
    p_item->count_ = idx;
    return 0;
}

unsigned int fifo_circular_buffer_is_full(struct fifo_circular_buffer * p_fifo)
{
    return MAX_ITEMS == fifo_circular_buffer_get_items_count(p_fifo);
}

