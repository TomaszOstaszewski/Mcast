/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file dsoundplay.h
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
 * @brief A DirectSound player interface.
 * @details This interface abstracts a bit the end user from the gory details of DirectX or rather DirectSound to be more precise. Instead of concentrating how to do COM programming, how to fill play primary buffers while filling the secondary buffer, and other chores, the end user has to do one thing only. She or he shall keep the FIFO queue packed with data. What this player does is constantly poll FIFO queue for data to play. So if only client of this interface keeps the instance of the FIFO queue, the one used to create this player, packed with PCM data, the PCM data playback will be done nice and smoothly.
 */
#if !defined DSOUNDPLAY_8F4A9172_53C7_4FD8_83DC_8A3DE09800FA
#define DSOUNDPLAY_8F4A9172_53C7_4FD8_83DC_8A3DE09800FA

#include <windows.h>
#include <dsound.h>

#if defined __cplusplus
extern "C" {
#endif 

struct fifo_circular_buffer;
struct play_settings;

/*!
 *
 */
typedef struct tagDSOUNDPLAY * DSOUNDPLAY;

/*!
 *
 */
DSOUNDPLAY dsoundplayer_create(HWND hWnd, WAVEFORMATEX const * p_WFE, struct fifo_circular_buffer * g_fifo, struct play_settings const * play_settings);

/*!
 *
 */
void dsoundplayer_destroy(DSOUNDPLAY handle);

/*!
 *
 */
int dsoundplayer_play(DSOUNDPLAY handle);

/*!
 *
 */
void dsoundplayer_pause(DSOUNDPLAY handle);

/*!
 *
 */
int dsoundplayer_stop(DSOUNDPLAY handle);

#if defined __cplusplus
}
#endif 

#endif /* define DSOUNDPLAY_8F4A9172_53C7_4FD8_83DC_8A3DE09800FA */
