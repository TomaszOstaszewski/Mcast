/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file mcast-receiver-state-machine.h
 * @author T.Ostaszewski
 * @date 04-Jan-2012
 * @brief Receiver's state machine front end.
 * @details
 */
#if !defined MCAST_RECEIVER_STATE_093A307A_2B03_4C05_8350_E3ED8C52A126
#define MCAST_RECEIVER_STATE_093A307A_2B03_4C05_8350_E3ED8C52A126

#include <windows.h>

/**
 * @brief Describes possible states for the receiver.
 * @details At each moment, the receiver is in one of the states
 * described by this enumeration. In each state, the receiver reacts
 * differently to the handlers being called. For instance, when
 * in RECEIVER_INITIAL state, it only accepts handlers of handler_mcastjoin() or handler_play().
 * All other handlers will not yield a state transition or a valid action for that matter.
 */
typedef enum receiver_state { 
    RECEIVER_INITIAL = 0, /**< Initial state, nothinig is being send or received, neither played, no multicast group is joined. */
    RECEIVER_MCASTJOINED = 1, /**< Multicast group is joined. */
    RECEIVER_PLAYING = 2, /**< Playing data from the buffer. */
    RECEIVER_MCASTJOINED_PLAYING = 3, /**< Playing data from the buffer and joined multicast group. */
    RECEIVER_RECEIVING_PLAYING = 4, /**< Receiving data into buffer and playing data from the buffer. */
    RECEIVER_RECEIVING = 5, /**< Receiving data into buffer. */
} receiver_state_t;

/*!
 * @brief Returns current state the receiver is at the moment of the call of this function.:w
 */
receiver_state_t receiver_get_state(struct mcast_receiver * p_receiver);

/*!
 * @brief Initializes the state machine.
 * @param[in]
 * @param[in]
 * @return 
 */
struct mcast_receiver * receiver_init(WAVEFORMATEX * p_wfex, struct mcast_settings const * mcast_settings);

/*!
 * @brief Join multicast group handler.
 */
void handle_mcastjoin(struct mcast_receiver * p_receiver);

/*!
 * @brief Leave multicast group handler.
 */
void handle_mcastleave(struct mcast_receiver * p_receiver);

/*!
 * @brief Play handler.
 * @details This function is called, when the client 
 * wants to play data from the buffer. The buffer is usually
 * filled with data being fetched from the network. But one can
 * start playing without any network data being fetched - in that case
 * they will only hear silence.
 */
void handle_play(struct mcast_receiver * p_receiver, HWND hMainWnd);

/*!
 * @brief Stop playing handler.
 * @details This one is called when the client wants to stop playback.
 */
void handle_stop(struct mcast_receiver * p_receiver);

/*!
 * @brief Receiver start handler.
 * @details This function is called when the client wants
 * to start receiving multicast packets.
 */
void handle_rcvstart(struct mcast_receiver * p_receiver);

/*!
 * @brief Receiver stop handler.
 * @details This function is called when the client wants
 * to stop receiving multicast packets.
 */
void handle_rcvstop(struct mcast_receiver * p_receiver);

#endif /*if !defined MCAST_RECEIVER_STATE_093A307A_2B03_4C05_8350_E3ED8C52A126*/
