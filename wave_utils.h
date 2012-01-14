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
 * @brief WAV file description
 * @details This structure is binary compatible with WAV bytes, 16 bytes from the beginning.
 * It describes number of channels in WAV file, its format, how many samples per second, 
 * how many bytes per second, how many bits per sample and so on.
 */
typedef struct waveformatex {
    uint16_t  wFormatTag; /*!< Format tag */
    uint16_t  nChannels; /*!< Number of interleaved channels */
    uint32_t  nSamplesPerSec; /*!< Sampling rate, blocks per second */
    uint32_t  nAvgBytesPerSec; /*!< Data rate */
    uint16_t  nBlockAlign; /*!< Data block size */
    uint16_t  wBitsPerSample; /*!< Bits per sample */
} waveformatex_t;

/*!
 * @brief A sub-chunk header
 * @details This is the last header, what follows is data. 
 */
typedef struct wav_subchunk { 
    uint8_t     subchunkId_[4]; /*!< 4 bytes, they give string 'data', 0x64 0x61 0x74 0x61 */
    uint32_t    subchunk_size_;  /*!< Number of bytes that follow. */
    union {
        int8_t      samples8_[1];
        int16_t     samples16_[1];
    };
} wav_subchunk_t;

/*!
 * @brief WAV file header.
 * @details Each WAV file begins with this bytes. 
 * @attention All the data in the file is stored in little endian. Thus, on little endian machine, 
 * it will suffice to read a WAV file into that structure and then you have it. On the big endian machine, 
 * you will need to do a lot of byte swapping.
 */
typedef struct wav_format_chunk {
    uint8_t     waveid_[4]; /*!< WaveID, 4 bytes that give "WAVE", 0x57 0x41 0x56 0x45 */ 
    uint8_t     ckid_[4]; /*!< Yet another chunk id, 4 bytes that give "fmt ", 0x66 0x6d 0x74 0x20 */
    uint32_t    cksize_; /*!< Chunk size, either 16, 18 or 40 decimal */
    struct waveformatex format_; /*!< Format of the chunk */
    struct wav_subchunk subchunk_; /*!< The chunk itself, prepended with some rudimentary header. */
} wav_format_chunk_t; 

/*!
 * @brief WAV file header.
 * @details  
 * @attention All the data in the file is stored in little endian.
 */
typedef struct master_riff_chunk {
    uint8_t     ckid_[4]; /*!< ChunkID, 4 bytes that give "RIFF", 0x52 0x49 0x46 0x46 in hex */
    uint32_t    cksize_; /*!< Chunk size, 4+n */
    union {
        wav_format_chunk_t format_chunk_;
        uint8_t     data_u8_[1];
        uint16_t    data_u16_[1];
        uint32_t    data_u32_[1];
    };
} master_riff_chunk_t;

/*!
 * @brief Helper function, dumps the WAVEFORMATEX to the debug view window.
 */
void dump_waveformatex(WAVEFORMATEX const * p_wfe);

/*!
 * @brief Copies the waveformatex structure into a valid WAVEFORMATEX structure.
 * @details The layout of both waveformatex and WAVEFORMATEX is almost the same. The only difference
 * is that to get a pointer to 'waveformatex' it is sufficient to cast a pointer to memory mapped WAV file 
 * to the 'waveformatex *' and then you get it. The WAVEFORMATEX this trick will not work, as it has the 
 * cbSize member. 
 * In short, you will need this function. 
 * @param[out] p_dest pointer to the memory location which will be written with data copied from p_source parameter.
 * @param[in] p_source pointer to the memory location which will be written with data copied from p_source parameter.
 */
void copy_waveformatex_2_WAVEFORMATEX(WAVEFORMATEX * p_dest, const struct waveformatex * p_source);

/*!
 * @brief Loads the WAV file from the resources.
 * @details The WAV files are stored as binary data in the resources.  
 * @param[out] pp_chunk this pointer will be written with a reference to the memory location, where the WAV file is stored. 
 * Example usage:
 * @code
 * int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
 * {
 *     int result;
 *     master_riff_chunk_t * g_pWavChunk;
 *     result = init_master_riff(&g_pWavChunk, hInstance, MAKEINTRESOURCE(IDR_0_1));
 *     assert(0 == result);
 *     return (int)0;
 * }
 * @endcode
 * @param[in] hModule describes from which module resources will be loaded.
 * @param[in] lpResName describes what resource to load
 * @return returns 0 on success, <>0 otherwise.
 */
int init_master_riff(PC_master_riff_chunk_t * pp_chunk, HINSTANCE hModule, LPCTSTR lpResName);

#if defined __cplusplus
}
#endif 

#endif /* WAVE_UTILS_16606182_713B_4823_B290_4BE6CBA65CCC */
