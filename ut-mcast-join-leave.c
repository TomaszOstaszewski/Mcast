/**
 * @file ut-mcast-join-leave.c
 * @brief 1
 * @details 2
 * @author Tomasz Ostaszewski (ato013)
 * @date 2014-Aug-08
 * @par History
 * <pre>
 * </pre>
 * @copyright Copyright (C) 2014 Motorola Solutions Inc. All rights reserved.
 * Motorola Solutions Confidential Restricted
 */
#include "pcc.h"
#include <assert.h>
#include "mcast_utils.h"

#define DEFAULT_MCAST_GROUP_ADDR "239.0.0.1"
#define DEFAULT_PORT 8888
#define DEFAULT_INTERFACE "192.168.0.117"
#define DEFAULT_TTL 2

struct AddrInfoSockAddr {
    struct addrinfo addrInfo_;
    struct sockaddr sockAddr_;
};

static void _getDefaultAddr(struct addrinfo* pAddrInfo, struct sockaddr_in* pSockAddrIn,
                            const char* pDottedAddr) {
    pAddrInfo->ai_family = AF_INET;
    pAddrInfo->ai_socktype = SOCK_DGRAM;
    pAddrInfo->ai_addr = (struct sockaddr*)pSockAddrIn;
    pAddrInfo->ai_addrlen = sizeof(struct sockaddr_in);
    inet_pton(AF_INET, pDottedAddr, &(pSockAddrIn->sin_addr));
    pSockAddrIn->sin_port = htons(DEFAULT_PORT);
    fprintf(stdout, "%4.4u %s : %20s %8.8x %5u\n", __LINE__, __func__, pDottedAddr,
            htonl(pSockAddrIn->sin_addr.s_addr), htons(pSockAddrIn->sin_port));
}

static const struct addrinfo* getDefaultMcastGroupInfo(void) {
    static __thread struct AddrInfoSockAddr sAddress_;
    struct sockaddr_in* pIpv4Addr = (struct sockaddr_in*)&sAddress_.sockAddr_;
    memset(&sAddress_, 0, sizeof(sAddress_));

    _getDefaultAddr(&sAddress_.addrInfo_, pIpv4Addr, DEFAULT_MCAST_GROUP_ADDR);
    return &sAddress_.addrInfo_;
}

static const struct addrinfo* getDefaultItfInfo(void) {
    static __thread struct AddrInfoSockAddr sAddress_;
    struct sockaddr_in* pIpv4Addr = (struct sockaddr_in*)&sAddress_.sockAddr_;
    memset(&sAddress_, 0, sizeof(sAddress_));

    _getDefaultAddr(&sAddress_.addrInfo_, pIpv4Addr, DEFAULT_INTERFACE);
    return &sAddress_.addrInfo_;
}

static void dumpAddrInfo(FILE* fp, const struct addrinfo* pInfo) {
    fprintf(fp, "%4.4u %s : %2d %2d %2d %2d %p %u ", __LINE__, __func__, 
            pInfo->ai_flags,
            pInfo->ai_family,
            pInfo->ai_socktype,
            pInfo->ai_protocol,
             pInfo->ai_addr, pInfo->ai_addrlen);
    switch (pInfo->ai_addrlen) {
        case sizeof(struct sockaddr_in): {
            const struct sockaddr_in* pSockAddrIn = (const struct sockaddr_in*)pInfo->ai_addr;
            fprintf(stdout, "%8.8x %5u ", htonl(pSockAddrIn->sin_addr.s_addr),
                    htons(pSockAddrIn->sin_port));

        } break;
        default:
            break;
    }
    fprintf(fp, "\n");
}

static void test_join_ipv4(void) {
    SOCKET s;
    s = socket(AF_INET, SOCK_DGRAM, 0);
    assert(s >= 0);
    struct addrinfo anMcastGroupInfo = { 0 };
    struct addrinfo anItfInfo = { 0 };

    memcpy(&anItfInfo, getDefaultItfInfo(), sizeof(anItfInfo));
    memcpy(&anMcastGroupInfo, getDefaultMcastGroupInfo(), sizeof(anMcastGroupInfo));
    dumpAddrInfo(stdout, &anItfInfo);
    dumpAddrInfo(stdout, &anMcastGroupInfo);
    int result;
    result = join_mcast_group_set_ttl(s, &anMcastGroupInfo, &anItfInfo, DEFAULT_TTL);
    assert(0 == result);
    close(s);
}

int main(int argc, char* argv[]) {
    test_join_ipv4();
    return 0;
}
