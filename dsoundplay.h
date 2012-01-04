/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file dsoundplay.h
 * @author
 * @date
 * @brief
 * @details
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
