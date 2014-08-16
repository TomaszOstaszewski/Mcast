/**
 * @file debug-ipv4.h
 * @brief Header file for IPv4 debug helpers.
 * @details 
 * @author T.Ostaszewski
 * @date 2014-Aug-16
 * @par History
 * <pre>
 * </pre>
 * @copyright
 */

 /**
 * @brief
 * @details
 * @param ppBuffer
 * @param *pBufferSize
 * @param pSockAddrIn
 */
void dumpSockAddrIn(char** ppBuffer, size_t* pBufferSize, const struct sockaddr_in* pSockAddrIn);

/**
 * @brief
 * @details
 * @param pBuffer
 * @param *pBufferSzie
 * @param pSockAddr
 */
void dumpSockAddr(char** ppBuffer, size_t* pBufferSize, const struct sockaddr* pSockAddr,
                  socklen_t nSockAddrLen);

/**
 * @brief
 * @details
 * @param pBuffer
 * @param *pBufferSize
 * @param pInfo
 */
void dumpAddrInfo(char** pBuffer, size_t* pBufferSize, struct addrinfo const* pInfo);

