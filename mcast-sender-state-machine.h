/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file mcast-sender-state-machine.h
 * @author T. Ostaszewski 
 */ 
#if !defined MCAST_SENDER_BF6FAC88_2286_488B_9256_997E81B13E49
#define MCAST_SENDER_BF6FAC88_2286_488B_9256_997E81B13E49

#include <windows.h>
#include "wave_utils.h"

struct sender_settings; 
struct mcast_sender;

/*!
 * @brief Describes a sender state.
 * @details At each moment, the sender is in one of the states
 * described by this enumeration. In each state, the sender reacts
 * differently to the handlers being called. For instance, when
 * in SENDER_INITIAL state, it only accepts handlers of handler_mcastjoin(), all other handlers will not yield a state transition or a valid action for that matter.
  */
typedef enum sender_state { 
    SENDER_INITIAL = 0,         /*!< Initial state, no sending, no receiving. */
    SENDER_MCAST_JOINED = 1,    /*!< Sender successfully joined a multicast group. */
    SENDER_SENDING = 2,         /*!< Sender sending data. */
} sender_state_t;

/*! 
 * @brief Initializes a sender state machine.
 */
struct mcast_sender * sender_create(struct sender_settings * p_settings);

/*! 
 * @brief Destroys the sender state machine.
 */
void sender_destroy(struct mcast_sender * p_sender);

/*! 
 * @brief Returns current sender state.
 */
sender_state_t sender_get_current_state(struct mcast_sender * p_sender);

/*! 
 * @brief Causes the sender to join multicast group (if not a member already or not sending already).:w
 * @details If the sender is in the state SENDER_INITIAL, then it joins the multicast group and transits to the
 * SENDER_MCAST_JOINED state. Otherwise, i.e. the sender state is different than SENDER_INITIAL, this handler does nothing.
 * While in the SENDER_MCAST_JOINED state, the sender maintains a membership to the multicast group.
 */
void sender_handle_mcastjoin(struct mcast_sender * p_sender);

/*! 
 * @brief Causes the sender to leave multicast group (if already was a member).
 * @details If the sender is in the state SENDER_MCAST_JOINED, then it transits to the SENDER_INITIAL state.
 * Otherwise, does nothing. While in the SENDER_INITIAL state, 
 * the sender is virtually idle, i.e. it does not maintain a multicast group membership neither it does send any data.
 */
void sender_handle_mcastleave(struct mcast_sender * p_sender);

/*! 
 * @brief Causes the sender to start sending data (if it was not doing so).
 * @details If the sender is in the SENDER_MCAST_JOINED state, then it transits to the
 * SENDER_SENDING state. Otherwise, does nothing. While in the SENDER_SENDING state, 
 * the sender runs a background thread that sends the WAV file over and over again to the
 * multicast group.
 */
void sender_handle_startsending(struct mcast_sender * p_sender);

/*! 
 * @brief Causes the sender to stop sending data (if it was doing so).
 * @details If the sender is in the SENDER_SENDING state, then it transits to the
 * SENDER_MCAST_JOINED state. Otherwise, does nothing.
 */
void sender_handle_stopsending(struct mcast_sender * p_sender);

#endif /*!defined MCAST_SENDER_BF6FAC88_2286_488B_9256_997E81B13E49*/

