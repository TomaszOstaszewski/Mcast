/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file mcast-receiver-state-machine.h
 * @author T.Ostaszewski
 * @par License
 * @code Copyright 2012 Tomasz Ostaszewski. All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * 	1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *	2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation 
 * 	and/or other materials provided with the distribution.
  * THIS SOFTWARE IS PROVIDED BY Tomasz Ostaszewski AS IS AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL Tomasz Ostaszewski OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE.
  * The views and conclusions contained in the software and documentation are those of the 
 * authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of Tomasz Ostaszewski.
 * @endcode
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
 * @param[in] p_wfex parameters of the WAV file to be played.
 * @param[in] p_settings pointer to the structure describing receiver settings (mcast connection params, timings and so on).
 * @return Returns a handle to the object that represents multicast PCM receiver. This handle can be then passed to functions handle_play() and so on.
 * @sa handle_mcastjoin
 * @sa handle_mcastleave
 * @sa handle_play
 * @sa handle_stop
 * @sa handle_rcvstart
 * @sa handle_rcvstop
 */
struct mcast_receiver * receiver_create(struct receiver_settings const * p_settings);

/*!
 * @brief Destroys the state machine.
 * @param[in] p_wfex parameters of the WAV file to be played.
 * @return Returns non-zero on sucess, 0 otherwise. Most likely cause of failure is that receiver is not in the RECEIVER_INITIAL state.
 * @sa receiver_init
 */
int receiver_destroy(struct mcast_receiver * p_receiver);

/*!
 * @brief Join multicast group handler.
 * @param[in] p_receiver handle to the receiver object. 
 */
void handle_mcastjoin(struct mcast_receiver * p_receiver);

/*!
 * @brief Leave multicast group handler.
 * @param[in] p_receiver handle to the receiver object. 
 */
void handle_mcastleave(struct mcast_receiver * p_receiver);

/*!
 * @brief Play handler.
 * @details This function is called, when the client 
 * wants to play data from the buffer. The buffer is usually
 * filled with data being fetched from the network. But one can
 * start playing without any network data being fetched - in that case
 * they will only hear silence.
 * @param[in] p_receiver handle to the receiver object. 
 * @param[in] hMainWnd handle to the window that will be used for playbac. This handle can be the handle of 
 * your main application window. Or it can be NULL, in which case the foreground window or the desktop window will be hijacked.
 */
void handle_play(struct mcast_receiver * p_receiver, HWND hMainWnd);

/*!
 * @brief Stop playing handler.
 * @details This one is called when the client wants to stop playback.
 * @param[in] p_receiver handle to the receiver object. 
 */
void handle_stop(struct mcast_receiver * p_receiver);

/*!
 * @brief Receiver start handler.
 * @details This function is called when the client wants
 * to start receiving multicast packets.
 * @param[in] p_receiver handle to the receiver object. 
 */
void handle_rcvstart(struct mcast_receiver * p_receiver);

/*!
 * @brief Receiver stop handler.
 * @details This function is called when the client wants
 * to stop receiving multicast packets.
 * @param[in] p_receiver handle to the receiver object. 
 */
void handle_rcvstop(struct mcast_receiver * p_receiver);

/*!
 * @brief Retrieves a FIFO queue that is used to pass the data between multicast
 * receiver and player.
 * @param[in] p_receiver handle to the receiver object. 
 * @return Returns a pointer to the FIFO queue.
 */
struct fifo_circular_buffer * receiver_get_fifo(struct mcast_receiver * p_receiver);

#endif /*if !defined MCAST_RECEIVER_STATE_093A307A_2B03_4C05_8350_E3ED8C52A126*/
