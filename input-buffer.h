/*!
 * @file input-buffer.h
 * @author T.Ostaszewski
 * @date 04-Jan-2012
 */
#if !defined INPUT_BUFFER_H_0C38BAD3_65A3_4F18_AA8C_0428AB3293D7
#define INPUT_BUFFER_H_0C38BAD3_65A3_4F18_AA8C_0428AB3293D7

#if defined __cplusplus
extern "C" { 
#endif

/**
 * @brief
 * @details
 */
struct buffer_desc {
    unsigned char * p_begin_;           /**!< */
    size_t      nCurrentOffset_;        /**!< */
    size_t      nMaxOffset_;            /**!< */
};

/**
 * @brief
 * @param[out]
 * @param[in]
 * @param[in]
 * @return
 */
//struct buffer_desc * buffer_desc_create(unsigned char * p_begin, size_t nMaxOffset);
//void buffer_desc_destroy(struct buffer_desc * );

/**
 * @brief
 * @param[out]
 * @param[in]
 * @param[in]
 * @return
 */
void copy_buffer(struct buffer_desc * p_output, struct buffer_desc * p_input, size_t data_size);

#if defined __cplusplus
}
#endif

#endif /* INPUT_BUFFER_H_0C38BAD3_65A3_4F18_AA8C_0428AB3293D7 */

