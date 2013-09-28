/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file wave_utils.c
 * @brief Implementation of WAV file utilities.
 * @details
 * @author T.Ostaszewski
 * @date 04-Jan-2012
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
#include "wave_utils.h"
#include "debug_helpers.h"

typedef struct wavinoutcaps_dwFormat_2_textDescription {
	DWORD dwFormat_;
	const char * tag_;
	const char * text_desc_;
} wavinoutcaps_dwFormat_2_textDescription_t;

typedef wavinoutcaps_dwFormat_2_textDescription_t * P_WAVINOUTCAPS_DWFORMAT_2_TEXTDESCRIPTION;
typedef wavinoutcaps_dwFormat_2_textDescription_t const * P_CONST_WAVINOUTCAPS_DWFORMAT_2_TEXTDESCRIPTION;

typedef struct waveFormatTag_2_textDescription {
    WORD wFormatTag_;
    const char * string_;
} waveFormatTag_2_textDescription_t;

/*! 
 * @brief Macro to facilitate structure creation.
 */
#define MAKE_DWFORMAT_DESC(flag,text) { flag, #flag, text }

/*! 
 * @brief Macro to facilitate structure creation.
 */
#define MAKE_WFORMATTAG_DESC(flag) { flag, #flag }

static wavinoutcaps_dwFormat_2_textDescription_t const g_dwFormatDesc[] = {
	MAKE_DWFORMAT_DESC(WAVE_INVALIDFORMAT, "invalid format "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_1M08, " 11.025 kHz, Mono,   8-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_1S08, " 11.025 kHz, Stereo, 8-bit  "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_1M16, " 11.025 kHz, Mono,   16-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_1S16, " 11.025 kHz, Stereo, 16-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_2M08, " 22.05  kHz, Mono,   8-bit  "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_2S08, " 22.05  kHz, Stereo, 8-bit  "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_2M16, " 22.05  kHz, Mono,   16-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_2S16, " 22.05  kHz, Stereo, 16-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_4M08, " 44.1   kHz, Mono,   8-bit  "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_4S08, " 44.1   kHz, Stereo, 8-bit  "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_4M16, " 44.1   kHz, Mono,   16-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_4S16, " 44.1   kHz, Stereo, 16-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_44M08, " 44.1   kHz, Mono,   8-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_44S08, " 44.1   kHz, Stereo, 8-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_44M16, " 44.1   kHz, Mono,   16-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_44S16, " 44.1   kHz, Stereo, 16-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_48M08, " 48     kHz, Mono,   8-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_48S08, " 48     kHz, Stereo, 8-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_48M16, " 48     kHz, Mono,   16-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_48S16, " 48     kHz, Stereo, 16-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_96M08, " 96     kHz, Mono,   8-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_96S08, " 96     kHz, Stereo, 8-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_96M16, " 96     kHz, Mono,   16-bit "),
	MAKE_DWFORMAT_DESC(WAVE_FORMAT_96S16, " 96     kHz, Stereo, 16-bit "),
};

static waveFormatTag_2_textDescription_t const  g_wFormatTagDesc[] = {
    MAKE_WFORMATTAG_DESC(WAVE_FORMAT_PCM),
    MAKE_WFORMATTAG_DESC(WAVE_FORMAT_IEEE_FLOAT),
    MAKE_WFORMATTAG_DESC(WAVE_FORMAT_EXTENSIBLE),
    MAKE_WFORMATTAG_DESC(WAVE_FORMAT_ADPCM),
};

static int get_dwFormat_desc(DWORD dwFormat, P_CONST_WAVINOUTCAPS_DWFORMAT_2_TEXTDESCRIPTION * p_pointers_table, size_t pointers_table_size)
{
    size_t idx;    
    size_t out_idx;
    for (out_idx = 0, idx = 0; idx < COUNTOF_ARRAY(g_dwFormatDesc); ++idx)
    {
        if (dwFormat & g_dwFormatDesc[idx].dwFormat_)
        {
            p_pointers_table[out_idx] = &g_dwFormatDesc[idx];
        } 
    }
    return 0;
}

static const char * wFormatTag2String(WORD wFormatTag)
{
	size_t idx = 0;
	for (idx = 0; idx < COUNTOF_ARRAY(g_wFormatTagDesc); ++idx)
	{
		if (g_wFormatTagDesc[idx].wFormatTag_ == wFormatTag)
			return g_wFormatTagDesc[idx].string_;
	}
	return "NULL";
}

int dump_pcmwaveformat(char * psz_buffer, size_t buffer_size, pcmwaveformat_t const * p_wfe)
{
    HRESULT hr;
    hr = StringCchPrintf(psz_buffer, buffer_size, 
            "%-30s : %-10s\n"
            "%-30s : %u\n"
            "%-30s : %hu\n"
            "%-30s : %hu\n"
            "%-30s : %u\n"
            "%-30s : %hu\n",
            "Type", wFormatTag2String(p_wfe->wf.wFormatTag),
            "Sample rate [Hz]", p_wfe->wf.nSamplesPerSec, 
            "Bits per sample", p_wfe->wBitsPerSample,
            "Channels", p_wfe->wf.nChannels, 
            "Avg rate [bytes/sec]", p_wfe->wf.nAvgBytesPerSec, 
            "Block align", p_wfe->wf.nBlockAlign 
            );
    return SUCCEEDED(hr);
}

void copy_pcmwaveformat_2_WAVEFORMATEX(WAVEFORMATEX * p_dest, pcmwaveformat_t const * p_source)
{
	p_dest->wFormatTag 		= p_source->wf.wFormatTag;
	p_dest->nChannels 		= p_source->wf.nChannels;
	p_dest->nSamplesPerSec 	= p_source->wf.nSamplesPerSec;
	p_dest->nAvgBytesPerSec = p_source->wf.nAvgBytesPerSec;
	p_dest->nBlockAlign 	= p_source->wf.nBlockAlign;
	p_dest->wBitsPerSample	= p_source->wBitsPerSample;
	p_dest->cbSize 			= sizeof(WAVEFORMATEX);
}

void waveformat_normalize(WAVEFORMATEX * p_dest)
{
	p_dest->nAvgBytesPerSec = p_dest->nSamplesPerSec * (p_dest->wBitsPerSample/8) * p_dest->nChannels ;
}

int LoadWavFromResoure(P_MASTER_RIFF_CONST * pp_chunk, HINSTANCE hModule, LPCTSTR lpResName)
{
    HRSRC hRes;
    int result = 0;
    if (NULL == hModule)
        hModule = GetModuleHandle(NULL);
    hRes = FindResource(hModule, lpResName, "0");
    if (NULL != hRes)
    {
        HGLOBAL globRes;
        globRes = LoadResource(hModule, hRes);
        if (NULL != hRes)
        {
            *pp_chunk = (P_MASTER_RIFF_CONST)LockResource(globRes);
            if (NULL != *pp_chunk)
            {
                result = 1;
            }
        }
        else
        {
            debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, GetLastError(), GetLastError());
        }
    }
    else
    {
        debug_outputln("%s %5.5d : %10.10d %8.8x", __FILE__, __LINE__, GetLastError(), GetLastError());
    }
    return result;
}

int16_t const * get_wave_data(P_MASTER_RIFF_CONST p_master_riff)
{
    switch (p_master_riff->format_chunk_2_.wFormatTag_)
    {
        case WAVE_FORMAT_PCM:
            return &p_master_riff->format_chunk_2_.plain_wav_.subchunk_.samples16_[0];
            break;
        default:
            return NULL;
    }
}

uint32_t get_wave_data_size(P_MASTER_RIFF_CONST p_master_riff)
{
    switch (p_master_riff->format_chunk_2_.wFormatTag_)
    {
        case WAVE_FORMAT_PCM:
            return p_master_riff->format_chunk_2_.plain_wav_.subchunk_.subchunk_size_;
            break;
        default:
            return 0;
    }
}

void get_waveformat(P_MASTER_RIFF_CONST p_master_riff, WAVEFORMAT * p_output)
{
    
}

