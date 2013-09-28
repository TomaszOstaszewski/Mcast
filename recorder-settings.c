/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @brief Defines operatoins on the multicast sender settings data structure.
 * @file sender-settings.c
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
 * @date Jan-2012
 */
#include "pcc.h"
#include "recorder-settings.h"
#include "debug_helpers.h"

/*!
 * @brief Default number of bytes in the audio packet.
 */
#define DEFAULT_WAV_CHUNK_SIZE_MS (64)

/*!
 * @brief Default recorder's sample rate.
 */
#define DEFAULT_RECORDER_SAMPLE_RATE (11025)

/*!
 * @brief Default recorder's sample rate.
 */
#define DEFAULT_RECORDER_BITS_PER_SAMPLE (16)

/*!
 * @brief Default bytes per sample.
 */
#define DEFAULT_RECORDER_CHANNELS (1)

#define DEFAULT_RECORDER_SAMPLES_BUFFER_SIZE (4096)

/*!
 * @brief Defines defaults for WAVEFORMATEX structure
 */
#define DEFAULT_WAV_FORMAT  { WAVE_FORMAT_PCM, \
 DEFAULT_RECORDER_CHANNELS, \
 DEFAULT_RECORDER_SAMPLE_RATE, \
 DEFAULT_RECORDER_SAMPLE_RATE*DEFAULT_RECORDER_BITS_PER_SAMPLE/8,\
 DEFAULT_RECORDER_CHANNELS * DEFAULT_RECORDER_BITS_PER_SAMPLE/8, \
 DEFAULT_RECORDER_BITS_PER_SAMPLE, \
 0 }

/*!
 * @brief Describes the sender settings.
 */
struct recorder_settings {
    GUID capture_device_; /*!< */
    WAVEFORMATEX wavFormat_; /*!< */ 
    uint16_t samples_buffer_size_; /*!< */
};

static BOOL CALLBACK callback_enumerate_capture_devices(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{
    static __declspec(thread) char tmp_buffer[1024];
    if (NULL != lpGuid)
    {
        struct recorder_settings * p_settings;
        p_settings = (struct recorder_settings*)lpContext;
        CopyMemory(&p_settings->capture_device_, lpGuid, sizeof(GUID));
        debug_outputln("Module: %s, Description: %s", lpcstrDescription, lpcstrModule);
        //return TRUE; 
    }
    return TRUE;
}

static uint32_t chunk_size_ms_to_bytes(uint16_t ms)
{
    return (ms*DEFAULT_RECORDER_SAMPLE_RATE * DEFAULT_RECORDER_BITS_PER_SAMPLE)/8000;
}

static uint32_t chunk_size_bytes_to_ms(uint16_t bytes)
{
    return (1000*bytes)/(DEFAULT_RECORDER_SAMPLE_RATE*DEFAULT_RECORDER_BITS_PER_SAMPLE);
}

static int recorder_settings_init(struct recorder_settings * p_settings)
{
    WAVEFORMATEX def_wave_format = DEFAULT_WAV_FORMAT;
	HRESULT hr;
	hr = DirectSoundCaptureEnumerate((LPDSENUMCALLBACK)&callback_enumerate_capture_devices, p_settings);
	assert(SUCCEEDED(hr));
    CopyMemory(&p_settings->wavFormat_, &def_wave_format, sizeof(p_settings->capture_device_));
    p_settings->samples_buffer_size_ = DEFAULT_RECORDER_SAMPLES_BUFFER_SIZE;
    return 0;
}

int recorder_settings_validate(struct recorder_settings const * p_settings)
{
    return -1;
}

int recorder_settings_compare(struct recorder_settings const * p_left, struct recorder_settings const * p_right)
{
	return !memcmp(p_left, p_right, sizeof(struct recorder_settings));
}

void recorder_settings_copy(struct recorder_settings * p_dest, struct recorder_settings const * p_source)
{
    struct recorder_settings tmp;
    CopyMemory(&tmp, p_source, sizeof(struct recorder_settings));
	CopyMemory(p_dest, &tmp, sizeof(struct recorder_settings));
}

void recorder_settings_swap(struct recorder_settings * p_left, struct recorder_settings * p_right)
{
    struct recorder_settings tmp;
	CopyMemory(&tmp, p_left, sizeof(struct recorder_settings));
	CopyMemory(p_left, p_right, sizeof(struct recorder_settings));
	CopyMemory(p_right, &tmp, sizeof(struct recorder_settings));
}

GUID const * recorder_settings_get_guid(struct recorder_settings const * p_settings)
{
    return &p_settings->capture_device_;
}

uint16_t recorder_settings_get_samples_buffer_size(struct recorder_settings const * p_settings)
{
    return p_settings->samples_buffer_size_;
}

WAVEFORMATEX const * recorder_settings_get_waveformatex(struct recorder_settings const * p_settings)
{
    return &p_settings->wavFormat_;
}

recorder_settings_t recorder_settings_get_default(void)
{
    struct recorder_settings * p_retval = 
        (struct recorder_settings *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct recorder_settings));
    if (NULL != p_retval)
        recorder_settings_init(p_retval);
    return p_retval;
}

void recorder_settings_destroy(struct recorder_settings * p_settings)
{
    HeapFree(GetProcessHeap(), 0, p_settings);
}

