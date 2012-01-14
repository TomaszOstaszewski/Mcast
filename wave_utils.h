/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file wave_utils.h
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
 * @brief
 * @details
 */
#if !defined WAVE_UTILS_16606182_713B_4823_B290_4BE6CBA65CCC
#define WAVE_UTILS_16606182_713B_4823_B290_4BE6CBA65CCC

#include <windows.h>
#include <MMreg.h>
#include "std-int.h"

#if defined __cplusplus
extern "C" { 
#endif 

/*!
 * 
 */
typedef struct waveformatex {
    uint16_t  wFormatTag;
    uint16_t  nChannels;
    uint32_t  nSamplesPerSec;
    uint32_t  nAvgBytesPerSec;
    uint16_t  nBlockAlign;
    uint16_t  wBitsPerSample;
} waveformatex_t;

/*!
 * 
 */
typedef struct wav_subchunk { 
    uint8_t     subchunkId_[4];
    uint32_t    subchunk_size_; 
    union {
        int8_t      samples8_[1];
        int16_t     samples16_[1];
    };
} wav_subchunk_t;

/*!
 * 
 */
typedef struct wav_format_chunk {
    uint8_t     waveid_[4];
    uint8_t     ckid_[4];
    uint32_t    cksize_;
    struct waveformatex format_; 
    struct wav_subchunk subchunk_;
} wav_format_chunk_t; 

/*!
 * @brief 
 */
typedef struct master_riff_chunk {
    uint8_t     ckid_[4];
    uint32_t    cksize_;
    union {
        uint8_t     data_u8_[1];
        uint16_t    data_u16_[1];
        uint32_t    data_u32_[1];
        wav_format_chunk_t format_chunk_;
    };
} master_riff_chunk_t;

/*!
 * @brief
 */
typedef master_riff_chunk_t * P_master_riff_chunk_t;

/*!
 * @brief 
 */
typedef master_riff_chunk_t const * PC_master_riff_chunk_t;

void dump_waveformatex(WAVEFORMATEX const * p_wfe);

WORD waveformatex_getFormatTag(WAVEFORMATEX const * p_wfe);
DWORD waveformatex_getChannels(WAVEFORMATEX const * p_wfe);
DWORD waveformatex_getSamplesPerSec(WAVEFORMATEX const * p_wfe);
DWORD waveformatex_getAvgBytesPerSec(WAVEFORMATEX const * p_wfe);
WORD waveformatex_getBlockAlign(WAVEFORMATEX const * p_wfe);
WORD waveformatex_getBitsPerSample(WAVEFORMATEX const * p_wfe);

/*!
 * @brief 
 * @details
 */
void copy_waveformatex_2_WAVEFORMATEX(WAVEFORMATEX * p_dest, const struct waveformatex * p_source);

/*!
 * @brief Loads the WAV file from the resources.
 * @details  
 * @param[out] pp_chunk
 * @param[in] hModule
 * @param[in] lpResName
 * @return
 */
int init_master_riff(PC_master_riff_chunk_t * pp_chunk, HINSTANCE hModule, LPCTSTR lpResName);

#if defined __cplusplus
}
#endif 

#endif /* WAVE_UTILS_16606182_713B_4823_B290_4BE6CBA65CCC */
