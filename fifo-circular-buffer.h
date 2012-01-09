/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file fifo-circular-buffer.h
 * @author T.Ostaszewski
 * @date 04-Jan-2012
 * @brief A circular buffer header file.
 * @details This file contains forward declarations of circular buffer functions. The circular buffer is 
 * a table based buffer. Buffer size is a power of 2, which allows for quite simple and straightforward implementation.
 */
#if !defined FIFO_CIRCULAR_BUFFER_C64CA1CA_A1C6_4C83_8CF4_DBAF21D17EF7
#define FIFO_CIRCULAR_BUFFER_C64CA1CA_A1C6_4C83_8CF4_DBAF21D17EF7

#if defined __cplusplus
extern "C" {
#endif

/**
 * @brief
 */
#define DATA_ITEM_SIZE (32*32)

typedef struct data_item {
    uint32_t    	count_;
    uint8_t     *	data_;
} data_item_t;

struct fifo_circular_buffer;

/**
 * @brief Create a circular buffer.
 * @details Fill me...
 * @return returns a handle to a circular buffer, or NULL if creation failed.
 */
struct fifo_circular_buffer *  fifo_circular_buffer_create(void);

/**
 * @brief Destroys a circular buffer
 * @details
 * @param[in] p_fifo a handle to the circular buffer obtained via call to fifo_circular_buffer_create
 * @sa fifo_circular_buffer_create
 */
void fifo_circular_buffer_delete(struct fifo_circular_buffer * p_fifo);

/**
 * @brief Returns total number of items that can be stored in the buffer.
 * @details The total number of items is maximum of what can be stored without the risk of overwriting last recently written items.
 * @param[in] p_fifo a handle to the circular buffer obtained via call to fifo_circular_buffer_create
 * @return number of items that can be stored
 */
uint32_t fifo_circular_buffer_get_capacity(struct fifo_circular_buffer * p_fifo);

/**
 * @brief Returns actual number of items that can be stored in the buffer.
 * @details
 * @param[in] p_fifo a handle to the circular buffer obtained via call to fifo_circular_buffer_create
 * @return actual number of items that can be stored in the buffer
 */
uint32_t fifo_circular_buffer_get_items_count(struct fifo_circular_buffer * p_fifo);

/**
 * @brief
 * @details
 * @param[in] p_fifo a handle to the circular buffer obtained via call to fifo_circular_buffer_create
 */
int fifo_circular_buffer_is_free_space(struct fifo_circular_buffer * p_fifo);

/**
 * @brief
 * @details
 * @param[in] p_fifo a handle to the circular buffer obtained via call to fifo_circular_buffer_create
 */
unsigned int fifo_circular_buffer_is_full(struct fifo_circular_buffer * p_fifo);

/**
 * @brief
 * @details
 * @param[in] p_fifo a handle to the circular buffer obtained via call to fifo_circular_buffer_create
 */
int fifo_circular_buffer_push_item(struct fifo_circular_buffer * p_fifo, struct data_item const * p_item);

/**
 * @brief
 * @details
 * @param[in] p_fifo a handle to the circular buffer obtained via call to fifo_circular_buffer_create
 */
int fifo_circular_buffer_fetch_item(struct fifo_circular_buffer * p_fifo, struct data_item * p_item);

#if defined __cplusplus
}
#endif

#endif /* !defined FIFO_CIRCULAR_BUFFER_C64CA1CA_A1C6_4C83_8CF4_DBAF21D17EF7 */
