/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file input-buffer.c
 * @author T. Ostaszewski
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
 * @brief 
 * @details 
 */
#include "pcc.h"
#include "input-buffer.h"

#if defined __cplusplus
extern "C" {
#endif 

void copy_buffer(struct buffer_desc * p_output, struct buffer_desc * p_input, size_t data_size)
{
    /* Adjust for input buffer boundaries. */ 
    if (data_size > p_input->nMaxOffset_ - p_input->nCurrentOffset_)
    {
        data_size = p_input->nMaxOffset_ - p_input->nCurrentOffset_;
    }
    /* Adjust for output buffer boundaries. */ 
    if (data_size > p_input->nMaxOffset_ - p_output->nCurrentOffset_ + 1)
    {
        data_size = p_output->nMaxOffset_ - p_output->nCurrentOffset_;
	}
    memcpy(p_output->p_begin_ + p_output->nCurrentOffset_, p_input->p_begin_ + p_input->nCurrentOffset_, sizeof(uint8_t)*data_size);
    /* Advance the read pointer */
    p_input->nCurrentOffset_ 	+= data_size;
    p_output->nCurrentOffset_ 	+= data_size;
}

#if defined __cplusplus
}
#endif 

