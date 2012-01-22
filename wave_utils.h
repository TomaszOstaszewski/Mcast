/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file wave_utils.h
 * @brief Utilities for handling WAV files.
 * @attention The description below is taken from <a href="https://ccrma.stanford.edu/courses/422/projects/WaveFormat/">Stanford's WAV file documentation</a>. All copyrights regarding the description 
 * belong to the creator of that page.
 * @details The WAVE file format is a subset of Microsoft's RIFF specification for the storage of multimedia files. 
 *
 * A RIFF file starts out with a file header followed by a sequence of data chunks. 
 *
 * A WAVE file is often just a RIFF file with a single "WAVE" chunk which consists of two sub-chunks -- a "fmt " chunk specifying 
 * the data format and a "data" chunk containing the actual sample data. Call this form the "Canonical form".  Who knows how it really all works. 
 * <pre> Offset  Size  Name             Description
 * The canonical WAVE format starts with the RIFF header:
 * 
 * 0         4   <b>ChunkID</b>          Contains the letters "RIFF" in ASCII form
 *                                (0x52494646 big-endian form).
 * 4         4   <b>ChunkSize</b>        36 + SubChunk2Size, or more precisely:
 *                                4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
 *                                This is the size of the rest of the chunk 
 *                                following this number.  This is the size of the 
 *                                entire file in bytes minus 8 bytes for the
 *                                two fields not included in this count:
 *                                ChunkID and ChunkSize.
 * 8         4   <b>Format</b>           Contains the letters "WAVE"
 *                                (0x57415645 big-endian form).
 * The "WAVE" format consists of two subchunks: "fmt " and "data":
 * The "fmt " subchunk describes the sound data's format:
 * 
 * 12        4   <b>Subchunk1ID</b>      Contains the letters "fmt "
 *                                (0x666d7420 big-endian form).
 * 16        4   <b>Subchunk1Size</b>    16 for PCM.  This is the size of the
 *                                rest of the Subchunk which follows this number.
 * 20        2   <b>AudioFormat</b>      PCM = 1 (i.e. Linear quantization)
 *                                Values other than 1 indicate some 
 *                                form of compression.
 * 22        2   <b>NumChannels</b>      Mono = 1, Stereo = 2, etc.
 * 24        4   <b>SampleRate</b>       8000, 44100, etc.
 * 28        4   <b>ByteRate</b>         == SampleRate * NumChannels * BitsPerSample/8
 * 32        2   <b>BlockAlign</b>       == NumChannels * BitsPerSample/8
 *                                The number of bytes for one sample including
 *                                all channels. I wonder what happens when
 *                                this number isn't an integer?
 * 34        2   <b>BitsPerSample</b>    8 bits = 8, 16 bits = 16, etc.
 *           2   <b>ExtraParamSize</b>   if PCM, then does not exist
 *           X   <b>ExtraParams</b>      space for extra parameters
 * 
 * The "data" subchunk contains the size of the data and the actual sound:
 * 
 * 36        4   <b>Subchunk2ID</b>      Contains the letters "data"
 *                                (0x64617461 big-endian form).
 * 40        4   <b>Subchunk2Size</b>    == NumSamples * NumChannels * BitsPerSample/8
 *                                This is the number of bytes in the data.
 *                                You can also think of this as the size
 *                                of the read of the subchunk following this 
 *                                number.
 * 44        *   <b>Data</b>             The actual sound data.
 * </pre>
 *
 *
 * <a name=example> </a>
 * <p> As an example, here are the opening 72 bytes of a WAVE file with
 * bytes shown as hexadecimal numbers:
 * 
 * <pre>
 * <b>52 49 46 46 24 08 00 00 57 41 56 45 66 6d 74 20 10 00 00 00 01 00 02 00 </b>
 * <b>22 56 00 00 88 58 01 00 04 00 10 00 64 61 74 61 00 08 00 00 00 00 00 00 </b>
 * <b>24 17 1e f3 3c 13 3c 14 16 f9 18 f9 34 e7 23 a6 3c f2 24 f2 11 ce 1a 0d </b>
 * </pre>
 * 
 * <p> Here is the interpretation of these bytes as a WAVE soundfile:
 * 
 * 
 * <h3> Notes: </h3>
 * 
 * <ul>
 * 
 * <li> The default byte ordering assumed for WAVE data files is little-endian.
 *  Files written using the big-endian byte ordering scheme have the identifier 
 *  RIFX instead of RIFF.
 * <li> The sample data must end on an even byte boundary. Whatever that means.
 * 
 * <li> 8-bit samples are stored as unsigned bytes, ranging from 0 to 255. 
 *      16-bit samples are stored as 2's-complement signed integers, 
 *      ranging from -32768 to 32767.
 * 
 * <li> There may be additional subchunks in a Wave data stream. If so,
 * each will have a char[4] SubChunkID, and unsigned long SubChunkSize,
 * and SubChunkSize amount of data.
 * 
 * <li> RIFF stands for <i>Resource Interchange File Format</i>.
 * 
 * </ul>
 * 
 * 
 * <h3> General discussion of RIFF files: </h3>
 * 
 * Multimedia applications require the storage and management of a
 * wide variety of data, including bitmaps, audio data, video data,
 * and peripheral device control information. RIFF provides a 
 * way to store all these varied types of data. The type of data a
 * RIFF file contains is indicated by the file extension. Examples of
 * data that may be stored in RIFF files are:
 * 
 * <ul>
 * <li> Audio/visual interleaved data (.AVI)
 * <li> Waveform data (.WAV)
 * <li> Bitmapped data (.RDI)
 * <li> MIDI information (.RMI)
 * <li> Color palette (.PAL)
 * <li> Multimedia movie (.RMN)
 * <li> Animated cursor (.ANI)
 * <li> A bundle of other RIFF files (.BND)
 * </ul>
 * 
 * NOTE:
 * 
 * At this point, AVI files are the only type of RIFF files that have
 * been fully implemented using the current RIFF specification. Although
 * WAV files have been implemented, these files are very simple, and
 * their developers typically use an older specification in constructing
 * them.
 * 
 * <p>
 * For more info see 
 * <a target=_top href=http://www.ora.com/centers/gff/formats/micriff/index.htm>
 * http://www.ora.com/centers/gff/formats/micriff/index.htm</a>
 * 
 * 
 * <h3> References: </h3>
 * 
 * <ol>
 * 
 * <li> <a target=_top href=http://netghost.narod.ru/gff/graphics/summary/micriff.htm>
 * http://netghost.narod.ru/gff/graphics/summary/micriff.htm</a> RIFF Format Reference (good).
 * 
 * <li> <a target=_top href=http://www.lightlink.com/tjweber/StripWav/WAVE.html>
 * http://www.lightlink.com/tjweber/StripWav/WAVE.html </a>
 * 
 * </ol>
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
#if !defined WAVE_UTILS_16606182_713B_4823_B290_4BE6CBA65CCC
#define WAVE_UTILS_16606182_713B_4823_B290_4BE6CBA65CCC

