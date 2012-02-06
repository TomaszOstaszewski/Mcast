/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file ut-circular-buffer-uint8.c
 * @author T.Ostaszewski
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
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <process.h>
#include <assert.h>
#include <time.h>
#include "timeofday.h"
#include "circular-buffer-uint8.h"

#define CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT (1<<CIRCULAR_BUFFER_DEFAULT_LEVEL)

struct producer_consumer_params {
	struct fifo_circular_buffer * p_fifo_;
	unsigned int items_;
	unsigned int producer_delay_;
	unsigned int consumer_delay_;
	unsigned int items_produced_;
	unsigned int items_consumed_;
};

struct thread_data { 
	HANDLE * events_;
	unsigned int event_count_;
	struct producer_consumer_params * p_params_;
};

static uint8_t  g_template_buffer[256];

static void test_create_destroy_0(void)
{
    const uint8_t levels[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    size_t idx ;
    for (idx = 0; idx < sizeof(levels)/sizeof(levels[0]); ++idx)
    {
        struct fifo_circular_buffer * p_circular_buffer = fifo_circular_buffer_create_with_level(levels[idx]);
        assert( NULL != p_circular_buffer);
        assert(0 == fifo_circular_buffer_get_items_count(p_circular_buffer));
        assert(fifo_circular_buffer_is_free_space(p_circular_buffer));
        assert(!fifo_circular_buffer_is_full(p_circular_buffer));
        fifo_circular_buffer_delete(p_circular_buffer);
    }
}

static void test_create_destroy_1(void)
{
    uint8_t idx ;
    for (idx = 0; idx < 1; ++idx)
    {
        struct fifo_circular_buffer * p_circular_buffer = fifo_circular_buffer_create_with_level(idx);
        assert( NULL == p_circular_buffer);
    }
    for (idx = 17; idx != 0; ++idx)
    {
        struct fifo_circular_buffer * p_circular_buffer = fifo_circular_buffer_create_with_level(idx);
        assert( NULL == p_circular_buffer);
    }
}

static void test_insert_4_elem_into_4_elem_circular_fifo(void)
{
    struct fifo_circular_buffer * p_circular_buffer;
    uint8_t data_to_put[] = { 0, 1, 2, 3 };
    int result;

    p_circular_buffer = fifo_circular_buffer_create_with_level(2); /* 2^2 equals 4, so FIFO will hold at most 4 items */
    assert(NULL != p_circular_buffer);
    result = fifo_circular_buffer_push_item(p_circular_buffer, &data_to_put[0], sizeof(data_to_put));
    assert(0 == result);
    assert(sizeof(data_to_put)/sizeof(data_to_put[0]) == fifo_circular_buffer_get_items_count(p_circular_buffer)); 
    assert(fifo_circular_buffer_is_full(p_circular_buffer));
    fifo_circular_buffer_delete(p_circular_buffer);
}

static void test_insert_6_elem_into_4_elem_circular_fifo(void)
{
    struct fifo_circular_buffer * p_circular_buffer;
    uint8_t data_to_put[]                   = { 0, 1, 2, 3, 4, 5 };
    uint8_t const expected_data_to_fetch[]  =       { 2, 3, 4, 5 };
    int result;
    size_t count;

    p_circular_buffer = fifo_circular_buffer_create_with_level(2); /* 2^2 equals 4, so FIFO will hold at most 4 items */
    assert(NULL != p_circular_buffer);
    result = fifo_circular_buffer_push_item(p_circular_buffer, &data_to_put[0], sizeof(data_to_put));
    assert(0 == result);
    assert((1<<2) == fifo_circular_buffer_get_items_count(p_circular_buffer)); 
    assert(fifo_circular_buffer_is_full(p_circular_buffer));
    count = fifo_circular_buffer_get_items_count(p_circular_buffer);
    fifo_circular_buffer_fetch_item(p_circular_buffer, &data_to_put[0], &count);
    assert(4 == count);
    assert(0 == fifo_circular_buffer_get_items_count(p_circular_buffer));  
    assert(0 == memcmp(&data_to_put[0], &expected_data_to_fetch[0], sizeof(expected_data_to_fetch)));
    fifo_circular_buffer_delete(p_circular_buffer);
}

static void test_fetch_from_empty_queue(void)
{
    const uint8_t levels[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    size_t idx ;
    for (idx = 0; idx < sizeof(levels)/sizeof(levels[0]); ++idx)
    {
        int result;
        uint8_t container[2];
        size_t req_count = 2;
        struct fifo_circular_buffer * p_circular_buffer = fifo_circular_buffer_create_with_level(levels[idx]);
        assert( NULL != p_circular_buffer);
        assert(0 == fifo_circular_buffer_get_items_count(p_circular_buffer));
        result = fifo_circular_buffer_fetch_item(p_circular_buffer, &container[0], &req_count);   
        assert(0 == req_count);
        fifo_circular_buffer_delete(p_circular_buffer);
    }
}

static void test_default_queue(void)
{
    struct fifo_circular_buffer * p_circular_buffer;
    uint8_t data_to_put[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT];
    uint8_t fetched_data[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT];
    uint32_t req_count;
    int result;
    /* Fill arrays with random data */
    {
        size_t idx;
        for (idx = 0; idx < CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT; ++idx)
        {
            data_to_put[idx] = (uint8_t)rand();
        }
    }
    p_circular_buffer = fifo_circular_buffer_create();
    assert(NULL != p_circular_buffer);

    req_count = CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT; 
    result = fifo_circular_buffer_push_item(p_circular_buffer, &data_to_put[0], req_count);
    assert(0 == result);

    assert(req_count == fifo_circular_buffer_get_items_count(p_circular_buffer)); 
    assert(fifo_circular_buffer_is_full(p_circular_buffer));

    req_count = CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT;
    result = fifo_circular_buffer_fetch_item(p_circular_buffer, &fetched_data[0], &req_count);
    assert(CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT == req_count);
    assert(0 == result);

    assert(0 == fifo_circular_buffer_get_items_count(p_circular_buffer));
#if 0
    {
        uint8_t const * p_data;
        p_data = &data_to_put[0];
        fprintf(stderr, "%2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", p_data[0], p_data[1], p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7]);   
        p_data = &fetched_data[0];
        fprintf(stderr, "%2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", p_data[0], p_data[1], p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7]);   
        p_data = &data_to_put[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT-8];
        fprintf(stderr, "%2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", p_data[0], p_data[1], p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7]);   
        p_data = &fetched_data[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT-8];
        fprintf(stderr, "%2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", p_data[0], p_data[1], p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7]);   
    }
