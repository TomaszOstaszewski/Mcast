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
    struct data_item item;
    int result;

    p_circular_buffer = fifo_circular_buffer_create_with_level(2); /* 2^2 equals 4, so FIFO will hold at most 4 items */
    assert(NULL != p_circular_buffer);
    item.count_ = sizeof(data_to_put); 
    item.data_ = &data_to_put[0];
    result = fifo_circular_buffer_push_item(p_circular_buffer, &item);
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
    struct data_item item;
    int result;

    p_circular_buffer = fifo_circular_buffer_create_with_level(2); /* 2^2 equals 4, so FIFO will hold at most 4 items */
    assert(NULL != p_circular_buffer);
    item.count_ = sizeof(data_to_put); 
    item.data_ = &data_to_put[0];
    result = fifo_circular_buffer_push_item(p_circular_buffer, &item);
    assert(0 == result);
    assert((1<<2) == fifo_circular_buffer_get_items_count(p_circular_buffer)); 
    assert(fifo_circular_buffer_is_full(p_circular_buffer));
    item.count_ = fifo_circular_buffer_get_items_count(p_circular_buffer);
    fifo_circular_buffer_fetch_item(p_circular_buffer, &item);
    assert(0 == fifo_circular_buffer_get_items_count(p_circular_buffer));  
    assert(4 == item.count_);
    assert(0 == memcmp(item.data_, expected_data_to_fetch, sizeof(expected_data_to_fetch)));
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
        struct data_item item;
        struct fifo_circular_buffer * p_circular_buffer = fifo_circular_buffer_create_with_level(levels[idx]);
        assert( NULL != p_circular_buffer);
        assert(0 == fifo_circular_buffer_get_items_count(p_circular_buffer));
        item.count_ = sizeof(container)/sizeof(container[0]);
        item.data_ = container;
        result = fifo_circular_buffer_fetch_item(p_circular_buffer, &item);   
        assert(0 == item.count_);
        fifo_circular_buffer_delete(p_circular_buffer);
    }
}

static void test_default_queue(void)
{
    struct fifo_circular_buffer * p_circular_buffer;
    uint8_t data_to_put[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT];
    uint8_t expected_data_to_fetch[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT];
    struct data_item item;
    int result;
    /* Fill arrays with random data */
    {
        size_t idx;
        srand(time(NULL));
        for (idx = 0; idx < CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT; ++idx)
        {
            data_to_put[idx] = expected_data_to_fetch[idx] = (uint8_t)rand();
        }
    }
    p_circular_buffer = fifo_circular_buffer_create();
    assert(NULL != p_circular_buffer);
    item.count_ = sizeof(data_to_put) - 1; 
    item.data_ = &data_to_put[0];
    result = fifo_circular_buffer_push_item(p_circular_buffer, &item);
    assert(0 == result);
    assert(sizeof(data_to_put) - 1 == fifo_circular_buffer_get_items_count(p_circular_buffer)); 

    item.count_ = 1;
    item.data_ = &data_to_put[sizeof(data_to_put)-1];
    result = fifo_circular_buffer_push_item(p_circular_buffer, &item);
    assert(0 == result);
    assert(sizeof(data_to_put) == fifo_circular_buffer_get_items_count(p_circular_buffer)); 
    assert(fifo_circular_buffer_is_full(p_circular_buffer));

    item.count_ = sizeof(data_to_put)-1;
    item.data_ = &data_to_put[0];
    result = fifo_circular_buffer_fetch_item(p_circular_buffer, &item);
    assert(0 == result);
    item.count_ = 1;
    item.data_ = &data_to_put[sizeof(data_to_put)-1];    
    result = fifo_circular_buffer_fetch_item(p_circular_buffer, &item);
    assert(0 == result);

    assert(0 == fifo_circular_buffer_get_items_count(p_circular_buffer));
#if 0
    {
        uint8_t const * p_data;
        p_data = &data_to_put[0];
        fprintf(stderr, "%2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", p_data[0], p_data[1], p_data[2], p_data[3]);   
        p_data = &expected_data_to_fetch[0];
        fprintf(stderr, "%2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", p_data[0], p_data[1], p_data[2], p_data[3]);   
        p_data = &data_to_put[65535-3];
        fprintf(stderr, "%2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", p_data[0], p_data[1], p_data[2], p_data[3]);   
        p_data = &expected_data_to_fetch[65535-3];
        fprintf(stderr, "%2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", p_data[0], p_data[1], p_data[2], p_data[3]);   
    }
#endif
    assert(0 == memcmp(data_to_put, expected_data_to_fetch, sizeof(expected_data_to_fetch)));
    fifo_circular_buffer_delete(p_circular_buffer);
}

