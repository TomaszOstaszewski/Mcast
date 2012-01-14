/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file input-buffer.h
 * @brief Header file for safe buffer copy routines.
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

