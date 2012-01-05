/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file hello.h
 * @author T.Ostaszewski
 * @date 04-Jan-2012
 * @brief
 * @details
 */
#if !defined HELLO_H_F6C604C8_ED2D_4D24_8656_399A5E60B2B1
#define HELLO_H_F6C604C8_ED2D_4D24_8656_399A5E60B2B1

#include "winsock_adapter.h"
#include "tei.h"

#if defined WIN32
#define PACKED __declspec(align(1))
#else
#define PACKED __attribute__((packed))
#endif

typedef struct hello_packet {
    uint8_t     type_;
    uint8_t     counter_;
    struct tei  tei_;
} PACKED hello_packet_t;

#endif /* HELLO_H_F6C604C8_ED2D_4D24_8656_399A5E60B2B1 */
