/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file dsoundplay.cpp
 * @brief DirectSound WAV playing file.
 * @details To begin with, let's start with the general idea of DirectSound playback. 
 * Let's ignore for the moment other important DirectSound topics and focus on a main subject. 
 * The main subject is that DirectSound uses an array of bytes for playback. That's it. There's nothing more to it. The whole issue of the playback
 * is to alter the contents of that array so the playback goes smoothly.

 * For the moment let's assume, that we have this buffer and it's 8192 bytes long. Then we tell the DirectSound subsystem to playback the data from this buffer.
 * The DirectSound will consequently ready bytes form that buffer and feed them to the sound card. This is a sequential process, i.e. not the entire 
 * buffer will be consumed at time. Instead, we may query DirectSound which part of the buffer is currently accessed by the sound card hardware. 
 * This part, to which the hardware has an access, is off-limits for us. However, with the rest of the buffer we may do as we please. 
 * This query is called a GetCurrentPosition() in DirectSound lingo, and the outputs are so called cursors positions. There are 2 cursors, the read cursor and 
 * the write cursor. What is there from read to write cursor, is off-limits. What is there from write to read cursor - we may change. The gab between
 * the read and write is the guard period. This guard period shall give us sufficient time to fill the remaining part of the buffer before the time will come
 * for us to play it.
 *
 * So imagine that we have a 10ms timer ticking, and with each timer tick we check the cursors. Here's an example output:
 * \code
 *   0ms Read:    0 bytes, Write:  480 bytes
 *
 *  10ms Read:  380 bytes, Write:  860 bytes 
 *
 *  20ms Read:  760 bytes, Write: 1240 bytes 
 *
 *  ...
 *
 * 210ms Read: 7980 bytes, Write:  268 bytes
 * \endcode
 *
 * As can be easily seen, cursors wrap around. Therefore, a small buffer, of couple kB long, can be used to play much larger files, provided we alter the contents
 * of the buffer fast enough.
 *
 * What we may change, is the area from the write cursor to the read cursor. The area from read to write cursor, the guard period, should not be changed. 
 * The are is about 480 bytes long on my PC. Assuming 8000 Hz sampling rate and 2 bytes per sample, this yields 30 milliseconds to fill the next chunk of the buffer.
 * Even given the medicore realtime capabilities of the Windows platform, this timespan usually suffices to successfully complete this task.
 *
 * With clever buffer organization, one can buy himself much more time, trading it for space. Let's assume that the 8192 bytes long buffer is divided into 2 chunks of 4096 bytes each.
 * We we notice that both the read and write currsor are in the range from 0 to 4095 bytes inclusive, then we fill entire 2nd chunk starting from 4096 up to 8191.
 * If both cursors move to range from 4096 to 8191 inclusive, we fill entire 1st chunk starting from 0 up to 4095.
 * Given the same 8000 samples per second and 2 bytes per sample it gives us about 256 milliseconds time to fill next chunk. This is more than enough for even low end machine.
 *
 * @author T. Ostaszewski
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
 * @date 30-Nov-2011
 */
#include "pcc.h"
#include "debug_helpers.h"
#include "dsoundplay.h"
#include "wave_utils.h"
#include "fifo-circular-buffer.h"
#include "input-buffer.h"
#include "receiver-settings.h"

/*!
 * @brief Number of chunks in the DirectSound secondary buffer.
 */
#define NUMBER_OF_BUFFERS (4)

/*!
 * @brief Indicates that a chunk is being played and can be filled after playing is done.
 * @details If a chunk is marked with <b>BUFFER_PLAYED</b>, it can be filled again with new data, as soon as the DirectSound
 * player is done with that buffer.
 */
#define BUFFER_PLAYED (0x00000000)

/*!
 * @brief Indicates that a chunk is filled and can be played.
 * @details To be frank, even if a chunk is not marked as <b>BUFFER_FILLED</b>, it will be played.
 * 
 * The real purpose of having the <b>BUFFER_FILLED</b> is the to avoid the situation, when we fill the same chunk over and over again.
 */
