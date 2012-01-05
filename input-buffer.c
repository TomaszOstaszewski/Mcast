/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file input-buffer.c
 * @author T. Ostaszewski
 * @date 04-Jan-2012
 * @brief 
 * @details 
 */
#include "pcc.h"
#include <assert.h>
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

struct buffer_desc * buffer_desc_create(unsigned char * p_begin, size_t nMaxOffset)
{
	struct buffer_desc * p_desc = (struct buffer_desc *)malloc(sizeof(struct buffer_desc));
	p_desc->p_begin_ = p_begin;
	p_desc->nMaxOffset_ = nMaxOffset;
	return p_desc;
}

void buffer_desc_destroy(struct buffer_desc * p_desc)
{
	free(p_desc);
}

#if defined __cplusplus
}
#endif 

