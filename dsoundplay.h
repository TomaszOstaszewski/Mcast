/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file dsoundplay.h
 * @author T.Ostaszewski
 * @date 04-Jan-2012
 * @brief A DirectSound player interface.
 * @details This interface abstracts a bit the end user from the gory details of DirectX or rather DirectSound to be more precise. Instead of concentrating how to do COM programming, how to fill play primary buffers while filling the secondary buffer, and other chores, the end user has to do one thing only. She or he shall keep the FIFO queue packed with data. What this player does is constantly poll FIFO queue for data to play. So if only client of this interface keeps the instance of the FIFO queue, the one used to create this player, packed with PCM data, the PCM data playback will be done nice and smoothly.
 */
#if !defined DSOUNDPLAY_8F4A9172_53C7_4FD8_83DC_8A3DE09800FA
#define DSOUNDPLAY_8F4A9172_53C7_4FD8_83DC_8A3DE09800FA

#include <windows.h>
#include <dsound.h>
#include <stddef.h>

#if defined __cplusplus
extern "C" {
#endif 

struct fifo_circular_buffer;

/*!
 *
 */
typedef struct tagDSOUNDPLAY * DSOUNDPLAY;

/*!
 *
 */
DSOUNDPLAY dsoundplayer_create(HWND hWnd, WAVEFORMATEX const * p_WFE, struct fifo_circular_buffer * g_fifo);

/*!
 *
 */
void dsoundplayer_destroy(DSOUNDPLAY handle);

/*!
 *
 */
void dsoundplayer_play(DSOUNDPLAY handle);

/*!
 *
 */
void dsoundplayer_pause(DSOUNDPLAY handle);

/*!
 *
 */
void dsoundplayer_stop(DSOUNDPLAY handle);

#if defined __cplusplus
}
#endif 

#endif /* define DSOUNDPLAY_8F4A9172_53C7_4FD8_83DC_8A3DE09800FA */
