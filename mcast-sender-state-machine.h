/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file mcast-sender-state-machine.h
 * @author T. Ostaszewski 
 */ 
#if !defined MCAST_SENDER_BF6FAC88_2286_488B_9256_997E81B13E49
#define MCAST_SENDER_BF6FAC88_2286_488B_9256_997E81B13E49

#include <windows.h>
#include "wave_utils.h"

/*!
 * @brief Describes a sender state.
 */
typedef enum sender_state { 
    SENDER_INITIAL = 0,         /*!< Initial state, no sending, no receiving. */
    SENDER_MCAST_JOINED = 1,    /*!< Sender successfully joined a multicast group. */
    SENDER_SENDING = 2,         /*!< Sender sending data. */
} sender_state_t;

/*! 
 * @brief Returns current sender state.
 */
sender_state_t sender_get_current_state(void);

/*! 
 * @brief 
 */
void sender_initialize(master_riff_chunk_t * p_wav_chunk);

/*! 
 * @brief
 */
void sender_handle_mcastjoin(void);

/*! 
 * @brief
 */
void sender_handle_mcastleave(void);

/*! 
 * @brief
 */
void sender_handle_startsending(void);

/*! 
 * @brief
 */
void sender_handle_stopsending(void);

#endif /*!defined MCAST_SENDER_BF6FAC88_2286_488B_9256_997E81B13E49*/

