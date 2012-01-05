/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file fifo-circular-buffer.h
 * @author T.Ostaszewski
 * @date 04-Jan-2012
 * @brief
 * @details
 */
#if !defined FIFO_CIRCULAR_BUFFER_C64CA1CA_A1C6_4C83_8CF4_DBAF21D17EF7
#define FIFO_CIRCULAR_BUFFER_C64CA1CA_A1C6_4C83_8CF4_DBAF21D17EF7

#if defined __cplusplus
extern "C" {
#endif

/**
 * @brief
 * @details
 */
#define DATA_ITEM_SIZE (32*32)

typedef struct data_item {
    uint32_t    	count_;
    uint8_t     *	data_;
} data_item_t;

/**
 * @brief
 * @detail
 */
struct fifo_circular_buffer;

/**
 * @brief
 * @detail
 *
 */
struct fifo_circular_buffer *  fifo_circular_buffer_create(void);

/**
 * @brief
 * @detail
 *
 */
void fifo_circular_buffer_delete(struct fifo_circular_buffer * p_fifo);

/**
 * @brief
 * @detail
 *
 */
uint32_t fifo_circular_buffer_get_capacity(struct fifo_circular_buffer * p_fifo);

/**
 * @brief
 * @detail
 *
 */
uint32_t fifo_circular_buffer_get_items_count(struct fifo_circular_buffer * p_fifo);

/**
 * @brief
 * @detail
 */
int fifo_circular_buffer_is_free_space(struct fifo_circular_buffer * p_fifo);

/**
 * @brief
 * @detail
 */
unsigned int fifo_circular_buffer_is_full(struct fifo_circular_buffer * p_fifo);

/**
 * @brief
 * @detail
 */
int fifo_circular_buffer_push_item(struct fifo_circular_buffer * p_fifo, struct data_item const * p_item);

/**
 * @brief
 * @detail
 */
int fifo_circular_buffer_fetch_item(struct fifo_circular_buffer * p_fifo, struct data_item * p_item);

#if defined __cplusplus
}
#endif

#endif /* !defined FIFO_CIRCULAR_BUFFER_C64CA1CA_A1C6_4C83_8CF4_DBAF21D17EF7 */
