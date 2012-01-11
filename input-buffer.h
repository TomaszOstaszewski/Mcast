/*!
 * @file input-buffer.h
 * @brief Header file for safe buffer copy routines.
 * @author T.Ostaszewski
 * @date 04-Jan-2012
 */
#if !defined INPUT_BUFFER_H_0C38BAD3_65A3_4F18_AA8C_0428AB3293D7
#define INPUT_BUFFER_H_0C38BAD3_65A3_4F18_AA8C_0428AB3293D7

#if defined __cplusplus
extern "C" { 
#endif

/**
 * @brief Structure that describes buffer capabilities
 * @details This structure wraps 3 basic buffer parameters. Those are:
 * \li where does the buffer begin
 * \li what is its total size
 * \li how much data is already stored
 */
struct buffer_desc {
	/** @brief Pointer to the very first byte of the buffer.*/
    unsigned char * p_begin_;           
	/** @brief Describes how many data is already stored. */
    size_t      nCurrentOffset_;        
	/** @brief Describes how many bytes can be stored.*/
    size_t      nMaxOffset_;            
};

/**
 * @brief Copies data between 2 buffers described by their buffer_desc parameters
 * @param[out] p_output description of the input buffer 
 * @param[in] p_input description of the output buffer
 * @param[in] data_size how many bytes to copy from input buffer to the output buffer
 * @return
 */
void copy_buffer(struct buffer_desc * p_output, struct buffer_desc * p_input, size_t data_size);

#if defined __cplusplus
}
#endif

#endif /* INPUT_BUFFER_H_0C38BAD3_65A3_4F18_AA8C_0428AB3293D7 */

