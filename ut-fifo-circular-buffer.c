#include "pcc.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <process.h>
#include <assert.h>
#include <time.h>
#include "fifo-circular-buffer.h"

static uint8_t  g_template_buffer[256];

static void test_000(void)
{
    struct fifo_circular_buffer * p_circular_buffer = fifo_circular_buffer_create();
    assert( NULL != p_circular_buffer);
    fifo_circular_buffer_delete(p_circular_buffer);
}

static void test_001(void)
{
    unsigned int is_free;
    unsigned int items_count;
    struct fifo_circular_buffer * p_circular_buffer = fifo_circular_buffer_create();
    assert( NULL != p_circular_buffer);
    items_count = fifo_circular_buffer_get_items_count(p_circular_buffer);
    assert(0==items_count);
    is_free = fifo_circular_buffer_is_free_space(p_circular_buffer);
    assert(is_free);
    fifo_circular_buffer_delete(p_circular_buffer);
}

/**
 * @test Test queue normal operation (no overflow)
 */
static void test_002(void)
{
	int result;
	size_t idx;
	unsigned int is_free;
	unsigned int items_count;
	struct fifo_circular_buffer * p_circular_buffer;
	struct data_item an_item;
	struct data_item * items[256];
	struct data_item  a_rec_item;
	uint8_t const * p_idx; 
	uint16_t page_count;

	an_item.count_= sizeof(g_template_buffer)/sizeof(g_template_buffer[0]);
	an_item.data_ = g_template_buffer;

	for (idx = 0; idx < sizeof(items)/sizeof(items[0]); ++idx)
	{
		items[idx] = &an_item;
	}

	p_circular_buffer = fifo_circular_buffer_create();
	assert( NULL != p_circular_buffer);
	items_count = fifo_circular_buffer_get_items_count(p_circular_buffer);
	assert(0==items_count);
	is_free = fifo_circular_buffer_is_free_space(p_circular_buffer);
	assert(is_free);
	/* Insert 65536 bytes of data in 256 chunks of 256 bytes each */
	for (idx = 0; idx < sizeof(items)/sizeof(items[0]); ++idx)
	{
		result = fifo_circular_buffer_push_item(p_circular_buffer, items[idx]);
		assert(0 == result);
	}
	items_count = fifo_circular_buffer_get_items_count(p_circular_buffer);
	assert(0x10000==items_count);
	/* Retrive data */
	{
		a_rec_item.data_ = (uint8_t *)malloc(sizeof(uint8_t)*items_count);
		a_rec_item.count_ = items_count;
		result = fifo_circular_buffer_fetch_item(p_circular_buffer, &a_rec_item);
		items_count = fifo_circular_buffer_get_items_count(p_circular_buffer);
		printf("%s %5.5d : %u\n", __FILE__, __LINE__, items_count);
		assert(0 == items_count);
		/* Check if what we got is what we put into */
		p_idx = a_rec_item.data_; 
		for (page_count = 0; page_count != 256; ++page_count, p_idx += 256)
		{
			printf("%s %5.5d : %6.6d %2.2hhx%2.2hhx%2.2hhx..  %2.2hhx%2.2hhx%2.2hhx\n",
					__FILE__, __LINE__,
					page_count,
					p_idx[0], p_idx[1], p_idx[2],
					g_template_buffer[0], g_template_buffer[1], g_template_buffer[2]);
			assert(0 == memcmp(p_idx, g_template_buffer, sizeof(g_template_buffer)));
		}
	}
	fifo_circular_buffer_delete(p_circular_buffer);
}

/**
 * @test Test queue overflow conditions.
 */