#include <windows.h>
#include <mmreg.h>
#include "std-int.h"

#if defined __cplusplus
extern "C" { 
#endif 

/*!
 * @brief WAV file description
 * @details This structure is binary compatible with WAV bytes, 16 bytes from the beginning.
 * It describes number of channels in WAV file, its format, how many samples per second, 
 * how many bytes per second, how many bits per sample and so on.
 * @attention All the data in the WAV file is stored in little endian. So on the little endian machine 
 * it is enough to load a WAV file into this structure.
 * @attention This structure is exactly the same as the PCMWAVEFORMAT structure from MMReg.h header file, provided 
 * by WinAPI.
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
 * @attention All the data in the WAV file is stored in little endian. So on the little endian machine 
 * it is enough to load a WAV file into this structure.
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
 * @param[in] pp_chunk this pointer will be written with a pointer value (because its a pointer to a pointer). The pointer value, in turn
 * will point to the WAV file data.
 * @param[in] hModule describes from which module resources will be loaded.
 * @param[in] lpResName describes what resource to load
 * @return returns 0 on success, <>0 otherwise.
 */
int init_master_riff(master_riff_chunk_t ** pp_chunk, HINSTANCE hModule, LPCTSTR lpResName);

#if defined __cplusplus
}
#endif 

#endif /* WAVE_UTILS_16606182_713B_4823_B290_4BE6CBA65CCC */
