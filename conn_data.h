/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file conn_data.h
 * @author
 * @date
 * @brief
 * @details
 */
#if !defined CONN_DATA_B39001CA_1BC2_48A8_839A_610C6BE95714
#define CONN_DATA_B39001CA_1BC2_48A8_839A_610C6BE95714

#include "winsock_adapter.h"
#include "hello.h"
#include "mcast_setup.h"

#define INPUT_BUFFER_SIZE (512)
#define MAX_CONNECTED_ENTITIES (8)

struct conn_table;

struct conn_data {
	struct hello_packet	my_hello_;
	struct timeval		next_hello_send_timeout_;
	struct timeval		next_purge_entry_timeout_;
	struct conn_table * p_conn_table_;
	struct mcast_connection connection_;
    uint8_t             buffer_[INPUT_BUFFER_SIZE];
	uint16_t			rec_data_len_;
    struct sockaddr_in  recvfrom_;
    socklen_t           recvfromlen_;
};

size_t send_hello(struct conn_data * p_data);
size_t recv_hello(struct conn_data * p_data, struct timeval const * p_recv_timeout);
int is_hello_mine(struct conn_data * p_data);

size_t update_missing(struct conn_data * p_data);
void update_new(struct conn_data * p_data, struct timeval const * p_purge_entry_timer_value);

#endif /* CONN_DATA_B39001CA_1BC2_48A8_839A_610C6BE95714 */