#define BUFFER_FILLED (0xffffffff)

/*!
 * @brief The sound player descriptor. 
 * @details Gathers all the variables needed to successfully play chunks of PCM
 * data using DirectSound.
 */
struct dsound_data {
    WAVEFORMATEX wfe_;
    LPDIRECTSOUND8          p_direct_sound_8_;              /*!< The DirectSound Object. */
    LPDIRECTSOUNDBUFFER     p_primary_sound_buffer_;        /*!< The DirectSound primary buffer. */
    LPDIRECTSOUNDBUFFER8    p_secondary_sound_buffer_;      /*!< The DirectSound secondary buffer. */
    MMRESULT timer_;                                        /*!< Multimedia timer that feeds the data to the buffers. */
    DWORD   buffer_markers_[NUMBER_OF_BUFFERS];             /*!< Array of markers whether a buffer has been filled or played. */
    size_t                  nSingleBufferSize_;               /*!< Size of a single buffer. */
    struct fifo_circular_buffer * fifo_;	/*!< A fifo queue - from that queue we fetch the data and feed to the buffers.*/
    struct play_settings play_settings_;	/*!< Settings for our player (how many bytes per buffer, timer frequency).*/
};

/*!
 * @brief Macro that creates a DirectSound buffer descriptor structures.
 * @details This is here to make the creation of a table of structures a bit easier. 
 * Instead of writting:
 * @code
 * static struct flag_2_desc {
 *  DWORD flag_;
 *  LPCTSTR desc_;
 * } flags_to_descs[] = {
 *  { DSBCAPS_PRIMARYBUFFER, "DSBCAPS_PRIMARYBUFFER" },
 * };
 * @endcode
 * one writes:
 * @code
 * static struct flag_2_desc {
 *     DWORD flag_;
 *     LPCTSTR desc_;
 * } flags_to_descs[] = {
 *     MAKE_FLAG_2_DESC(DSBCAPS_PRIMARYBUFFER),
 * };
 * @endcode
 */
#define MAKE_FLAG_2_DESC(x) { x, #x }

/*!
 * @brief Maps DWORD flag to its textual counterpart.
 */
struct dword_2_desc {
    DWORD flag_; /*!< A flag value.*/
    LPCTSTR desc_; /*!< Flag's textual description. */
};
 
/*!
 * @brief Description of the DirectSound buffer flags.
 */
static struct dword_2_desc flags_to_descs[] = {
    MAKE_FLAG_2_DESC(DSBCAPS_PRIMARYBUFFER),
    MAKE_FLAG_2_DESC(DSBCAPS_STATIC),
    MAKE_FLAG_2_DESC(DSBCAPS_LOCHARDWARE),
    MAKE_FLAG_2_DESC(DSBCAPS_LOCSOFTWARE),
    MAKE_FLAG_2_DESC(DSBCAPS_CTRL3D),
    MAKE_FLAG_2_DESC(DSBCAPS_CTRLFREQUENCY),
    MAKE_FLAG_2_DESC(DSBCAPS_CTRLPAN),
    MAKE_FLAG_2_DESC(DSBCAPS_CTRLVOLUME),
    MAKE_FLAG_2_DESC(DSBCAPS_CTRLPOSITIONNOTIFY),
    MAKE_FLAG_2_DESC(DSBCAPS_CTRLFX),
    MAKE_FLAG_2_DESC(DSBCAPS_STICKYFOCUS),
    MAKE_FLAG_2_DESC(DSBCAPS_GLOBALFOCUS),
    MAKE_FLAG_2_DESC(DSBCAPS_GETCURRENTPOSITION2),
    MAKE_FLAG_2_DESC(DSBCAPS_MUTE3DATMAXDISTANCE),
    MAKE_FLAG_2_DESC(DSBCAPS_LOCDEFER),
    { 0x00080000, "DSBCAPS_TRUEPLAYPOSITION" },
};

/**
 * @brief Helper routine, gets the device capabilities.
 * @param[in] lpdsb pointer to the direct sound buffer, either primary or secondary.
 * @return returns S_OK if succeeded, any other value indicates an error.
 * @sa http://bit.ly/zP10oa
 */
