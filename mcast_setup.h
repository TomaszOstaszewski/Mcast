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
 * @brief Configuration of the multicast connection.
 */
struct mcast_settings {
	BOOL bConnect_;
	BOOL bDontJoin_;
	BOOL bReuseAddr_;
	int nProtocol_;
	int nLoopback_;
	int nTTL_;
	char * mcast_addr_;
	char * mcast_port_;
    char * bindAddr_;
    char * interface_;
};

/*!
 * @brief Setup the multicast connection with given parameters.
 * @param[in] bConnect
 * @param[in] bReuseAddr
 * @param[in] nTTL
 * @param[in] p_multicast_addr IPv4 of the multicast group to connect, i.e. "224.5.6.7"
 * @param[in] p_port  port number on which data will be send/received.
 * @param[out] p_mcast_conn this memory location will be written with active multicast connection upon successful exit.
 * @return returns 0 on success, <>0 otherwise.
 */
int setup_multicast(BOOL bConnect, BOOL bReuseAddr, char * bindAddr, uint8_t nTTL, char * p_multicast_addr, char * p_port, struct mcast_connection * p_mcast_conn);

/*!
 * @brief Setup the multicast connection with given parameters.
 * @param[in] p_multicast_addr IPv4 of the multicast group to connect, i.e. "224.5.6.7"
 * @param[in] p_port  port number on which data will be send/received.
 * @param[out] p_mcast_conn this memory location will be written with active multicast connection upon successful exit.
 * @return returns 0 on success, <>0 otherwise.
 * @sa setup_multicast
 */
int setup_multicast_default(char * p_multicast_addr, char * p_port, struct mcast_connection * p_mcast_conn);

/*!
 * @brief Setup the multicast connection with given parameters.
 * @param[in] p_multicast_addr IPv4 of the multicast group to connect, i.e. "224.5.6.7"
 * @param[in] p_port  port number on which data will be send/received.
 * @param[out] p_conn this memory location will be written with active multicast connection upon successful exit.
 * @return returns 0 on success, <>0 otherwise.
 */
int setup_multicast_indirect(char * p_multicast_addr, char * p_port, struct mcast_connection * p_conn);


/*!
 * @brief
 */
int close_multicast(struct mcast_connection * p_mcast_conn);

#endif /*MCAST_SETUP_H_870702C0_B65B_4828_949C_490704388A28 */
