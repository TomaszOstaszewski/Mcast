/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file conn_table.c
 * @author T. Ostaszewski
 * @date 04-Jan-2012
 * @brief 
 * @details 
 */
#include "winsock_adapter.h"
#if defined WIN32
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
#endif
#include "conn_table.h"

struct conn_entry_wrapper {
	uint8_t used_;
	struct conn_entry entry_;
};

#define CONN_ENTRY_MAX_ITEMS (16)

struct conn_table {
	struct conn_entry_wrapper table_[CONN_ENTRY_MAX_ITEMS];
};

static struct conn_table g_data;

struct conn_table * conn_table_create(void)
{
	return &g_data;
}

size_t find_by_tei(struct conn_table * p_data, struct tei const * p_key)
{
    size_t idx;
    for (idx = 0; idx < CONN_ENTRY_MAX_ITEMS; ++idx)
        if (p_data->table_[idx].used_ && 0 == memcmp(p_key, &p_data->table_[idx].entry_.tei_, sizeof(struct tei)))
            return idx;
    return (size_t)-1;
}

size_t find_by_id(struct conn_table * p_data, const char * id)
{
    size_t idx;
    for (idx = 0; idx < CONN_ENTRY_MAX_ITEMS; ++idx)
        if (p_data->table_[idx].used_ && 0 == strcmp(id, p_data->table_[idx].entry_.id_))
            return idx;
    return (size_t)-1;
}

size_t find_by_address(struct conn_table * p_data, struct sockaddr_in const * p_key)
{
    size_t idx;
    for (idx = 0; idx < CONN_ENTRY_MAX_ITEMS; ++idx)
        if (p_data->table_[idx].used_ && 0 == memcmp(p_key, &p_data->table_[idx].entry_.address_, sizeof(struct tei)))
            return idx;
    return (size_t)-1;
}

size_t find_first_unused(struct conn_table * p_data)
{
    size_t idx;
    for (idx = 0; idx < CONN_ENTRY_MAX_ITEMS; ++idx)
        if (0 == p_data->table_[idx].used_) 
            return idx;
    return (size_t)-1;
}

static int is_entry_timed_out(struct conn_entry * p_entry, struct timeval const * p_timeout)
{
	if (timercmp(p_timeout,&p_entry->purge_timeout_, >))
            return 1;
	return 0; 
}

size_t find_first_timedout(struct conn_table * p_data, struct timeval const * p_timeout)
{
    size_t idx;
    for (idx = 0; idx < CONN_ENTRY_MAX_ITEMS; ++idx)
        if (p_data->table_[idx].used_ && is_entry_timed_out(&p_data->table_[idx].entry_, p_timeout))
            return idx;
    return (size_t)-1;
}

/*
void purge_entry(struct conn_table * p_data, struct conn_entry * p_entry);
{
	return &p_data->table_[idx].entry_;
    p_entry->used_ = 0;    
}
*/

size_t purge_all_timedout(struct conn_table * p_data, struct timeval const * p_timeout)
{
    struct conn_entry_wrapper * p_entry = &p_data->table_[0];
    struct conn_entry_wrapper const * const p_past_last = &p_data->table_[CONN_ENTRY_MAX_ITEMS];
	size_t purged_count = 0;
    for (; p_past_last != p_entry; ++p_entry)
    {
		if (is_entry_timed_out(&p_entry->entry_, p_timeout))
		{
			p_entry->used_ = 0;	
    		++purged_count;
		}
    }
    return purged_count;
}

void foreach_entry(struct conn_table * p_data, P_CONN_ENTRY_ACTION action)
{
    struct conn_entry_wrapper * p_entry = &p_data->table_[0];
    struct conn_entry_wrapper const * const p_past_last = &p_data->table_[CONN_ENTRY_MAX_ITEMS];
    for (; p_past_last != p_entry; ++p_entry)
    {
		if (p_entry->used_)
			(*action)(&p_entry->entry_);
    }
}

void mark_used(struct conn_table * p_data, size_t idx)
{
	p_data->table_[idx].used_ = 1;
}

struct conn_entry * get_item(struct conn_table * p_data, size_t idx)
{
	return &p_data->table_[idx].entry_;
}