static HRESULT get_buffer_caps(LPDIRECTSOUNDBUFFER lpdsb)
{
    DSBCAPS caps;
    HRESULT hr;
    ZeroMemory(&caps, sizeof(DSBCAPS));
    caps.dwSize = sizeof(DSBCAPS);
    hr = lpdsb->GetCaps(&caps);
    if (SUCCEEDED(hr))
    {
        size_t index;
        
        debug_outputln("%s %4.4u : %8.8x %8.8u %8.8u %8.8u"
                ,__FILE__, __LINE__
                ,caps.dwFlags
                ,caps.dwBufferBytes
                ,caps.dwUnlockTransferRate
                ,caps.dwPlayCpuOverhead
                );    
        for (index = 0; index <sizeof(flags_to_descs)/sizeof(flags_to_descs[0]); ++index)
        {
            if (caps.dwFlags & flags_to_descs[index].flag_)
                debug_outputln("%s %4.4u : %s"
                    ,__FILE__, __LINE__
                    ,flags_to_descs[index].desc_);
         }
    }
    else
    {
        debug_outputln("%s %4.4u : %8.8x", __FILE__, __LINE__, hr);    
    }
    return hr;
}

/**
 * @brief Creates the primary buffer and the secondary buffers.
 * @param[in] p_direct_sound_8 pointer to the IDirectSound8 interface. This interface is a factory for creating both the primary buffer and the secondary buffers.
 * @param[out] pp_primary_buffer pointer to the memory location which will be written with the primary buffer interface pointer.
 * @param[out] pp_secondary_buffer pointer to the memory location which will be written with the secondary buffer interface pointer. 
 * @param[in] p_wfe pointer to the WAVEFORMATEX structure. This parameter defines the buffer format (number of samples per second, how many bytes per sample and so on).
 * @param[in] single_buffer_size size of the single play buffer
 * @return
 */
static HRESULT create_buffers(LPDIRECTSOUND8 p_direct_sound_8, LPDIRECTSOUNDBUFFER * pp_primary_buffer, LPDIRECTSOUNDBUFFER8 * pp_secondary_buffer, 
        WAVEFORMATEX * p_wfe, size_t single_buffer_size)
{
    HRESULT hr;
    DSBUFFERDESC bufferDesc;
    LPDIRECTSOUNDBUFFER lpDSB = NULL;
    if (NULL == pp_primary_buffer || NULL == pp_secondary_buffer || NULL != *pp_primary_buffer || NULL != *pp_secondary_buffer)
    {
        debug_outputln("%s %4.4u : %p %p %p %p", __FILE__, __LINE__, pp_primary_buffer, pp_secondary_buffer, *pp_primary_buffer, *pp_secondary_buffer);
        return E_INVALIDARG;
    }
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.dwSize = sizeof(bufferDesc);
    bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER; 
    bufferDesc.guid3DAlgorithm = DS3DALG_DEFAULT;
    /* All others must be null for primary buffer */
    hr = p_direct_sound_8->CreateSoundBuffer(&bufferDesc, pp_primary_buffer, NULL);
    if (FAILED(hr))
    {
        debug_outputln("%s %4.4u : %x", __FILE__, __LINE__, hr);
        goto error;
    }
    hr = (*pp_primary_buffer)->SetFormat(p_wfe);
    if (FAILED(hr))
    {
        debug_outputln("%s %4.4u : %8.8x", __FILE__, __LINE__, hr);
        goto error;
    }
    get_buffer_caps(*pp_primary_buffer);
    /* Secondary buffer */
    bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
    bufferDesc.dwBufferBytes = NUMBER_OF_BUFFERS*single_buffer_size; /* double buffering - one buffer being played, whereas the other is being filled in */
    bufferDesc.lpwfxFormat = p_wfe;
    hr = p_direct_sound_8->CreateSoundBuffer(&bufferDesc, &lpDSB, NULL);
    if (FAILED(hr))
    {
        debug_outputln("%s %4.4u : %x", __FILE__, __LINE__, hr);
        goto error;
    }
    hr = lpDSB->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)pp_secondary_buffer);
    if (FAILED(hr))
    {
        debug_outputln("%s %4.4u : %x", __FILE__, __LINE__, hr);
        goto error;
    }
    get_buffer_caps(*pp_secondary_buffer);
    if (NULL != lpDSB)
    {
        lpDSB->Release();
        lpDSB = NULL;
    }
    return hr;
