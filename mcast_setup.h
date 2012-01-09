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
#include "mcast-settings.h"

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
 * @brief Setup the multicast connection with given parameters.
 * @param[in] bConnect
 * @param[in] bReuseAddr
 * @param[in] bindAddr
 * @param[in] interfaceAddr
 * @param[in] nTTL
 * @param[in] p_multicast_addr IPv4 of the multicast group to connect, i.e. "224.5.6.7"
 * @param[in] p_port  port number on which data will be send/received.
 * @param[out] p_mcast_conn this memory location will be written with active multicast connection upon successful exit.
 * @return returns 0 on success, <>0 otherwise.
 */
int setup_multicast(BOOL bConnect, BOOL bReuseAddr, char * bindAddr, char * interfaceAddr, uint8_t nTTL, char * p_multicast_addr, char * p_port, struct mcast_connection * p_mcast_conn);

/*!
 * @brief Wrapper for the setup_multicast function with most of the parameters set to so called "reasonable defaults".
 * @details The so called "reasonable defaults" are:
 * \li bConnect set to FALSE - don't call connect() after joining a multicast group;
 * \li bReuseAddr set to TRUE so the address can be reused immediatelly without relaying on Windows to free that after some period of time;
 * \li bindAddr set to NULL to bind to any address
 * \li interfaceAddr set to NULL to bind to any interface
 * \li nTTL set to value 8
 * @param[in] p_multicast_addr IPv4 of the multicast group to connect, i.e. "224.5.6.7"
 * @param[in] p_port  port number on which data will be send/received.
 * @param[out] p_mcast_conn this memory location will be written with active multicast connection upon successful exit.
 * @return returns 0 on success, <>0 otherwise.
 * @sa setup_multicast
 */
int setup_multicast_default(char * p_multicast_addr, char * p_port, struct mcast_connection * p_mcast_conn);

/*!
 * @brief Setup the multicast connection with given parameters.
 * @param[in] p_settings contains all the multicast connection related settings.
 * @param[out] p_conn this memory location will be written with active multicast connection upon successful exit.
 * @return returns 0 on success, <>0 otherwise.
 */
int setup_multicast_indirect(struct mcast_settings const * p_settings, struct mcast_connection * p_conn);

/*!
 * @brief Leaves the multicast group, closes socket.
 * @param p_mcast_conn descriptor of the multicast connection obtained via setup_multicast_indirect, setup_mutlicast_default or setup_multicast call.
 * @return returns 0 on success, <>0 otherwise
 */
int close_multicast(struct mcast_connection * p_mcast_conn);

#endif /*MCAST_SETUP_H_870702C0_B65B_4828_949C_490704388A28 */
