/*!
 * @brief
 * @file mcast-settings.h
 * @author T.Ostaszewski
 * @date Jan-2012
 */
#if !defined MCAST_SETTINGS_4220923E_A4E4_4081_8291_4D05B9D799B1
#define MCAST_SETTINGS_4220923E_A4E4_4081_8291_4D05B9D799B1

#if defined __cplusplus
extern "C" {
#endif

/*!
 * @brief Configuration of the multicast connection.
 */
struct mcast_settings {
	int bConnect_; /*!< Whether or not call connect() on the socket bound to the multicast group */
	int bReuseAddr_; /*!< Whether or not reuse address.  */
    char * bindAddr_; /*!< Name of the interface to bind to */
    char * interface_; /*!< Name of the interface to bind to */
	int nTTL_; /*!< The 'Time To Live' parameter to set on the socket. */
	char * mcast_addr_; /*!< The address of the multicast group */
	char * mcast_port_; /*!< The port number on which communication will be performed */
};

#if defined __cplusplus
}
#endif

#endif /* MCAST_SETTINGS_4220923E_A4E4_4081_8291_4D05B9D799B1 */