#endif
    assert(0 == memcmp(data_to_put, fetched_data, sizeof(fetched_data)));
    fifo_circular_buffer_delete(p_circular_buffer);
}

static void test_default_queue_overflow(void)
{
    struct fifo_circular_buffer * p_circular_buffer;
    uint32_t req_count;
    uint32_t extra_count = CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT/2;
    uint8_t data_to_put[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT];
    uint8_t fetched_data[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT];
    int result;
    /* Fill arrays with random data */
    {
        size_t idx;
        for (idx = 0; idx < CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT; ++idx)
        {
            data_to_put[idx] = (uint8_t)rand();
        }
    }

    p_circular_buffer = fifo_circular_buffer_create();
    assert(NULL != p_circular_buffer);

    /* Put the entire buffer into queue. */
    req_count = CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT; 
    result = fifo_circular_buffer_push_item(p_circular_buffer, &data_to_put[0], req_count);
    assert(0 == result);

    assert(CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT  == fifo_circular_buffer_get_items_count(p_circular_buffer)); 
    assert(fifo_circular_buffer_is_full(p_circular_buffer));

    /* Put the half of the buffer */
    req_count = extra_count;
    result = fifo_circular_buffer_push_item(p_circular_buffer, &data_to_put[0], req_count);
    assert(0 == result);

    assert(CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT == fifo_circular_buffer_get_items_count(p_circular_buffer)); 
    assert(fifo_circular_buffer_is_full(p_circular_buffer));
    
    /* Fetch the entire buffer. */
    req_count = fifo_circular_buffer_get_items_count(p_circular_buffer);
    result = fifo_circular_buffer_fetch_item(p_circular_buffer, &fetched_data[0], &req_count);
    assert(0 == result);

    assert(0 == fifo_circular_buffer_get_items_count(p_circular_buffer));
#if 0
    {
        uint8_t const * p_data;

        p_data = &data_to_put[0];
        fprintf(stderr, "%2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", p_data[0], p_data[1], p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7]);   
        p_data = &fetched_data[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT/2];
        fprintf(stderr, "%2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", p_data[0], p_data[1], p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7]);   

        p_data = &data_to_put[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT/2];
        fprintf(stderr, "%2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", p_data[0], p_data[1], p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7]);   
        p_data = &fetched_data[0];
        fprintf(stderr, "%2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", p_data[0], p_data[1], p_data[2], p_data[3], p_data[4], p_data[5], p_data[6], p_data[7]);   
    }
#endif
    assert(0 == memcmp(&fetched_data[0], &data_to_put[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT/2], CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT/2));
    assert(0 == memcmp(&fetched_data[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT/2], &data_to_put[0], CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT/2));
    fifo_circular_buffer_delete(p_circular_buffer);
}

int main(int argc, char ** argv)
{
    srand(time(NULL));
    test_create_destroy_0();
    test_create_destroy_1();
    test_insert_4_elem_into_4_elem_circular_fifo();
    test_insert_6_elem_into_4_elem_circular_fifo();
    test_fetch_from_empty_queue();
    test_default_queue();
    test_default_queue_overflow();
    //test_006();
    //test_007();
    return 0;
}

