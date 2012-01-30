/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file abstract-tone.c
 * @brief The abstraction of the tone to be played. The implementation.
 * @author T.Ostaszewski
 * @date Jan-2012
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
 */
#include "pcc.h"
#include "abstract-tone.h"
#include "wave_utils.h"

struct abstract_tone {
    tone_type_t e_type_; /*!< Type of the tone */
    master_riff_chunk_t * mriff_; /*!< Pointer to the WAV file first bytes */
};

struct abstract_tone * abstract_tone_create(tone_type_t eType, LPCTSTR psz_tone_name)
{
    struct abstract_tone * retval = (struct abstract_tone *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct abstract_tone));
    assert(retval);
    if (retval)
    {
        retval->e_type_ = eType;
        switch (eType)
        {
            case EMBEDDED_TEST_TONE:
                {
                    if (!init_master_riff(&retval->mriff_, NULL, psz_tone_name))
                    {
                        HeapFree(GetProcessHeap(), 0, retval);
                        retval = NULL;
                    }
                    assert(retval);
                }
                break;
            default:
                assert(0);
                break;
        }
    }
    return retval;
}

void abstract_tone_destroy(struct abstract_tone * p_tone)
{
    switch (p_tone->e_type_)
    {
        case EMBEDDED_TEST_TONE:
            HeapFree(GetProcessHeap(), 0, p_tone);
            break;
        case EXTERNAL_WAV_TONE:
            break;
        default:
            assert(0);
            break;
    }
}

PCMWAVEFORMAT const * abstract_tone_get_pcmwaveformat(struct abstract_tone const * p_tone)
{
    assert(p_tone);
    return &p_tone->mriff_->format_chunk_.format_;
}

void const * abstract_tone_get_wave_data(struct abstract_tone const * p_tone, size_t * p_data_size)
{
    assert(p_tone);
    *p_data_size = p_tone->mriff_->format_chunk_.subchunk_.subchunk_size_;
    return &p_tone->mriff_->format_chunk_.subchunk_.samples8_;
}