error:
    if (NULL != *pp_secondary_buffer)
    {
        (*pp_secondary_buffer)->Release();  
        *pp_secondary_buffer = NULL;    
    }
    if (NULL != *pp_primary_buffer)
    {
        (*pp_primary_buffer)->Release();
        *pp_primary_buffer = NULL;
    }
    if (NULL != lpDSB)
    {
        lpDSB->Release();
        lpDSB = NULL;
    }
    return hr;
}

/*!
 * @brief Sets up the DirectSound for playback
 * @details Example code:
 * @code
 * HRESULT hr;
 * struct dsound_data * p_retval = (struct dsound_data*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct dsound_data));
 * if (NULL != p_retval)
 * {
 *     hr = init_ds_data(hWnd, p_WFE, p_retval);
 *     if (SUCCEEDED(hr))
 *     {
 *         return (DSOUNDPLAY)(p_retval);
 *     }
 * }
 * debug_outputln("%s %4.4u", __FILE__, __LINE__);
 * HeapFree(GetProcessHeap(), 0, p_retval);
 * return NULL;
 * @endcode
 * @param[in] hwnd handle to the window that goes into the call of IDirectSound::SetCooperationLevel. Can be NULL, in which case either
 * a foreground window or the desktop window will be used. See <a href="http://msdn.microsoft.com/en-us/library/ms898135.aspx">this link</a> for more information.
 * @param[in] p_WFE pointer to the WAVEFORMATEX structure, describing the data to be played.
 * @param[out] p_ds_data refernce to the structure, whose members will be filled with DirectSound interface pointers.
 * @retrun returns the status of the operation, test it with SUCCEEDED() or FAILED() macros. 
 */
static HRESULT init_ds_data(HWND hwnd, WAVEFORMATEX const * p_WFE, struct dsound_data * p_ds_data)
{
    HRESULT hr;
    hr = DirectSoundCreate8(&DSDEVID_DefaultVoicePlayback, &p_ds_data->p_direct_sound_8_, NULL);
    if (FAILED(hr))
    {
        debug_outputln("%s %4.4u : %x", __FILE__, __LINE__, hr);
        goto error;
    }
    if (NULL == hwnd)
    {
        hwnd = GetForegroundWindow();
    }
    if (NULL == hwnd)
    {
        hwnd = GetDesktopWindow();
    }
    hr = p_ds_data->p_direct_sound_8_->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
    if (FAILED(hr))
    {
        debug_outputln("%s %4.4u : %x", __FILE__, __LINE__, hr);
        goto error;
    }
    CopyMemory(&p_ds_data->wfe_, p_WFE, sizeof(WAVEFORMATEX));
    p_ds_data->nSingleBufferSize_ = p_ds_data->play_settings_.play_buffer_size_;
    hr = create_buffers(p_ds_data->p_direct_sound_8_, &p_ds_data->p_primary_sound_buffer_, &p_ds_data->p_secondary_sound_buffer_, &p_ds_data->wfe_, p_ds_data->nSingleBufferSize_);
    if (FAILED(hr))
    {
        debug_outputln("%s %4.4u : %x", __FILE__, __LINE__, hr);
        goto error;
    }
    return hr;
error:
    if (NULL != p_ds_data->p_direct_sound_8_)
    {
        p_ds_data->p_direct_sound_8_->Release();
        p_ds_data->p_direct_sound_8_ = NULL;
    }
    return hr;
}


/**
 * @brief Copy the data from "userspace" to the place from which they will be played on the speakers.
 * @param[in] dwOffset the offset, from the beginning of the buffer to the first byte of the buffer to be filled. 
 * @param[in] req_size number of bytes to fill the buffer with.
 * @param[in] p_buffer buffer to be filled. 
 * @param[in] p_input_buffer_desc from this source we will fetch the data
 * @return returns S_OK on success, any other result indicates a failure.
 */
