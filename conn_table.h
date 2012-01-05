/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file conn_table.h
 * @author T.Ostaszewski
 * @date 04-Jan-2012
 * @brief
 * @details
 */
#if !defined CONN_TABLE_H
#define CONN_TABLE_H

#include "winsock_adapter.h"
/* For struct timeval */
#if !defined WIN32
#include <time.h>
#endif

#include "conn_entry.h"

struct sockaddr_in;
struct timeval;
struct conn_table;

struct conn_table * conn_table_create(void);

size_t find_by_id(struct conn_table * p_data, const char * id);
size_t find_by_tei(struct conn_table * p_data, struct tei const * p_key);
size_t find_by_address(struct conn_table * p_data, struct sockaddr_in const * p_key);
size_t find_first_unused(struct conn_table * p_data);
size_t find_first_timedout(struct conn_table * p_data, struct timeval const * p_timeout);

struct conn_entry * get_item(struct conn_table * p_data, size_t idx);
void mark_used(struct conn_table * p_data, size_t idx);

typedef void (*P_CONN_ENTRY_ACTION)(struct conn_entry * p_entry);

void foreach_entry(struct conn_table * p_data, P_CONN_ENTRY_ACTION action);

size_t purge_all_timedout(struct conn_table * p_data, struct timeval const * p_timeout);

#endif /* defined CONN_TABLE_H */
