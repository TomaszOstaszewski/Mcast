/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * 
 */

#include "pcc.h"
#include "directSound-caps.h"

typedef struct dscaps_dwflags_2_string {
    DWORD dwFlag_;
    const char * desc_;
    const char * elaborate_desc_;
} dscaps_dwflags_2_string_t;

#define MAKE_DWFLAGS_2_STRING(x,y) { x, #x, y }

typedef struct member_desc {
    size_t offset_;
    const char * desc_;
} member_desc_t;

#define MAKE_MEMBER_DESC(STRUCT,MEMBER) { offsetof(STRUCT,MEMBER), #MEMBER }

const member_desc_t g_dsbcaps_desc[] = {
    MAKE_MEMBER_DESC(DSCAPS, dwSize),
    MAKE_MEMBER_DESC(DSCAPS, dwFlags),
    MAKE_MEMBER_DESC(DSCAPS, dwMinSecondarySampleRate),
    MAKE_MEMBER_DESC(DSCAPS, dwMaxSecondarySampleRate),
    MAKE_MEMBER_DESC(DSCAPS, dwUnlockTransferRateHwBuffers),
    MAKE_MEMBER_DESC(DSCAPS, dwMaxHwMixingAllBuffers),
    MAKE_MEMBER_DESC(DSCAPS, dwFreeHwMixingAllBuffers),
    MAKE_MEMBER_DESC(DSCAPS, dwMaxHwMixingStaticBuffers),
    MAKE_MEMBER_DESC(DSCAPS, dwMaxHwMixingStreamingBuffers),
    MAKE_MEMBER_DESC(DSCAPS, dwFreeHwMixingStaticBuffers),
    MAKE_MEMBER_DESC(DSCAPS, dwFreeHwMixingStreamingBuffers),
    MAKE_MEMBER_DESC(DSCAPS, dwMaxHw3DAllBuffers),
    MAKE_MEMBER_DESC(DSCAPS, dwMaxHw3DStaticBuffers),
    MAKE_MEMBER_DESC(DSCAPS, dwMaxHw3DStreamingBuffers),
    MAKE_MEMBER_DESC(DSCAPS, dwFreeHw3DAllBuffers),
    MAKE_MEMBER_DESC(DSCAPS, dwFreeHw3DStaticBuffers),
    MAKE_MEMBER_DESC(DSCAPS, dwFreeHw3DStreamingBuffers),
    MAKE_MEMBER_DESC(DSCAPS, dwMaxContigFreeHwMemBytes),
    MAKE_MEMBER_DESC(DSCAPS, dwPlayCpuOverheadSwBuffers),
    MAKE_MEMBER_DESC(DSCAPS, dwTotalHwMemBytes),
    MAKE_MEMBER_DESC(DSCAPS, dwFreeHwMemBytes),
    MAKE_MEMBER_DESC(DSCAPS, dwPrimaryBuffers),
    MAKE_MEMBER_DESC(DSCAPS, dwReserved1),
    MAKE_MEMBER_DESC(DSCAPS, dwReserved2),
};

const dscaps_dwflags_2_string_t g_dscaps_dwflags_2_string_table[] = {
    MAKE_DWFLAGS_2_STRING(DSCAPS_CERTIFIED, "The driver has been tested and certified by Microsoft. This flag is always set for WDM drivers. To test for certification, use IDirectSound8::VerifyCertification."),  
    MAKE_DWFLAGS_2_STRING(DSCAPS_CONTINUOUSRATE, "The device supports all sample rates between the dwMinSecondarySampleRate and dwMaxSecondarySampleRate member values. Typically, this means that the actual output rate will be within +/- 10 hertz (Hz) of the requested frequency. "), 
    MAKE_DWFLAGS_2_STRING(DSCAPS_EMULDRIVER, "The device does not have a DirectSound driver installed, so it is being emulated through the waveform-audio functions. Performance degradation should be expected. "), 
    MAKE_DWFLAGS_2_STRING(DSCAPS_PRIMARY16BIT, "The device supports a primary buffer with 16-bit samples."),  
    MAKE_DWFLAGS_2_STRING(DSCAPS_PRIMARY8BIT, "The device supports primary buffers with 8-bit samples."),  
    MAKE_DWFLAGS_2_STRING(DSCAPS_PRIMARYMONO, "The device supports monophonic primary buffers."),  
    MAKE_DWFLAGS_2_STRING(DSCAPS_PRIMARYSTEREO, "The device supports stereo primary buffers."),  
    MAKE_DWFLAGS_2_STRING(DSCAPS_SECONDARY16BIT, "The device supports hardware-mixed secondary sound buffers with 16-bit samples."),  
    MAKE_DWFLAGS_2_STRING(DSCAPS_SECONDARY8BIT, "The device supports hardware-mixed secondary buffers with 8-bit samples."),  
    MAKE_DWFLAGS_2_STRING(DSCAPS_SECONDARYMONO, "The device supports hardware-mixed monophonic secondary buffers."),  
    MAKE_DWFLAGS_2_STRING(DSCAPS_SECONDARYSTEREO, "The device supports hardware-mixed stereo secondary buffers."), 
};

size_t get_caps_desc(DSCAPS const * p_caps, char * p_buffer, size_t dest_buffer_size)
{
    HRESULT hr;
    DWORD * p_member;
    size_t idx;
    char * p_current_buffer;
    size_t remaining_buffer_size;

    p_current_buffer = p_buffer;
    remaining_buffer_size = dest_buffer_size;

    for (idx = 0; idx < COUNTOF_ARRAY(g_dsbcaps_desc); ++idx)
    {
        p_member = (DWORD*)((unsigned char *)p_caps + g_dsbcaps_desc[idx].offset_);
        hr = StringCbPrintfEx(p_current_buffer, remaining_buffer_size, 
                &p_current_buffer, &remaining_buffer_size,
                STRSAFE_IGNORE_NULLS,
                "%s: %u\n ", 
                g_dsbcaps_desc[idx].desc_, *p_member);
        if (FAILED(hr))
            break;
    }
    return hr;
}

size_t get_caps_dwflags_desc(DSCAPS const * p_caps, char * p_buffer, size_t dest_buffer_size)
{
    HRESULT hr;
    DWORD dwFlagsCopy;
    size_t idx;
    char * p_current_buffer;
    size_t remaining_buffer_size;

    p_current_buffer = p_buffer;
    remaining_buffer_size = dest_buffer_size;

    dwFlagsCopy = p_caps->dwFlags;
    for (idx = 0; idx < COUNTOF_ARRAY(g_dscaps_dwflags_2_string_table); ++idx)
    {
        if (dwFlagsCopy & g_dscaps_dwflags_2_string_table[idx].dwFlag_)
        {
            dwFlagsCopy &= ~g_dscaps_dwflags_2_string_table[idx].dwFlag_;
            hr = StringCbPrintfEx(p_current_buffer, remaining_buffer_size, 
                &p_current_buffer, &remaining_buffer_size,
                STRSAFE_IGNORE_NULLS,
                 "0x%8.8x %s ", 
                g_dscaps_dwflags_2_string_table[idx].dwFlag_,
                g_dscaps_dwflags_2_string_table[idx].desc_);
            if (FAILED(hr))
                break;
        }
    }
    return 0;
}

