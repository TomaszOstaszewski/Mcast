/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file ut-input-buffer.c
 * @author T.Ostaszewski
 * @date 04-Jan-2012
 * @brief
 * @details
 */

#include "pcc.h"
#include <string.h>
#include <process.h>
#include <assert.h>
#include <time.h>
#include "input-buffer.h"

static void test_000(void)
{
    uint8_t input[] = { 
		0, 1, 2, 
        3, 4, 5, 
        6, 7 
	};
    uint8_t output[] = { 0, 0, 0 };
    struct buffer_desc   an_input_desc   = { input, 0, sizeof(input)/sizeof(input[0]) };
    struct buffer_desc  an_out_desc     = { output, 0, sizeof(output)/sizeof(output[0]) };
    copy_buffer(&an_out_desc, &an_input_desc, 3);
    fprintf(stderr, "%s %d : %u %u\n", __FILE__, __LINE__, an_input_desc.nCurrentOffset_, an_input_desc.nMaxOffset_);
    assert(an_input_desc.nCurrentOffset_<= an_input_desc.nMaxOffset_);
    fprintf(stderr, "%s %d : %2.2x%2.2x%2.2x %2.2x%2.2x%2.2x\n", __FILE__, __LINE__,
           input[0], input[1], input[2],
           output[0], output[1], output[2]
           ); 
	assert(0==memcmp(output, &input[0], 3));
	an_out_desc.nCurrentOffset_ = 0;
    copy_buffer(&an_out_desc,&an_input_desc,  3);
    fprintf(stderr, "%s %d : %u %u\n", __FILE__, __LINE__, an_input_desc.nCurrentOffset_, an_input_desc.nMaxOffset_);
    assert(an_input_desc.nCurrentOffset_<= an_input_desc.nMaxOffset_);
    fprintf(stderr, "%s %d : %2.2x%2.2x%2.2x %2.2x%2.2x%2.2x\n", __FILE__, __LINE__,
           input[3], input[4], input[5],
           output[0], output[1], output[2]
           ); 
    assert(0==memcmp(output, &input[3], 3));

	an_out_desc.nCurrentOffset_ = 0;
    copy_buffer(&an_out_desc, &an_input_desc,  3);
    fprintf(stderr, "%s %d : %u %u\n", __FILE__, __LINE__, an_input_desc.nCurrentOffset_, an_input_desc.nMaxOffset_);
    assert(an_input_desc.nCurrentOffset_<= an_input_desc.nMaxOffset_);
    fprintf(stderr, "%s %d : %2.2x%2.2x %2.2x%2.2x\n", __FILE__, __LINE__,
           input[6], input[7], 
           output[0], output[1]
           ); 
     assert(0==memcmp(output, &input[6], 2));
}

static void test_001(void)
{
    uint8_t input[] = { 0, 1, 2, 3 };
    uint8_t output[] = { 0, 0, 0 };
    struct buffer_desc   an_input_desc   = { input, 0, sizeof(input)/sizeof(input[0]) - 1 };
    struct buffer_desc  an_out_desc     = { output, 0, sizeof(output)/sizeof(output[0]) - 1 };
    fprintf(stderr, "%s %d : %u %u\n", __FILE__, __LINE__, an_input_desc.nCurrentOffset_, an_input_desc.nMaxOffset_);

    copy_buffer(&an_out_desc, &an_input_desc, 5);

    fprintf(stderr, "%s %d : %u %u\n", __FILE__, __LINE__, an_input_desc.nCurrentOffset_, an_input_desc.nMaxOffset_);
    assert(an_input_desc.nCurrentOffset_<= an_input_desc.nMaxOffset_);

    fprintf(stderr, "%s %d : %2.2x%2.2x%2.2x %2.2x%2.2x%2.2x\n", __FILE__, __LINE__,
           input[0], input[1], input[2], 
           output[0], output[1], output[2]); 
    assert(0==memcmp(output, &input[0], 3));
}

static void test_002(void)
{
}

static void test_003(void)
{
}

static void test_004(void)
{
}

static void test_005(void)
{
}

int main(int argc, char ** argv)
{
    test_000();
    test_001();
    test_002();
    test_003();
    test_004();
    test_005();
    return 0;
}

