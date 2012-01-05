/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file conn_entry.h
 * @author T.Ostaszewski
 * @date 04-Jan-2012
 * @brief
 * @details
 */
#if !defined CONN_ENTRY_B6C1F8A4_23E1_44FD_885B_5C3CB63D7FFF
#define CONN_ENTRY_B6C1F8A4_23E1_44FD_885B_5C3CB63D7FFF

#include "tei.h"
#define CONN_ENTRY_ID_LENGTH (32)

typedef enum tag_tetra_device_type {
	TETRA_BRICK = 0,
	TETRA_CONTROL_HEAD = 1,
} tetra_device_type;

typedef struct conn_entry { 
    struct sockaddr_in  address_;
    struct tei          tei_;
    tetra_device_type   type_;
    struct timeval      purge_timeout_;
	char				id_[CONN_ENTRY_ID_LENGTH];
} conn_entry_t;

#endif /* CONN_ENTRY_B6C1F8A4_23E1_44FD_885B_5C3CB63D7FFF */

