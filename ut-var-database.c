/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file ut-var-database.c
 * @author
 * @date
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
#include "dsoundplay.h"
#include "var-database.h"

void fifo_circular_buffer_delete(struct fifo_circular_buffer * p_circular_buffer)
{
}

void dsoundplayer_destroy(DSOUNDPLAY handle)
{
}

/*!
 * @test
 */
static void test_000(void)
{
	struct var_database * p_database;
	p_database = var_database_create();	
	assert(NULL != p_database);
	var_database_destroy(p_database);
}

/*!
 * @test
 */
static void test_001(void)
{
	struct var_database *p_database0, *p_database1;
    void *p_data0, *p_data1;
    int result;

	p_database0 = var_database_create();	
	assert(NULL != p_database0);
	p_database1 = var_database_create();	
	assert(NULL != p_database1);
    result = set_var(p_database0, GLOBAL_MCAST_CONNECTION, (void*)0xcafebabe, NULL);
    assert(0 == result);
    p_data0 = get_var(p_database0, GLOBAL_MCAST_CONNECTION);
    p_data1 = get_var(p_database1, GLOBAL_MCAST_CONNECTION);
    assert(NULL == p_data1);
    assert((void*)0xcafebabe == p_data0);
    result = set_var(p_database1, GLOBAL_MCAST_CONNECTION, (void*)0xdeadbeef, NULL);
    assert(0 == result);
    p_data0 = get_var(p_database0, GLOBAL_MCAST_CONNECTION);
    assert((void*)0xcafebabe == p_data0);
    p_data1 = get_var(p_database1, GLOBAL_MCAST_CONNECTION);
    assert((void*)0xdeadbeef == p_data1);
	var_database_destroy(p_database1);
	var_database_destroy(p_database0);
}

/*!
 * @test
 */
static void test_002(void)
{
}

/*!
 * @test
 */
static void test_003(void)
{
}

/*!
 * @test
 */
static void test_004(void)
{
}

/*!
 * @test
 */
static void test_005(void)
{
}

/*!
 * @test
 */
static void test_006(void)
{
}

/*!
 * @test
 */
static void test_007(void)
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
    test_006();
    test_007();
    return 0;
}

