/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file resolve.h
 * @brief This file contains function prototypes for resolve.cpp
 * @details These are common routines for resolving and printing IPv4 and IPv6 addresses.
 * @date 04-Jan-2011
 */

#ifndef _RESOLVE_H_
#define _RESOLVE_H_

#ifdef _cplusplus
extern "C" {
#endif

struct sockaddr;
struct sockaddr_in;
struct sockaddr_in6; /* This one is from "ws2tcpip.h" */
struct addrinfo;

/**
 * @brief A wrapper for <a href="http://bit.ly/wQasV3">getnameinfo()</a> routine.
 * @details Just like <a href="http://bit.ly/wQasV3">getnameinfo()</a>, the FormatAddress 
 * function is the inverse of <a href="http://bit.ly/A81NVs">getaddrinfo()</a>: 
 * it converts a socket address to a corresponding host and service, in a protocol-independent manner.
 * It combines the functionality of gethostbyaddr and getservbyport,
 * but unlike those functions, this function is reentrant and allows programs to eliminate IPv4-versus-IPv6 dependencies.
 * The argument addrbuf is a pointer to caller-allocated buffer (of size addrbuflen) in which FormatAddress 
 * places null-terminated string containing the host and service name.
 * @param[in] sa A pointer to a generic socket address structure (of type  sockaddr_in  or  sockaddr_in6) 
 * of size salen that holds the input IP address and port number. 
 * @param[in] salen length of the SOCKADDR structure.
 * @param[in] addrbuf The pointer to caller-allocated buffer (of size addrbuflen) into which getnameinfo() 
 * places null-terminated strings containing the host and service name.
 * @param[in] addrbuflen length of the buffer given as addrbuf parameter.
 * @return returns 0 on success, <>0 on failure
 * @attention If the above hyperlinks to external pages fail, just use a internet search engine of your choice
 * to find (or "google up") getaddrinfo and getnameinfo function descriptions.
 */
int FormatAddress(struct sockaddr *sa, int salen, char *addrbuf, int addrbuflen);

/**
 * @brief A wrapper for <a href="http://bit.ly/A81NVs">getaddrinfo()</a> function.
 * @details Just like <a href="http://bit.ly/A81NVs">getaddrinfo()</a> function, this function, given node and service, 
 * which identify an Internet host and a service, returns one or more addrinfo structures, 
 * each of which contains an Internet address that can be specified in a call to bind or connect.  
 * The getaddrinfo function combines the functionality provided by the getservbyname and getservbyport functions
 * into a single interface, but unlike the latter functions, this function is reentrant and allows programs to eliminate
 * IPv4-versus-IPv6 dependencies.
 * This routine resolves the specified address and returns a list of addrinfo
 * structure containing SOCKADDR structures representing the resolved addresses.
 * Note that if 'addr' is non-NULL, then this routine will resolve it whether
 * it is a string listeral address or a hostname.
 * @param[in] addr Null terminated string that contains the IPv4 or IPv6 address, i.e. "192.168.0.1".
 * @param[in] port Null terminated string that contains the IP port, i.e. "4242".
 * @param[in] af This field specifies the desired address family for the returned addresses. 
 * Valid values for this field include <b>AF_INET</b> and <b>AF_INET6</b>. The value <b>AF_UNSPEC</b> indicates that the function should return socket addresses 
 * for any address family (either IPv4 or IPv6, for example) that can be used with node and service. 
 * @param[in] type This field specifies the preferred socket type, for example <b>SOCK_STREAM</b> or <b>SOCK_DGRAM</b>. 
 * Specifying 0 in this field indicates that socket addresses of any type can be returned by the function. 
 * @param[in] proto This field specifies the protocol for the returned socket addresses. 
 * Specifying 0 in this field indicates that socket addresses with any protocol can be returned. 
 * @return returns a pointer to the addrinfo structure. 
 * The caller is responsible for calling <a hre="http://bit.ly/tOD5Ts">freeaddrinfo()</a> when he is done with the value returned to free resources occupied by that structure.
 * On failure returns NULL.
 * @attention If the above hyperlinks to external pages fail, just use a internet search engine of your choice
 * to find (or "google up") getaddrinfo function description.
 */
struct addrinfo *ResolveAddress(char *addr, char *port, int af, int type, int proto);

/**
 * @brief Another wariant of ResolveAddress, but takes the flags parameter.
 * @details Just like <a href="http://bit.ly/A81NVs">getaddrinfo()</a> function, this function, given node and service, 
 * which identify an Internet host and a service, returns one or more addrinfo structures, 
 * each of which contains an Internet address that can be specified in a call to bind or connect.  
 * The getaddrinfo function combines the functionality provided by the getservbyname and getservbyport functions
 * into a single interface, but unlike the latter functions, this function is reentrant and allows programs to eliminate
 * IPv4-versus-IPv6 dependencies.
 * This routine resolves the specified address and returns a list of addrinfo
 * structure containing SOCKADDR structures representing the resolved addresses.
 * Note that if 'addr' is non-NULL, then this routine will resolve it whether
 * it is a string listeral address or a hostname.
 * @param[in] addr Null terminated string that contains the IPv4 or IPv6 address, i.e. "192.168.0.1".
 * @param[in] port Null terminated string that contains the IP port, i.e. "4242".
 * @param[in] af This field specifies the desired address family for the returned addresses. 
 * Valid values for this field include <b>AF_INET</b> and <b>AF_INET6</b>. The value <b>AF_UNSPEC</b> indicates that the function should return socket addresses 
 * for any address family (either IPv4 or IPv6, for example) that can be used with node and service. 
 * @param[in] type This field specifies the preferred socket type, for example <b>SOCK_STREAM</b> or <b>SOCK_DGRAM</b>. 
 * Specifying 0 in this field indicates that socket addresses of any type can be returned by the function. 
 * @param[in] proto This field specifies the protocol for the returned socket addresses. 
 * Specifying 0 in this field indicates that socket addresses with any protocol can be returned. 
 * @param[in] flags Either 0 or one or more of the following values (ORed together):
 * \li <b>AI_PASSIVE</b> indicatest that socket address will be used in bind() call
 * \li <b>AI_CANONNAME</b> indicates to return canonical name in first ai_canonname
 * \li <b>AI_NUMERICHOST</b> indicates that node name must be a numeric address string
 * @return returns a pointer to the addrinfo structure. 
 * The caller is responsible for calling <a hre="http://bit.ly/tOD5Ts">freeaddrinfo()</a> when he is done with the value returned to free resources occupied by that structure.
 * On failure returns NULL.
 * @attention If the above hyperlinks to external pages fail, just use a internet search engine of your choice
 * to find (or "google up") getaddrinfo function description.
 */
struct addrinfo *ResolveAddressWithFlags(char *addr, char *port, int af, int type, int proto, int flags);

/**
 * @brief Another wrapper for <a href="http://bit.ly/A81NVs">getaddrinfo()</a> function.
 * @details Just like <a href="http://bit.ly/A81NVs">getaddrinfo()</a> function, this function, given node and service, 
 * which identify an Internet host and a service, returns one or more addrinfo structures, 
 * each of which contains an Internet address that can be specified in a call to bind or connect.  
 * The getaddrinfo function combines the functionality provided by the getservbyname and getservbyport functions
 * into a single interface, but unlike the latter functions, this function is reentrant and allows programs to eliminate
 * IPv4-versus-IPv6 dependencies.
 * This routine resolves the specified address and returns a list of addrinfo
 * structure containing SOCKADDR structures representing the resolved addresses.
 * Note that if 'addr' is non-NULL, then this routine will resolve it whether
 * it is a string listeral address or a hostname.
 * @param[in] p_in_addr Pointer to the sockaddr_in structure, that contains IPv4 address and port requested.
 * @param[in] type This field specifies the preferred socket type, for example <b>SOCK_STREAM</b> or <b>SOCK_DGRAM</b>. 
 * Specifying 0 in this field indicates that socket addresses of any type can be returned by the function. 
 * @param[in] proto This field specifies the protocol for the returned socket addresses. 
 * Specifying 0 in this field indicates that socket addresses with any protocol can be returned. 
 * @param[in] flags Either 0 or one or more of the following values (ORed together):
 * \li <b>AI_PASSIVE</b> indicatest that socket address will be used in bind() call
 * \li <b>AI_CANONNAME</b> indicates to return canonical name in first ai_canonname
 * \li <b>AI_NUMERICHOST</b> indicates that node name must be a numeric address string
 * @return returns a pointer to the addrinfo structure. 
 * The caller is responsible for calling <a hre="http://bit.ly/tOD5Ts">freeaddrinfo()</a> when he is done with the value returned to free resources occupied by that structure.
 * @return returns a pointer to the addrinfo structure. 
 * @attention If the above hyperlinks to external pages fail, just use a internet search engine of your choice
 * to find (or "google up") getaddrinfo function description.
 */
struct addrinfo *resolve_address_ipv4(struct sockaddr_in const * p_in_addr, int type, int proto, int flags);

#ifdef _cplusplus
}
#endif

#endif
