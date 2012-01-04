#if !defined MCAST_RECEIVER_STATE_093A307A_2B03_4C05_8350_E3ED8C52A126
#define MCAST_RECEIVER_STATE_093A307A_2B03_4C05_8350_E3ED8C52A126

#include <windows.h>
/**
 * @brief Describes possible states for the receiver.
 * @details
 */
typedef enum receiver_state { 
    RECEIVER_INITIAL = 0, /**< Initial state, nothinig is being send or received, neither played, no multicast group is joined. */
    RECEIVER_MCASTJOINED = 1, /**< Multicast group is joined. */
    RECEIVER_PLAYING = 2, /**< Playing data from the buffer. */
    RECEIVER_MCASTJOINED_PLAYING = 3, /**< Playing data from the buffer and joined multicast group. */
    RECEIVER_RECEIVING_PLAYING = 4, /**< Receiving data into buffer and playing data from the buffer. */
    RECEIVER_RECEIVING = 5, /**< Receiving data into buffer. */
    RECEIVER_STATE_LAST = 6 /**< Tags last possible state, used to set bounds on the array of data associated with each state. */
} receiver_state_t;

receiver_state_t receiver_get_state(void);

/*!
 * @brief
 */
void receiver_init(WAVEFORMATEX * p_wfex);

/*!
 * @brief
 */
void handle_mcastjoin(void);

/*!
 * @brief
 */
void handle_mcastleave(void);

/*!
 * @brief
 */
void handle_play(HWND hMainWnd);

/*!
 * @brief
 */
void handle_stop(void);

/*!
 * @brief
 */
void handle_rcvstart(void);

/*!
 * @brief
 */
void handle_rcvstop(void);

#endif /*if !defined MCAST_RECEIVER_STATE_093A307A_2B03_4C05_8350_E3ED8C52A126*/