static HRESULT fill_buffer(DWORD dwOffset, DWORD req_size, LPDIRECTSOUNDBUFFER8 p_buffer, struct buffer_desc * p_input_buffer_desc)
{
    LPVOID lpvWrite1;
    DWORD dwLength1;
    HRESULT hr;
    hr = p_buffer->Lock(dwOffset, // Offset at which to start lock.
            req_size,               // Size of lock;
            (LPVOID*)&lpvWrite1,    // Gets address of first part of lock.
            &dwLength1,             // Gets size of first part of lock.
            NULL, /* Second part not needed as we will never wrap around - we lock equal buffer chunks */
            NULL,
            0);                     // Flag.
    if (SUCCEEDED(hr))
    {
        struct buffer_desc output_desc;
        output_desc.p_begin_    = (unsigned char *)lpvWrite1;
        output_desc.nMaxOffset_ = dwLength1;
        output_desc.nCurrentOffset_ = 0;
        copy_buffer(&output_desc, p_input_buffer_desc, dwLength1);
        /* If output buffer was not filled completely, fill rest with zeros */
        if (output_desc.nCurrentOffset_ < output_desc.nMaxOffset_)
        {
            ZeroMemory(output_desc.p_begin_ + output_desc.nCurrentOffset_, output_desc.nMaxOffset_ - output_desc.nCurrentOffset_ + 1);
            p_input_buffer_desc->nCurrentOffset_ = 0;
        }
        /* If input buffer exhausted, wrap it around */
        if (p_input_buffer_desc->nCurrentOffset_ == p_input_buffer_desc->nMaxOffset_)
        {
            p_input_buffer_desc->nCurrentOffset_ = 0;
        }
        hr = p_buffer->Unlock(lpvWrite1, dwLength1, NULL, 0);
    }
    else
    {
        debug_outputln("%s %4.4u : %8.8x", __FILE__, __LINE__, hr);
    }
    return hr;
}

/*!
 * @brief Fills the secondary buffer with audio samples.
 * @param[in] p_ds_data pointer to the player descriptor.
 */
static void play_data_chunk(struct dsound_data * p_ds_data) 
{
    DWORD dwRead, dwWrite;
    HRESULT hr;
    hr = p_ds_data->p_secondary_sound_buffer_->GetCurrentPosition(&dwRead, &dwWrite);
    if (SUCCEEDED(hr))
    {
        struct data_item data;
        struct buffer_desc buf_desc;
        int read_buf_index, write_buf_index;
        data.data_ = (uint8_t*)alloca(p_ds_data->nSingleBufferSize_);
        data.count_ = p_ds_data->nSingleBufferSize_;
        buf_desc.p_begin_ = data.data_;
        buf_desc.nMaxOffset_ = p_ds_data->nSingleBufferSize_;
        buf_desc.nCurrentOffset_ = 0;
        read_buf_index = dwRead / p_ds_data->nSingleBufferSize_;
        write_buf_index = dwWrite / p_ds_data->nSingleBufferSize_;
        /* Check if both write & read cursor point to the same buffer */
        if (read_buf_index == write_buf_index)
        {
            int next_buf_index = (read_buf_index + 1) % NUMBER_OF_BUFFERS;
            assert(next_buf_index < NUMBER_OF_BUFFERS); /* 0 means - buffer not yet filled */
            assert(read_buf_index < NUMBER_OF_BUFFERS);
            if (BUFFER_PLAYED == p_ds_data->buffer_markers_[next_buf_index])
            {
                if (fifo_circular_buffer_get_items_count(p_ds_data->fifo_)>0)
                {
                    fifo_circular_buffer_fetch_item(p_ds_data->fifo_, &data);
                }
                else
                {
                    memset(data.data_, 0, sizeof(uint8_t)*data.count_);
                }
                hr = fill_buffer(next_buf_index * (p_ds_data->nSingleBufferSize_), p_ds_data->nSingleBufferSize_, p_ds_data->p_secondary_sound_buffer_, &buf_desc);
                p_ds_data->buffer_markers_[next_buf_index] = BUFFER_FILLED;
            }
            p_ds_data->buffer_markers_[read_buf_index] = BUFFER_PLAYED;
        }
    }   
    else
    {
        debug_outputln("%s %4.4u : %8.8x", __FILE__, __LINE__, hr);
    }
    return;
}