static void test_003(void)
{
	int result;
	size_t idx;
	unsigned int is_free;
	unsigned int items_count;
	struct fifo_circular_buffer * p_circular_buffer;
	struct data_item an_item;
	struct data_item * items[256];
	uint8_t zero_data[256];
	struct data_item  a_rec_item;
	uint16_t page_count;
	an_item.count_ = sizeof(g_template_buffer)/sizeof(g_template_buffer[0]);
	an_item.data_ = g_template_buffer;
	for (idx = 0; idx < sizeof(items)/sizeof(items[0]); ++idx)
	{
		items[idx] = &an_item;
	}
	p_circular_buffer = fifo_circular_buffer_create();
	assert( NULL != p_circular_buffer);
	items_count = fifo_circular_buffer_get_items_count(p_circular_buffer);
	assert(0==items_count);
	is_free = fifo_circular_buffer_is_free_space(p_circular_buffer);
	assert(is_free);
	/* Insert 65536+256 bytes of data in 256 chunks of 256 bytes each */
	for (idx = 0; idx < sizeof(items)/sizeof(items[0]); ++idx)
	{
		result = fifo_circular_buffer_push_item(p_circular_buffer, items[idx]);
		assert(0 == result);
	}
	memset(zero_data, 0, sizeof(zero_data));
	an_item.count_ = sizeof(zero_data);
	an_item.data_ = &zero_data[0];
	result = fifo_circular_buffer_push_item(p_circular_buffer, &an_item);
	assert(0 == result);

	items_count = fifo_circular_buffer_get_items_count(p_circular_buffer);
	assert(0x10000==items_count);
	/* Retrive data */
	a_rec_item.data_ = (uint8_t *)malloc(sizeof(g_template_buffer));
	a_rec_item.count_ = sizeof(g_template_buffer);
	/* First 255 pages will be the very same as template data */
	for (page_count = 0; page_count != 255; ++page_count)
	{
		a_rec_item.data_ = (uint8_t *)malloc(sizeof(uint8_t)*items_count);
		a_rec_item.count_ = 256;
		result = fifo_circular_buffer_fetch_item(p_circular_buffer, &a_rec_item);
		printf("%s %5.5d : %6.6d %2.2hhx%2.2hhx%2.2hhx..  %2.2hhx%2.2hhx%2.2hhx\n",
				__FILE__, __LINE__,
				page_count,
				a_rec_item.data_[0], a_rec_item.data_[1], a_rec_item.data_[2],
				g_template_buffer[0], g_template_buffer[1], g_template_buffer[2]);
		assert(0 == memcmp(a_rec_item.data_, g_template_buffer, sizeof(g_template_buffer)));
	}
	/* Last page shall be all zeros */
	items_count = fifo_circular_buffer_get_items_count(p_circular_buffer);
	printf("%s %5.5d : %u\n", __FILE__, __LINE__, items_count);
	assert(256 == items_count);
	result = fifo_circular_buffer_fetch_item(p_circular_buffer, &a_rec_item);
	printf("%s %5.5d : %6.6d %2.2hhx%2.2hhx%2.2hhx..  %2.2hhx%2.2hhx%2.2hhx\n",
			__FILE__, __LINE__,
			page_count,
			a_rec_item.data_[0], a_rec_item.data_[1], a_rec_item.data_[2],
			zero_data[0], zero_data[1], zero_data[2]);
	assert(0 == memcmp(a_rec_item.data_, zero_data, sizeof(zero_data)));
	items_count = fifo_circular_buffer_get_items_count(p_circular_buffer);
	assert(0 == items_count);
	fifo_circular_buffer_delete(p_circular_buffer);
}

static void test_004(void)
{
}

/**
 * @brief Add 128k items and see if it still works the same.
 * @details 
 */
static void test_005(void)
{
}

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

#define SHR_BYTE(value,shift) ((uint8_t)(value>>shift))
#define SHL_BYTE(value,shift) ((uint8_t)(value<<shift))

int main(int argc, char ** argv)
{
	uint16_t idx; 
	for (idx = 0; idx != 256; ++idx)
	{
		g_template_buffer[idx] = (uint8_t)(idx & 0xff);
	}
#if 0
	uint8_t ubase = (uint8_t)(-1);
	int8_t sbase = (int8_t)-1;
	int8_t sbase2 = (int8_t)1;
	printf("%2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", SHL_BYTE(ubase,0), SHL_BYTE(ubase,1), SHL_BYTE(ubase,2), SHL_BYTE(ubase,3));
	printf("%2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", SHR_BYTE(ubase,0), SHR_BYTE(ubase,1), SHR_BYTE(ubase,2), SHR_BYTE(ubase,3));
	printf("%2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", SHL_BYTE(sbase,0), SHL_BYTE(sbase,1), SHL_BYTE(sbase,2), SHL_BYTE(sbase,3));
	printf("%2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", SHR_BYTE(sbase,0), SHR_BYTE(sbase,1), SHR_BYTE(sbase,2), SHR_BYTE(sbase,3));
	printf("%2.2hhx %2.2hhx %2.2hhx %2.2hhx\n", SHL_BYTE(sbase2,0), SHL_BYTE(sbase2,1), SHL_BYTE(sbase2,2), SHL_BYTE(sbase2,3));
#endif
    test_000();
    test_001();
    test_002();
    test_003();
    //test_004();
    //test_005();
    //test_006();
    //test_007();
    return 0;
}

