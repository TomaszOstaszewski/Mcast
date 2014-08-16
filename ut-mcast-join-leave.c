/**
 * @file ut-mcast-join-leave.c
 * @brief Unit/Functional test for sending data over Multicast
 * @details
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
#include "debug-ipv4.h"

#define DEFAULT_MCAST_GROUP_ADDR "239.0.0.1"
#define DEFAULT_PORT 25000
#define DEFAULT_INTERFACE "192.168.0.254"
#define DEFAULT_TTL 2

typedef struct AddrInfoSockAddrIn {
    struct addrinfo addrInfo_;
    struct sockaddr_in sockaddrIn_;
} AddrInfoSockAddrIn_t;

typedef struct CmdLineOptions {
    const char* pItfAddr_;
    const char* pMcastGroupAddr_;
    unsigned short nPortNumber_;
} CmdLineOptions_t;

#define DEFAULT_CMD_LINE_OPTIONS                                                                   \
    { .pItfAddr_ = DEFAULT_INTERFACE, .pMcastGroupAddr_ = DEFAULT_MCAST_GROUP_ADDR };

int set_reuse_addr(SOCKET s);

static int parseCmdLine(int argc, char** argv, CmdLineOptions_t* pCmdLineOpt) {
    int nTokensParsed;
    unsigned int nPortNumber = (unsigned int)-1;

    do {
        int result;
        result = getopt(argc, argv, "i:m:p:");
        if (-1 == result) {
            break;
        }
        switch (result) {
            case 'i':
                /** @todo: Add IP address validation. */
                pCmdLineOpt->pItfAddr_ = optarg;
                break;
            case 'm':
                pCmdLineOpt->pMcastGroupAddr_ = optarg;
                break;
            case 'p':
                nTokensParsed = sscanf(optarg, "%u", &nPortNumber);
                if (1 != nTokensParsed || nPortNumber > USHRT_MAX) {
                    fprintf(stderr, "%4.4u %s : %d %u\n", __LINE__, __func__, nTokensParsed,
                            nPortNumber);
                    return -1;
                } else {
                    pCmdLineOpt->nPortNumber_ = nPortNumber;
                }
                break;
            default:
                fprintf(stderr, "Unknown option: %c\n", result);
                return -1;
        }
    } while (1);
    return 0;
}

static void fillDefaultAddrInfo(struct AddrInfoSockAddrIn* pTarget, const char* pAddress) {
    struct sockaddr_in* pInet4Addr = &pTarget->sockaddrIn_;

    pInet4Addr->sin_family = AF_INET;
    pInet4Addr->sin_port = htons(DEFAULT_PORT);
    inet_pton(AF_INET, pAddress, &pInet4Addr->sin_addr);

    pTarget->addrInfo_.ai_flags = 0;
    pTarget->addrInfo_.ai_family = AF_INET;
    pTarget->addrInfo_.ai_socktype = SOCK_DGRAM;
    pTarget->addrInfo_.ai_protocol = 0;
    pTarget->addrInfo_.ai_addrlen = sizeof(struct sockaddr_in);
    pTarget->addrInfo_.ai_addr = (struct sockaddr*)pInet4Addr;
    pTarget->addrInfo_.ai_next = NULL;
    pTarget->addrInfo_.ai_canonname = NULL;
}

static void testJoinMcastIpv4(const char* pItfAddr, const char* pMcastGroupAddr,
                              unsigned short nPort) {
    struct AddrInfoSockAddrIn anMcastGroupInfo;
    struct AddrInfoSockAddrIn anItfInfo;
    int result;
    SOCKET s;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    assert(s >= 0);
    fillDefaultAddrInfo(&anItfInfo, pItfAddr);
    fillDefaultAddrInfo(&anMcastGroupInfo, pMcastGroupAddr);

    char aBuffer[256] = { 0 };
    size_t nBuffer;
    char* pBufferBegin;
    pBufferBegin = &aBuffer[0], nBuffer = sizeof(aBuffer);
    dumpAddrInfo(&pBufferBegin, &nBuffer, &anItfInfo.addrInfo_);
    fprintf(stdout, "%s\n", &aBuffer[0]);
    pBufferBegin = &aBuffer[0], nBuffer = sizeof(aBuffer);
    dumpAddrInfo(&pBufferBegin, &nBuffer, &anMcastGroupInfo.addrInfo_);
    fprintf(stdout, "%s\n", &aBuffer[0]);

    set_reuse_addr(s);
    result =
        join_mcast_group_set_ttl(s, &anMcastGroupInfo.addrInfo_, &anItfInfo.addrInfo_, DEFAULT_TTL);

    assert(0 == result);
    {
        static const char buffer[] = "The old new thing.";
        result = sendto(s, &buffer[0], sizeof(buffer), 0, anMcastGroupInfo.addrInfo_.ai_addr,
                        anMcastGroupInfo.addrInfo_.ai_addrlen);
        assert(result > 0);
        fprintf(stdout, "%4.4u %s : %d\n", __LINE__, __func__, result);
    }
    close(s);
}

int main(int argc, char* argv[]) {
    CmdLineOptions_t aCmdLineOpt = DEFAULT_CMD_LINE_OPTIONS;
    int result;

    result = parseCmdLine(argc, argv, &aCmdLineOpt);
    if (0 == result) {
        testJoinMcastIpv4(aCmdLineOpt.pItfAddr_, aCmdLineOpt.pMcastGroupAddr_,
                          aCmdLineOpt.nPortNumber_);
    }
    return 0;
}