/*!
 * @brief A player timer callback.
 * @details This callback fires once a while and checks if there is new data to be played.
 * if so, this new data is placed in the secondary buffer and then automagically played.
 */
static void CALLBACK sTimerCallback(UINT uTimerID, UINT uMsg, DWORD dwUser, 
        DWORD dw1 /* reserved - do not use */, 
        DWORD dw2 /* reserved * - do not use */) 
{
    struct dsound_data * p_ds_data = (struct dsound_data *)dwUser;
    play_data_chunk(p_ds_data);
}

extern "C" DSOUNDPLAY dsoundplayer_create(HWND hWnd, struct receiver_settings const * p_settings, struct fifo_circular_buffer * fifo)
{
    struct dsound_data * p_retval = 
		(struct dsound_data*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct dsound_data));
    if (NULL != p_retval)
    {
        HRESULT hr;
        p_retval->fifo_ = fifo;
        play_settings_copy(&p_retval->play_settings_, &p_settings->play_settings_);
        hr = init_ds_data(hWnd, &p_settings->wfex_, p_retval);
        if (SUCCEEDED(hr))
        {
            return (DSOUNDPLAY)(p_retval);
        }
        HeapFree(GetProcessHeap(), 0, p_retval);
        p_retval = NULL;
    }
    return NULL;
}

extern "C" void dsoundplayer_destroy(DSOUNDPLAY handle) 
{
    struct dsound_data * p_ds_data = (struct dsound_data*)handle;
    p_ds_data->p_secondary_sound_buffer_->Release();
    p_ds_data->p_primary_sound_buffer_->Release();
    p_ds_data->p_direct_sound_8_->Release();
    HeapFree(GetProcessHeap(), 0, p_ds_data);
}

extern "C" int dsoundplayer_play(DSOUNDPLAY handle) 
{
    struct dsound_data * p_ds_data = (struct dsound_data*)handle;
	struct play_settings const * p_set = &p_ds_data->play_settings_;
    LPDIRECTSOUNDBUFFER8 lpdsbStatic = p_ds_data->p_secondary_sound_buffer_;
    p_ds_data->timer_ = timeSetEvent(
		p_set->timer_delay_, 
		p_set->timer_resolution_, 
		&sTimerCallback, (DWORD)p_ds_data, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
    if (NULL != p_ds_data->timer_)
    {
        HRESULT hr;
        hr = lpdsbStatic->SetCurrentPosition(0);
        if (SUCCEEDED(hr))
        {
            hr = lpdsbStatic->Play( 0, 0, DSBPLAY_LOOPING);
            if (SUCCEEDED(hr))
            {
                return 1;
            }
            debug_outputln("%s %4.4u : %8.8x", __FILE__, __LINE__, hr);
        }
        debug_outputln("%s %4.4u : %8.8x", __FILE__, __LINE__, hr);
    }
    debug_outputln("%s %4.4u", __FILE__, __LINE__);
    return 0;
}

extern "C" void dsoundplayer_pause(DSOUNDPLAY handle) 
{
}

extern "C" int dsoundplayer_stop(DSOUNDPLAY handle) 
{
    int result = 0;
    struct dsound_data * p_ds_data = (struct dsound_data*)handle;
    LPDIRECTSOUNDBUFFER8 lpdsbStatic = p_ds_data->p_secondary_sound_buffer_;
    HRESULT hr = lpdsbStatic->Stop();
    MMRESULT res = timeKillEvent(p_ds_data->timer_);
    if (SUCCEEDED(hr) && TIMERR_NOERROR == res)
        result = 1;
    return result;
}

