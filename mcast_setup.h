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
	struct addrinfo * bindAddr_; /*!< */
	struct addrinfo * resolveAddr_; /*!< */
	struct addrinfo * multiAddr_; /*!< */
	SOCKET socket_; /*!< */
};

/*!
 * @brief
 * @param[in]
 * @param[in]
 * @param[out]
 * @return
 */
struct mcast_connection * setup_multicast_2(char * p_multicast_addr, char * p_port);

/*!
 * @brief Setup the multicast connection with given parameters.
 * @param[in] p_multicast_addr IPv4 of the multicast group to connect, i.e. "224.5.6.7"
 * @param[in] p_port  port number on which data will be send/received.
 * @param[out] p_conn this memory location will be written with active multicast connection upon successful exit.
 * @return returns 0 on success, <>0 otherwise.
 */
int setup_multicast_3(char * p_multicast_addr, char * p_port, struct mcast_connection * p_conn);

/*!
 * @brief Setup the multicast connection with default parameters.
 * @param[in,out] p_mcast_conn
 * @return 
 */
int setup_multicast(struct mcast_connection * p_mcast_conn);

/*!
 * @brief
 * @param[in,out] p_conn_data
 * @return
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
