/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file circular-buffer-uint8.h
 * @author T.Ostaszewski
 * @brief A circular buffer interface.
 * @details This file contains forward declarations of circular buffer functions. The circular buffer is 
 * a table based buffer. Buffer size is a power of 2, which allows for quite simple and straightforward implementation.
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
 */
#if !defined CIRCULAR_BUFFER_UINT16_C9525583_D956_45DA_8380_D24A3EAC3250
#define CIRCULAR_BUFFER_UINT16_C9525583_D956_45DA_8380_D24A3EAC3250

#if defined __cplusplus
extern "C" {
#endif

#include "std-int.h"
#include <stddef.h>

/*!
 * @brief Forward declaration.
 */
struct circular_buffer_uint16;

/**
 * @brief Creates a circular buffer, specifies its size.
 * @details <b>Fill me...</b>
 * @param[in] level this is the exponent of the buffer size. Actual buffer, when successfully created, holds up to 2^level items without overwritting the oldest ones.
 * @return returns a handle to a circular buffer, or NULL if creation failed.
 * @sa circular_buffer_uint16_delete
 */
struct circular_buffer_uint16 * circular_buffer_uint16_create_with_size(uint8_t level);

/**
 * @brief Destroys a circular buffer
 * @param[in] p_fifo a handle to the circular buffer obtained via call to circular_buffer_create
 * @sa circular_buffer_create
 */
void circular_buffer_uint16_delete(struct circular_buffer_uint16 * p_fifo);

/**
 * @brief Returns total number of items that can be stored in the buffer.
 * @details The total number of items is maximum of what can be stored without the risk of overwriting last recently written items.
 * @param[in] p_fifo a handle to the circular buffer obtained via call to circular_buffer_create
 * @return number of items that can be stored
 */
uint32_t circular_buffer_uint16_get_capacity(struct circular_buffer_uint16 const * p_fifo);

/**
 * @brief Returns actual number of items that can be stored in the buffer.
 * @details
 * @param[in] p_fifo a handle to the circular buffer obtained via call to circular_buffer_create
 * @return actual number of items that can be stored in the buffer
 */
uint32_t circular_buffer_uint16_get_items_count(struct circular_buffer_uint16 const * p_fifo);

/**
 * @brief Checks, if the queue can accomodate more data.
 * @details
 * @param[in] p_fifo a handle to the circular buffer obtained via call to circular_buffer_create
 * @return returns non-zero if there is still free space in the buffer. Otherwise, returns 0.
 */
int circular_buffer_uint16_is_free_space(struct circular_buffer_uint16 * p_fifo);

/**
 * @brief Checks, if queue is completely full.
 * @details If queue is full, then next insert will overwrite date that was written at the beginning of the queue.
 * @param[in] p_fifo a handle to the circular buffer obtained via call to circular_buffer_create
 * @return returns a non zero value if queue is full. If returns 0, then queue can accomodate more data.
 */
unsigned int circular_buffer_uint16_is_full(struct circular_buffer_uint16 * p_fifo);

/**
 * @brief Puts new data into the queue.
 * @param[in] p_fifo a handle to the circular buffer obtained via call to circular_buffer_create. It is that 
 * queue the data will be put into.
 * @param[in] p_data pointer to the array whose contents will be filled with data retrieved from the queue.
 * @param[in] count indicates the length tof the array given as the p_data parameter.
 * @return This call returns the number of bytes pushed, or -1 if an error occurred. 
 */
size_t circular_buffer_uint16_push_item(struct circular_buffer_uint16 * p_fifo, uint16_t const * p_data, uint32_t count);

/**
 * @brief Removes data from the queue.
 * @param[in] p_fifo a handle to the circular buffer obtained via call to circular_buffer_create. It is that queue
 * from which data will be fetched.
 * @param[in] p_data pointer to the array whose contents will be filled with data retrieved from the queue.
 * @param[in] p_req_count pointer to the variable which holds the length of the array given as the p_data parameter.
 * @return This call returns the number of bytes retrieved, or -1 if an error occurred. 
 */
size_t circular_buffer_uint16_fetch_item(struct circular_buffer_uint16 * p_fifo, uint16_t * p_data, uint32_t req_count);

#if defined __cplusplus
}
#endif

#endif /* CIRCULAR_BUFFER_UINT16_C9525583_D956_45DA_8380_D24A3EAC3250 */

