/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file sender-sm-states.h
 * @brief Defines states for the sender's state machine.
 * @author T. Ostaszewski
 * @date Feb-2012
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

#if !defined SENDER_SM_STATES_B5AE1FBE_0B24_46AE_B274_3F8340C25E41
#define SENDER_SM_STATES_B5AE1FBE_0B24_46AE_B274_3F8340C25E41

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

#endif /* SENDER_SM_STATES_B5AE1FBE_0B24_46AE_B274_3F8340C25E41 */

