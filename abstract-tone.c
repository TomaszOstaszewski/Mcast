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
    P_MASTER_RIFF_CONST mriff_; /*!< Pointer to the WAV file first bytes */
    TCHAR name_[MAX_PATH+1];
    HANDLE hf_;
    HANDLE mapping_; 
};

static void destroy_tone_impl(struct abstract_tone * p_tone)
{
    if (EXTERNAL_WAV_TONE == p_tone->e_type_ && p_tone->mriff_)
        UnmapViewOfFile(p_tone->mriff_);
    if (NULL != p_tone->mapping_)
        CloseHandle(p_tone->mapping_);
    if (INVALID_HANDLE_VALUE != p_tone->hf_)
        CloseHandle(p_tone->hf_);
}

struct abstract_tone * abstract_tone_create(tone_type_t eType, LPCTSTR psz_tone_name)
{
    struct abstract_tone * retval = (struct abstract_tone *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct abstract_tone));
    assert(retval);
    if (retval)
    {
        retval->hf_ = INVALID_HANDLE_VALUE;
        retval->e_type_ = eType;
        switch (eType)
        {
            case EXTERNAL_WAV_TONE :
                retval->hf_ = CreateFile(psz_tone_name, GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES) NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
                assert(INVALID_HANDLE_VALUE != retval->hf_);
                retval->mapping_ = CreateFileMapping(retval->hf_, NULL, PAGE_READONLY, 0, 0, NULL);
                assert(NULL != retval->mapping_);
                retval->mriff_ = (P_MASTER_RIFF_CONST)MapViewOfFile(retval->mapping_, FILE_MAP_READ, 0, 0, 0);
                assert(retval->mriff_);
                if (retval->mriff_)
                {
                    StringCchCopy(retval->name_, MAX_PATH+1, psz_tone_name);
                    return retval;
                }
                break;
            case EMBEDDED_TEST_TONE:
                if (init_master_riff(&retval->mriff_, NULL, psz_tone_name))
                    return retval;
                assert(0);
                break;
            default:
                assert(0);
                break;
        }
        if (retval)
        {
            destroy_tone_impl(retval);
        }
        HeapFree(GetProcessHeap(), 0, retval);
        retval = NULL;
    }
    return retval;
}

void abstract_tone_destroy(struct abstract_tone * p_tone)
{
    destroy_tone_impl(p_tone);
    HeapFree(GetProcessHeap(), 0, p_tone);
}

tone_type_t abstract_tone_get_type(struct abstract_tone * p_tone)
{
    return p_tone->e_type_;
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

size_t abstract_tone_dump(struct abstract_tone const * p_tone, LPTSTR pszBuffer, size_t size)
{
    HRESULT hr = S_OK;
    size_t retval = 0;
    switch (p_tone->e_type_)
    {
        case EMBEDDED_TEST_TONE:
            break;
        case EXTERNAL_WAV_TONE :
            hr = StringCchPrintf(pszBuffer, size, "%s", p_tone->name_);
            if (SUCCEEDED(hr))
            {
                hr = StringCchLength(pszBuffer, size, &retval); 
            }
            assert(SUCCEEDED(hr));
            break;
        default:
            assert(0);
            break;
    }
    if (SUCCEEDED(hr))
    {
        pszBuffer += retval;
        dump_pcmwaveformat(pszBuffer, size, abstract_tone_get_pcmwaveformat(p_tone));
    } 
    return retval;
}

