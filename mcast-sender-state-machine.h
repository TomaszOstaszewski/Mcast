/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file mcast-sender-state-machine.h
 * @brief Interface for the sender's state machine.
 * @author T. Ostaszewski
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
 */ 
#if !defined MCAST_SENDER_BF6FAC88_2286_488B_9256_997E81B13E49
#define MCAST_SENDER_BF6FAC88_2286_488B_9256_997E81B13E49

#include <windows.h>

struct sender_settings; 
struct mcast_sender;
struct abstract_tone;

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
    SENDER_TONE_SELECTED = 2,   /*!< Sender has a valid tone selected. */
    SENDER_MCASTJOINED_TONESELECTED = 3, /*!< Sender has a tone selected and has joined multicast group. */
    SENDER_SENDING = 4,         /*!< Sender sending data. */
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
 * @param[in] p_sender pointer to the sender object obtained via call to sender_create()
 * @return returns a state that the sender is currently (at the time of the call) in.
 * @sa sender_create()
 */
sender_state_t sender_get_current_state(struct mcast_sender * p_sender);

/*! 
 * @brief Causes the sender to join multicast group (if not a member already or not sending already).
 * @details If the sender is in the state SENDER_INITIAL, then it joins the multicast group and transits to the
 * SENDER_MCAST_JOINED state. Otherwise, i.e. the sender state is different than SENDER_INITIAL, this handler does nothing.
 * While in the SENDER_MCAST_JOINED state, the sender maintains a membership to the multicast group.
 * @param[in] p_sender pointer to the sender object obtained via call to sender_create()
 * @sa sender_create()
 */
void sender_handle_mcastjoin(struct mcast_sender * p_sender);

/*! 
 * @brief Causes the sender to leave multicast group (if already was a member).
 * @details If the sender is in the state SENDER_MCAST_JOINED, then it transits to the SENDER_INITIAL state.
 * Otherwise, does nothing. While in the SENDER_INITIAL state, 
 * the sender is virtually idle, i.e. it does not maintain a multicast group membership neither it does send any data.
 * @param[in] p_sender pointer to the sender object obtained via call to sender_create()
 * @sa sender_create()
 */
void sender_handle_mcastleave(struct mcast_sender * p_sender);

/*! 
 * @brief Selects a particular tone to be send via multicast.
 * @param[in] p_sender pointer to the sender object obtained via call to sender_create()
 * @param[in] p_tone handle of the tone whose to be selected to send.
 * @sa sender_handle_deselecttone()
 */
void sender_handle_selecttone(struct mcast_sender * p_sender, struct abstract_tone * p_tone);

/*! 
 * @brief Indicates, that the tone shall no longer be send onto the multicast group.
 * @param[in] p_sender pointer to the sender object obtained via call to sender_create()
 * @sa sender_handle_selecttone()
 */
void sender_handle_deselecttone(struct mcast_sender * p_sender);

/*! 
 * @brief Causes the sender to start sending data (if it was not doing so).
 * @details If the sender is in the SENDER_MCAST_JOINED state, then it transits to the
 * SENDER_SENDING state. Otherwise, does nothing. While in the SENDER_SENDING state, 
 * the sender runs a background thread that sends the WAV file over and over again to the
 * multicast group.
 * @param[in] p_sender pointer to the sender object obtained via call to sender_create()
 * @sa sender_create()
 */
void sender_handle_startsending(struct mcast_sender * p_sender);

/*! 
 * @brief Causes the sender to stop sending data (if it was doing so).
 * @details If the sender is in the SENDER_SENDING state, then it transits to the
 * SENDER_MCAST_JOINED state. Otherwise, does nothing.
 * @param[in] p_sender pointer to the sender object obtained via call to sender_create()
 * @sa sender_create()
 */
void sender_handle_stopsending(struct mcast_sender * p_sender);

#endif /*!defined MCAST_SENDER_BF6FAC88_2286_488B_9256_997E81B13E49*/

