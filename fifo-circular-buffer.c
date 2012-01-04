/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file fifo-circular-buffer.c
 * @author T. Ostaszewski
 * @date 04-Jan-2012
 * @brief 
 * @details 
 */
#include "pcc.h"
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
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

