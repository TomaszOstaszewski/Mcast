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
#define DEFAULT_INTERFACE "192.168.0.254"
#define DEFAULT_TTL 2

static const struct sockaddr_in* getDefaultMcastGroupAddr(void) { 
    static struct sockaddr_in sAddress_;
    sAddress_.sin_family = AF_INET;
    inet_pton(AF_INET, DEFAULT_MCAST_GROUP_ADDR, &sAddress_.sin_addr);
    sAddress_.sin_port = htons(DEFAULT_PORT);
    return &sAddress_;
}

static const struct sockaddr_in* getDefaultItfAddr(void) { 
    static struct sockaddr_in sAddress_;
    sAddress_.sin_family = AF_INET;
    inet_pton(AF_INET, DEFAULT_INTERFACE, &sAddress_.sin_addr);
    sAddress_.sin_port = htons(DEFAULT_PORT);
    return &sAddress_;
}

static void test_join_ipv4(void) {
    SOCKET s;
    s = socket(AF_INET, SOCK_DGRAM, 0);
    assert(s >= 0);
    struct addrinfo anMcastGroupInfo = { 0 };
    struct addrinfo anItfInfo = { 0 };
    struct sockaddr_in aGroupAddr;
    struct sockaddr_in anItfAddr;

    memcpy(&anItfAddr, getDefaultItfAddr(), sizeof(anItfAddr));
    memcpy(&aGroupAddr, getDefaultMcastGroupAddr(), sizeof(aGroupAddr));
    
    anMcastGroupInfo.ai_family = AF_INET;
    anMcastGroupInfo.ai_socktype = SOCK_DGRAM;
    anMcastGroupInfo.ai_addr = (struct sockaddr*)&aGroupAddr;
    anMcastGroupInfo.ai_addrlen = sizeof(aGroupAddr);

    anItfInfo.ai_family = AF_INET;
    anItfInfo.ai_socktype = SOCK_DGRAM;
    anItfInfo.ai_addr = (struct sockaddr *)&anItfAddr;
    anItfInfo.ai_addrlen = sizeof(anItfAddr);

    int result;
    result = join_mcast_group_set_ttl(s, &anMcastGroupInfo, &anItfInfo, DEFAULT_TTL);
    assert(0 == result);
    close(s);
}

int main(int argc, char *argv[])
{
    test_join_ipv4();
    return 0;
}
