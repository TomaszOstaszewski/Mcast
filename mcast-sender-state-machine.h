/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file mcast-sender-state-machine.h
 * @author T. Ostaszewski 
 */ 
#if !defined MCAST_SENDER_BF6FAC88_2286_488B_9256_997E81B13E49
#define MCAST_SENDER_BF6FAC88_2286_488B_9256_997E81B13E49

#include <windows.h>
#include "wave_utils.h"

typedef enum sender_state { 
    SENDER_INITIAL = 0,
    SENDER_MCAST_JOINED = 1,
    SENDER_SENDING = 2,
    SENDER_LAST = 3
} sender_state_t;

sender_state_t sender_get_current_state(void);

void sender_initialize(master_riff_chunk_t * p_wav_chunk);
void sender_handle_mcastjoin(void);
void sender_handle_mcastleave(void);
void sender_handle_startsending(void);
void sender_handle_stopsending(void);

#endif /*!defined MCAST_SENDER_BF6FAC88_2286_488B_9256_997E81B13E49*/
