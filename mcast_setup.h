/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file mcast_setup.h
 * @author T.Ostaszewski
 * @date 04-Jan-2012
 * @brief
 * @details
 */
#if !defined MCAST_SETUP_H_870702C0_B65B_4828_949C_490704388A28
#define MCAST_SETUP_H_870702C0_B65B_4828_949C_490704388A28

#include "pcc.h"

struct addrinfo;

/*!
 * @brief Describes the MCAST connection.
 */
struct mcast_connection {
	struct addrinfo * bindAddr_;
	struct addrinfo * resolveAddr_;
	struct addrinfo * multiAddr_;
	SOCKET socket_;
};

/*!
 * @brief
 */
struct mcast_connection * setup_multicast_2(char * p_multicast_addr, char * p_port);

/*!
 * @brief
 */
int setup_multicast_3(char * p_multicast_addr, char * p_port, struct mcast_connection * p_conn);

/*!
 * @brief
 */
int setup_multicast(struct mcast_connection * p_mcast_conn);

/*!
 * @brief
 */
int setup_mcast(struct conn_data * p_conn_data);

/*!
 * @brief
 */
int act_on_timer(struct conn_data * p_conn_data);

/*!
 * @brief
 */
int close_multicast(struct mcast_connection * p_mcast_conn);

#endif /*MCAST_SETUP_H_870702C0_B65B_4828_949C_490704388A28 */