static void test_default_queue_overflow(void)
{
    struct fifo_circular_buffer * p_circular_buffer;
    uint8_t data_to_put[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT];
    uint8_t expected_data_to_fetch[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT];
    struct data_item item;
    int result;
    /* Fill arrays with random data */
    {
        size_t idx;
        srand(time(NULL));
        for (idx = 0; idx < CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT; ++idx)
        {
            data_to_put[idx] = expected_data_to_fetch[idx] = (uint8_t)rand();
        }
    }

    p_circular_buffer = fifo_circular_buffer_create();
    assert(NULL != p_circular_buffer);
    item.count_ = sizeof(data_to_put) - 1; 
    item.data_ = &data_to_put[0];
    result = fifo_circular_buffer_push_item(p_circular_buffer, &item);
    assert(0 == result);
    assert(sizeof(data_to_put) - 1 == fifo_circular_buffer_get_items_count(p_circular_buffer)); 

    item.count_ = 1;
    item.data_ = &data_to_put[sizeof(data_to_put)-1];
    result = fifo_circular_buffer_push_item(p_circular_buffer, &item);
    assert(0 == result);
    assert(sizeof(data_to_put) == fifo_circular_buffer_get_items_count(p_circular_buffer)); 
    assert(fifo_circular_buffer_is_full(p_circular_buffer));

    item.count_ = CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT / 2;
    item.data_ = &data_to_put[0];
    result = fifo_circular_buffer_push_item(p_circular_buffer, &item);
    assert(0 == result);
    assert(sizeof(data_to_put) == fifo_circular_buffer_get_items_count(p_circular_buffer)); 
    assert(fifo_circular_buffer_is_full(p_circular_buffer));
     
    item.count_ = sizeof(data_to_put)-1;
    item.data_ = &data_to_put[0];
    result = fifo_circular_buffer_fetch_item(p_circular_buffer, &item);
    assert(0 == result);
    item.count_ = 1;
    item.data_ = &data_to_put[sizeof(data_to_put)-1];    
    result = fifo_circular_buffer_fetch_item(p_circular_buffer, &item);
    assert(0 == result);

    assert(0 == fifo_circular_buffer_get_items_count(p_circular_buffer));
#if 0
    {
        uint8_t const * p_data;
        p_data = &data_to_put[0];
        fprintf(stderr, "%2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", p_data[0], p_data[1], p_data[2], p_data[3]);   
        p_data = &expected_data_to_fetch[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT/2];
        fprintf(stderr, "%2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", p_data[0], p_data[1], p_data[2], p_data[3]);   
        p_data = &data_to_put[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT/2];
        fprintf(stderr, "%2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", p_data[0], p_data[1], p_data[2], p_data[3]);   
        p_data = &expected_data_to_fetch[0];
        fprintf(stderr, "%2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", p_data[0], p_data[1], p_data[2], p_data[3]);   
    }
#endif
    assert(0 == memcmp(&data_to_put, &expected_data_to_fetch[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT/2], CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT/2));
    assert(0 == memcmp(&data_to_put[CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT/2], &expected_data_to_fetch[0], CIRCULAR_BUFFER_DEFAULT_ITEMS_COUNT/2));
    fifo_circular_buffer_delete(p_circular_buffer);
}

unsigned int produce_0(struct fifo_circular_buffer * p_fifo)
{
	return 0;
}

