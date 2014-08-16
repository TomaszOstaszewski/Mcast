/**
 * @file debug-ipv4.c
 * @brief Implementation of the debug helpers.
 * @details
 * @author T.Ostaszewski
 * @date 2014-Aug-16
 * @par History
 * <pre>
 * </pre>
 * @copyright
 */

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/types.h>

void dumpSockAddrIn(char** ppBuffer, size_t* pBufferSize, const struct sockaddr_in* pSockAddrIn) {
    char* pTmpBuffer = NULL;
    asprintf(&pTmpBuffer, "(list %d %u \"%s\")", pSockAddrIn->sin_family,
             ntohs(pSockAddrIn->sin_port), inet_ntoa(pSockAddrIn->sin_addr));
    if (NULL != pTmpBuffer) {
        size_t outputDataSize = strlen(pTmpBuffer);
        if (outputDataSize + 1 < *pBufferSize) {
            strncpy(*ppBuffer, pTmpBuffer, outputDataSize);
            (*ppBuffer)[outputDataSize] = '\0';
            *ppBuffer += outputDataSize;
            *pBufferSize -= outputDataSize;
        }
    }
    free(pTmpBuffer);
}

void dumpSockAddr(char** ppBuffer, size_t* pBufferSize, const struct sockaddr* pSockAddr,
                  socklen_t nSockAddrLen) {
    const struct sockaddr_in* pSockAddrIn;

    switch (nSockAddrLen) {
        case sizeof(struct sockaddr_in):
            pSockAddrIn = (const struct sockaddr_in*)pSockAddr;
            dumpSockAddrIn(ppBuffer, pBufferSize, pSockAddrIn);
            break;
        case AF_INET6:
            break;
        default:
            break;
    }
}

void dumpAddrInfo(char** ppBuffer, size_t* pBufferSize, struct addrinfo const* pInfo) {

    size_t nBufferSize = 32;
    size_t nOldSize = nBufferSize;
    char* pTmpBufferBegin = malloc(sizeof(char) * nBufferSize);
    memset(pTmpBufferBegin, 0xff, sizeof(char) * nBufferSize);
    char* pTmpBuffer = pTmpBufferBegin;
    /* Log ai_addr member to a temporary buffer */
    do {
        fprintf(stderr, "%4.4u %s : %u \n", __LINE__, __func__, nBufferSize);
        dumpSockAddr(&pTmpBuffer, &nBufferSize, pInfo->ai_addr, pInfo->ai_addrlen);
        if (nBufferSize != nOldSize) {
            break;
        }
        pTmpBufferBegin = realloc(pTmpBufferBegin, 2 * nBufferSize);
        memset(pTmpBufferBegin, 0, sizeof(char) * nBufferSize * 2);
        pTmpBuffer = pTmpBufferBegin;
        nBufferSize *= 2;
        nOldSize = nBufferSize;
    } while (1);

    snprintf(*ppBuffer, *pBufferSize, "(list "
                                      "(list 'ai_flags  %d) "
                                      "(list 'ai_family  %d) "
                                      "(list 'ai_socktype  %d) "
                                      "(list 'ai_protocol  %d) "
                                      "(list 'ai_addrlen  %d) "
                                      "(list 'ai_addr %s) "
                                      "(list 'ai_canonname \"%s\") "
                                      "(list 'ai_next %p) ) ",
             pInfo->ai_flags, pInfo->ai_family, pInfo->ai_socktype, pInfo->ai_protocol,
             pInfo->ai_addrlen, pTmpBufferBegin, pInfo->ai_canonname, pInfo->ai_next);

    free(pTmpBufferBegin);

    return;
}
