#include "pcc.h"
#include "wave_utils.h"
#include "debug_helpers.h"

static const char * wFormatTag2String(WORD wFormatTag)
{
#define MAKE_STRUCT(x)	{x, #x}
	static const struct tag_wFormatTag2String {
		WORD wFormatTag_;
		const char * string_;
	} format2String_[] = {
		 MAKE_STRUCT(WAVE_FORMAT_PCM),
		 MAKE_STRUCT(WAVE_FORMAT_IEEE_FLOAT),
		 MAKE_STRUCT(WAVE_FORMAT_EXTENSIBLE),
		 MAKE_STRUCT(WAVE_FORMAT_ADPCM),
		 //MAKE_STRUCT(WAVE_FORMAT_XMA2),
		 //MAKE_STRUCT(WAVE_FORMAT_WMAUDIO2),
		 //MAKE_STRUCT(WAVE_FORMAT_WMAUDIO3),
	};
	size_t idx = 0;
	for (idx = 0; idx < sizeof(format2String_)/sizeof(format2String_[0]); ++idx)
	{
		if (format2String_[idx].wFormatTag_ == wFormatTag)
			return format2String_[idx].string_;
	}
	return "NULL";
}

void dump_waveformatex(WAVEFORMATEX const * p_wfe)
{
	debug_outputln("%s %d : {%hx, %s} %d %d %d %d %d %d", __FILE__, __LINE__, 
			p_wfe->wFormatTag, wFormatTag2String(p_wfe->wFormatTag),
			p_wfe->nChannels,
			p_wfe->nSamplesPerSec,
			p_wfe->nAvgBytesPerSec,
			p_wfe->nBlockAlign,
			p_wfe->wBitsPerSample,
			p_wfe->cbSize
			);
}

WORD waveformatex_getFormatTag(WAVEFORMATEX const * p_wfe)
{
	return p_wfe->wFormatTag;
}

DWORD waveformatex_getChannels(WAVEFORMATEX const * p_wfe)
{
	return p_wfe->nChannels;
}

DWORD waveformatex_getSamplesPerSec(WAVEFORMATEX const * p_wfe)
{
	return p_wfe->nSamplesPerSec;
}

DWORD waveformatex_getAvgBytesPerSec(WAVEFORMATEX const * p_wfe)
{
	return p_wfe->nAvgBytesPerSec;
}

WORD waveformatex_getBlockAlign(WAVEFORMATEX const * p_wfe)
{
	return p_wfe->nBlockAlign;
}

WORD waveformatex_getBitsPerSample(WAVEFORMATEX const * p_wfe)
{
	return p_wfe->wBitsPerSample;
}

/*!
 * 
 */
void copy_waveformatex_2_WAVEFORMATEX(WAVEFORMATEX * p_dest, const struct waveformatex * p_source)
{
	p_dest->wFormatTag 		= p_source->wFormatTag;
	p_dest->nChannels 		= p_source->nChannels;
	p_dest->nSamplesPerSec 	= p_source->nSamplesPerSec;
	p_dest->nAvgBytesPerSec = p_source->nAvgBytesPerSec;
	p_dest->nBlockAlign 	= p_source->nBlockAlign;
	p_dest->wBitsPerSample	= p_source->wBitsPerSample;
	p_dest->cbSize 			= sizeof(WAVEFORMATEX);
}

/*!
 * 
 */
int init_master_riff(PC_master_riff_chunk_t * pp_chunk, HINSTANCE hModule, LPCTSTR lpResName)
{
    HRSRC hRes;
    int result = -1;
    hRes = FindResource(hModule, lpResName, "0");
    if (NULL != hRes)
    {
        HGLOBAL globRes;
        globRes = LoadResource(hModule, hRes);
        if (NULL != hRes)
        {
            *pp_chunk = (PC_master_riff_chunk_t)LockResource(globRes);
            if (NULL != *pp_chunk)
                result = 0;
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
    return 0;
}