static unsigned __stdcall producer_0_entry_point(void * lparam)
{
	struct thread_data * p_data;
	struct producer_consumer_params * p_params;
	unsigned int messages;
	p_data = (struct thread_data *)lparam;
	srand((unsigned int)time(NULL));
	p_params = p_data->p_params_;
	for (messages = 0; messages < p_params->items_; ++messages, ++p_params->items_produced_)
	{
		struct data_item data_item;
		gettimeofday((struct timeval*)&data_item, NULL);
		fifo_circular_buffer_push_item(p_params->p_fifo_, &data_item);
		Sleep(p_params->producer_delay_);
	}
	SetEvent(p_data->events_[0]);
	_endthreadex(0);
	return 0;
}

static unsigned __stdcall consumer_0_entry_point(void * lparam)
{
	struct thread_data * p_data;
	struct producer_consumer_params * p_params;
	//srand((unsigned int)time(NULL));
	p_data = (struct thread_data *)lparam;
	//srand((unsigned int)time(NULL));
	p_params = p_data->p_params_;
	for (;;)
	{
		DWORD dwResult = WaitForSingleObject(p_data->events_[0], p_params->consumer_delay_);
		if (WAIT_TIMEOUT == dwResult)
		{	
			if (fifo_circular_buffer_get_items_count(p_params->p_fifo_)>0)
			{
				struct data_item data_item;
				fifo_circular_buffer_fetch_item(p_params->p_fifo_, &data_item);
				//fifo_circular_buffer_pop_item(p_params->p_fifo_);
#if 0
				fprintf(stderr, "%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x\n",
						data_item.data_[0], data_item.data_[1], data_item.data_[2], data_item.data_[3],
						data_item.data_[4], data_item.data_[5], data_item.data_[6], data_item.data_[7]);
#endif
				++p_params->items_consumed_;
			}
		}
		else
		{
			break;
		}
	}
	SetEvent(p_data->events_[1]);
	_endthreadex(0);
	return 0;
}

static void test_006(void)
{
	HANDLE events[2];
	uintptr_t producer, consumer;
	struct thread_data prod_data, cons_data;
	struct producer_consumer_params param;
	memset(&param, 0, sizeof(struct producer_consumer_params));
	memset(&prod_data, 0, sizeof(struct thread_data));
	memset(&cons_data, 0, sizeof(struct thread_data));
	param.p_fifo_ = fifo_circular_buffer_create();
	param.items_ = 100;
	param.producer_delay_ = 10;
	param.consumer_delay_ = 1;
	prod_data.p_params_ = &param;
	cons_data.p_params_ = &param;
	events[0] = CreateEvent(NULL, TRUE, FALSE, "producer-0");
	events[1] = CreateEvent(NULL, TRUE, FALSE, "consumer-0");
	prod_data.events_ = events;
	cons_data.events_ = events;
	producer = _beginthreadex(NULL, 0, &producer_0_entry_point, &prod_data, 0, NULL);
	consumer = _beginthreadex(NULL, 0, &consumer_0_entry_point, &cons_data, 0, NULL);
	WaitForMultipleObjects(2, events, TRUE, INFINITE);
	fifo_circular_buffer_delete(param.p_fifo_);
}

static void test_007(void)
{
	HANDLE events[2];
	uintptr_t producer, consumer;
	struct thread_data prod_data, cons_data;
	struct producer_consumer_params param;
	param.p_fifo_ = fifo_circular_buffer_create();
	param.items_ = 100;
	param.producer_delay_ = 1;
	param.consumer_delay_ = 5;
	prod_data.p_params_ = &param;
	cons_data.p_params_ = &param;
	events[0] = CreateEvent(NULL, TRUE, FALSE, "producer-0");
	events[1] = CreateEvent(NULL, TRUE, FALSE, "consumer-0");
	prod_data.events_ = events;
	cons_data.events_ = events;
	producer = _beginthreadex(NULL, 0, &producer_0_entry_point, &prod_data, 0, NULL);
	consumer = _beginthreadex(NULL, 0, &consumer_0_entry_point, &cons_data, 0, NULL);
	WaitForMultipleObjects(2, events, TRUE, INFINITE);
	fifo_circular_buffer_delete(param.p_fifo_);
}

int main(int argc, char ** argv)
{
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

