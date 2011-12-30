#include "conn_data.h"
#include "conn_table.h"

int is_hello_mine(struct conn_data * p_data)
{
	if (p_data->rec_data_len_ == sizeof(struct hello_packet) && 0 == memcmp(p_data->buffer_, &p_data->my_hello_, sizeof(struct hello_packet)))
	{
		return 1;
	}
	return 0;
}

size_t send_hello(struct conn_data * p_data)
{
	/* Here send hello packet again */
	return sendto(
			(SOCKET)p_data->connection_.socket_, 
			(const char *)&p_data->my_hello_, 
			sizeof(struct hello_packet),
			0,
			p_data->connection_.multiAddr_->ai_addr,
			(int) p_data->connection_.multiAddr_->ai_addrlen
			);
}

/**
 * \brief Update neighbours table - remove those that were missing for too long 
 */
size_t update_missing(struct conn_data * p_data)
{
	size_t purged_count;
	struct timeval now;
	gettimeofday(&now, NULL);
	purged_count = purge_all_timedout(p_data->p_conn_table_, &now);
	return purged_count;
}

static int is_new_data(struct conn_data * p_conn_data, struct timeval * p_timeout)
{
	int nfds;
	fd_set readfs;

	nfds = (SOCKET)p_conn_data->connection_.socket_ + 1;
	FD_ZERO(&readfs);
	FD_SET((SOCKET)p_conn_data->connection_.socket_, &readfs);
	return select(nfds, &readfs, NULL, NULL, p_timeout);
}

size_t recv_hello(struct conn_data * p_data, struct timeval const * p_recv_timeout)
{
	struct timeval now;
	int rc;
	gettimeofday(&now, NULL);
	/* Update neighbours table - add or update those which report that are present */
	memcpy(&now, p_recv_timeout, sizeof(struct timeval));
	rc = is_new_data(p_data, &now);
	if (rc>0)
	{
		/* Read data */
		p_data->recvfromlen_ = sizeof(struct sockaddr_in);
		p_data->rec_data_len_ = recvfrom(
				(SOCKET)p_data->connection_.socket_, 
				p_data->buffer_,
				INPUT_BUFFER_SIZE,0,
				(SOCKADDR *)&p_data->recvfrom_, 
				&p_data->recvfromlen_);
		return p_data->rec_data_len_;
	}
	return 0;
}

void update_new(struct conn_data * p_data, struct timeval const * p_purge_entry_timer_value)
{
	/* Update neighbours table - add or update those which report that are present */
	size_t idx;
	struct hello_packet const * p_packet = (struct hello_packet const * )p_data->buffer_;
	struct conn_entry * p_entry;
	idx = find_by_tei(p_data->p_conn_table_, &p_packet->tei_);
	if ((size_t)-1 == idx)
	{
		/* Update entry */
		idx = find_first_unused(p_data->p_conn_table_);
	}
	p_entry = get_item(p_data->p_conn_table_, idx);
	memcpy(&p_entry->tei_, &p_packet->tei_, sizeof(struct tei));
	p_entry->type_ = p_packet->type_;
	mark_used(p_data->p_conn_table_, idx);
	/* Set this entry to expire at 'purge_entry_timer_value' from now */
	gettimeofday(&p_entry->purge_timeout_, NULL);
	timeradd(&p_entry->purge_timeout_, p_purge_entry_timer_value, &p_entry->purge_timeout_);
}

