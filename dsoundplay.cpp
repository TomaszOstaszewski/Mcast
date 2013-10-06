/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file dsoundplay.cpp
 * @brief DirectSound WAV playing file.
 * @details To begin with, let's start with the general idea of DirectSound playback. 
 * Let's ignore for the moment other important DirectSound topics and focus on a main subject. 
 * The main subject is that DirectSound uses an array of bytes for playback. That's it. There's nothing more to it. The whole issue of the playback
 * is to alter the contents of that array so the playback goes smoothly.

 * For the moment let's assume, that we have this buffer and it's 8192 bytes long. Then we tell the DirectSound subsystem to playback the data from this buffer.
 * The DirectSound consequently reads bytes form that buffer and feeds them to the sound card. This is a sequential process, i.e. not the entire 
 * buffer will be consumed at time. The speed, at which this buffer is consumed, depends on what is the sampling rate, how many bits per sample are there, how many channels and so on (please see nAvgBytesPerSec member of the WAVEFORMATEX structure).
 * The DirectSound subsytem reserves a part of the buffer, couple of milliseconds long, 
 * to secure the needs of the hardware sound card buffer. 
 * This part, from so called read cursor to the write cursor, is off limits - we shall 
 * not access it. This is also the the guard period. This guard period shall give us sufficient time 
 * to fill the remaining part of the buffer before the times comes to play it.
 * The rest of the buffer, from write cursor to the end of the buffer 
 * and then wrapped around beginning to the read cursor, can be accessed freely. 
 * So imagine that we have a 10ms timer ticking, and with each timer tick we check the cursors. 
 * Our buffer is 8192 bytes long.  Here's an example output:
 * <pre>
 *   0ms Read:    0 bytes, Write:  480 bytes
 *  10ms Read:  380 bytes, Write:  860 bytes 
 *  20ms Read:  760 bytes, Write: 1240 bytes 
 *  .. rest of lines removed for brevity ..
 * 210ms Read: 7980 bytes, Write:  268 bytes
 *  .. rest of lines removed for brevity ..
 * <pre>
 * As can be easily seen, cursors wrap around. 
 * Therefore, a small buffer, of couple kB long, can be used to play much larger files, 
 * provided we alter the contents of the buffer fast enough.
 * How fast is fast enough? Well, it took about 200ms to play entire buffer. 
 * So it takes about 100ms to play the first half of it. The guard period is about 30 ms long (480 bytes) on my PC. 
 * If we then divide this buffer into 2 halves, 4096 bytes each, we can alternate between those two buffers, playing one, 
 * and then, at the same time, filling the other one. We have about 100 +/- 30 ms to do this. We only need to care about 
 * filling a buffer, as the hardware automatically plays the other one.
 * Even given the medicore realtime capabilities of the Windows platform, 
 * this timespan usually more then sufficient on even the very low-end hardware.
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
#include "circular-buffer-uint8.h"
#include "input-buffer.h"
#include "receiver-settings.h"
#include "perf-counter-itf.h"
#include "dsbcaps-utils.h"

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
 * @brief Maximum supported number of buffer chunks.
 */
#define MAX_CHUNKS_COUNT (16)

/*!
 * Number of positions in the replay buffer, upon reaching which, the notification will be send to the player thread.
 */
#define NOTIFY_OBJECTS_COUNT (0)

#define MAX_THREAD_WAIT_TIMEOUT_MS (1000)

#define MAX_WAIT_TIMEOUT_FOR_THREAD (2*MAX_THREAD_WAIT_TIMEOUT_MS)


typedef enum e_player_state {
    PLAYER_IDLE,
    PLAYER_PLAYING,
    PLAYER_EXITTING
} e_player_state_t;

struct dsound_data;

typedef struct dxaudio_player_thread_information_block {
    struct perf_counter * counter1_;
    _int64 total, avg, freq;
    LPDIRECTSOUND8          p_direct_sound_8_;              /*!< The DirectSound Object. */
    LPDIRECTSOUNDBUFFER     p_primary_sound_buffer_;        /*!< The DirectSound primary buffer. */
    LPDIRECTSOUNDBUFFER8    p_secondary_sound_buffer_;      /*!< The DirectSound secondary buffer. */
    DWORD   buffer_markers_[MAX_CHUNKS_COUNT];             /*!< Array of markers whether a buffer has been filled or played. */
    struct dsound_data * p_dsound_data;
    volatile e_player_state_t e_state_;
    HANDLE wait_objects_table_[3+NOTIFY_OBJECTS_COUNT]; /*!< Handles of the notification marks plus 3 events for start, stop, and exit */
} dxaudio_player_thread_information_block_t;

/*!
 * @brief The sound player descriptor. 
 * @details Gathers all the variables needed to successfully play chunks of PCM
 * data using DirectSound.
 */
struct dsound_data {
    struct fifo_circular_buffer * fifo_;	/*!< A fifo queue - from that queue we fetch the data and feed to the buffers.*/
    struct play_settings play_settings_;	/*!< Settings for our player (how many bytes per buffer, timer frequency).*/
    size_t                  nSingleBufferSize_;               /*!< Size of a single buffer. */
    size_t                  number_of_chunks_; 
    WAVEFORMATEX wfe_;
    HANDLE hStartPlay_;
    HANDLE hStopPlay_;
    HANDLE hExitPlay_;
    HANDLE hPlayerThread_;
    HWND hWnd_; /*!< Handle of the application window to be passed to the IDirectSound8::SetCooperativeLevel() */
};

/**
 * @brief Creates the primary buffer and the secondary buffers.
 * @param[in] p_direct_sound_8 pointer to the IDirectSound8 interface. This interface is a factory for creating both the primary buffer and the secondary buffers.
 * @param[out] pp_primary_buffer pointer to the memory location which will be written with the primary buffer interface pointer.
 * @param[out] pp_secondary_buffer pointer to the memory location which will be written with the secondary buffer interface pointer. 
 * @param[in] p_wfe pointer to the WAVEFORMATEX structure. This parameter defines the buffer format (number of samples per second, how many bytes per sample and so on).
 * @param[in] single_buffer_size size of the single play buffer
 * @return
 */
static HRESULT create_buffers(dxaudio_player_thread_information_block_t * p_ds_data)
{
    HRESULT hr;
    DSBUFFERDESC bufferDesc;
    LPDIRECTSOUNDBUFFER lpDSB = NULL;
    if (NULL != p_ds_data->p_primary_sound_buffer_ || NULL != p_ds_data->p_secondary_sound_buffer_)
    {
        debug_outputln("%s %4.4u : %p %p", __FILE__, __LINE__, p_ds_data->p_primary_sound_buffer_, p_ds_data->p_secondary_sound_buffer_);
        return E_INVALIDARG;
    }
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.dwSize = sizeof(bufferDesc);
    bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER; 
    bufferDesc.guid3DAlgorithm = DS3DALG_DEFAULT;
    /* All others must be null for primary buffer */
    hr = p_ds_data->p_direct_sound_8_->CreateSoundBuffer(&bufferDesc, &p_ds_data->p_primary_sound_buffer_, NULL);
    if (FAILED(hr))
    {
        debug_outputln("%s %4.4u : %x", __FILE__, __LINE__, hr);
        goto error;
    }
    hr = p_ds_data->p_primary_sound_buffer_->SetFormat(&p_ds_data->p_dsound_data->wfe_);
    if (FAILED(hr))
    {
        debug_outputln("%s %4.4u : %8.8x", __FILE__, __LINE__, hr);
        goto error;
    }
    get_buffer_caps(p_ds_data->p_primary_sound_buffer_);
    /* Secondary buffer */
    bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME /* The buffer has volume control capability. */ 
        | DSBCAPS_CTRLPAN /* The buffer has pan control capability. */ 
        | DSBCAPS_CTRLFREQUENCY /* The buffer has frequency control capability. */ 
        | DSBCAPS_GLOBALFOCUS /* With this flag set, an application using DirectSound can continue to play its buffers if the user switches focus to another application, even if the new application uses DirectSound. */ 
        | DSBCAPS_CTRLPOSITIONNOTIFY; /* The buffer has position notification capability. */

    /* double buffering - one buffer being played, whereas the other is being filled in */
    bufferDesc.dwBufferBytes = p_ds_data->p_dsound_data->number_of_chunks_*p_ds_data->p_dsound_data->nSingleBufferSize_;
    bufferDesc.lpwfxFormat = &p_ds_data->p_dsound_data->wfe_;
    hr = p_ds_data->p_direct_sound_8_->CreateSoundBuffer(&bufferDesc, &lpDSB, NULL);
    if (FAILED(hr))
    {
        debug_outputln("%s %4.4u : %x", __FILE__, __LINE__, hr);
        goto error;
    }
    hr = lpDSB->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&p_ds_data->p_secondary_sound_buffer_);
    if (FAILED(hr))
    {
        debug_outputln("%s %4.4u : %x", __FILE__, __LINE__, hr);
        goto error;
    }
    get_buffer_caps(p_ds_data->p_secondary_sound_buffer_);
    if (NULL != lpDSB)
    {
        lpDSB->Release();
        lpDSB = NULL;
    }
    return hr;
error:
    if (NULL != p_ds_data->p_secondary_sound_buffer_)
    {
        p_ds_data->p_secondary_sound_buffer_->Release();
        p_ds_data->p_secondary_sound_buffer_ = NULL;
    }
    if (NULL != p_ds_data->p_primary_sound_buffer_)
    {
        p_ds_data->p_primary_sound_buffer_->Release();
        p_ds_data->p_primary_sound_buffer_ = NULL;
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
static HRESULT init_ds_data(HWND hwnd, WAVEFORMATEX const * p_WFE, dxaudio_player_thread_information_block_t * p_ds_data)
{
    HRESULT hr = E_FAIL;
#if 0
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
    CopyMemory(&p_ds_data->p_dsound_data->wfe_, p_WFE, sizeof(WAVEFORMATEX));
    p_ds_data->p_dsound_data->wfe_.cbSize = sizeof(WAVEFORMATEX);
    p_ds_data->p_dsound_data->nSingleBufferSize_ = p_ds_data->p_dsound_data->play_settings_.play_buffer_size_;
    hr = create_buffers(p_ds_data);
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
#endif
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
#if 0
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
#endif
}

/*!
 * @brief Fills the secondary buffer with audio samples.
 * @param[in] p_ds_data pointer to the player descriptor.
 */
static void play_data_chunk(dxaudio_player_thread_information_block_t* p_ds_data) 
{
#if 0
    DWORD dwRead, dwWrite;
    HRESULT hr;
    hr = p_ds_data->p_secondary_sound_buffer_->GetCurrentPosition(&dwRead, &dwWrite);
    if (SUCCEEDED(hr))
    {
        uint8_t * p_data = (uint8_t*)alloca(p_ds_data->p_dsound_data->nSingleBufferSize_);
        uint32_t size = p_ds_data->p_dsound_data->nSingleBufferSize_;
        struct buffer_desc buf_desc;
        unsigned int read_buf_index, write_buf_index;
        buf_desc.p_begin_ = p_data;
        buf_desc.nMaxOffset_ = p_ds_data->p_dsound_data->nSingleBufferSize_;
        buf_desc.nCurrentOffset_ = 0;
        read_buf_index = dwRead / p_ds_data->p_dsound_data->nSingleBufferSize_;
        write_buf_index = dwWrite / p_ds_data->p_dsound_data->nSingleBufferSize_;
        /* Check if both write & read cursor point to the same buffer */
        if (read_buf_index == write_buf_index)
        {
            unsigned int next_buf_index = (read_buf_index + 1) % p_ds_data->p_dsound_data->number_of_chunks_;
            assert(next_buf_index < p_ds_data->p_dsound_data->number_of_chunks_);
            assert(read_buf_index < p_ds_data->p_dsound_data->number_of_chunks_);
            if (BUFFER_PLAYED == p_ds_data->buffer_markers_[next_buf_index])
            {
                if (fifo_circular_buffer_get_items_count(p_ds_data->fifo_)>0)
                {
                    fifo_circular_buffer_fetch_item(p_ds_data->fifo_, p_data, &size);
                }
                else
                {
                    memset(p_data, 0, sizeof(uint8_t)*size);
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
#endif
    return;
}

static dxaudio_player_thread_information_block_t * create_player_thread_information_block(struct dsound_data * p_data)
{
    dxaudio_player_thread_information_block_t * p_player = (dxaudio_player_thread_information_block_t *)
        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(dxaudio_player_thread_information_block_t));
    if (NULL != p_player)
    {
        HANDLE * p_wait_objects_table_iter = &p_player->wait_objects_table_[0];
        *p_wait_objects_table_iter = p_data->hStartPlay_;
        ++p_wait_objects_table_iter;
        *p_wait_objects_table_iter = p_data->hStopPlay_;
        ++p_wait_objects_table_iter;
        *p_wait_objects_table_iter = p_data->hExitPlay_;
        ++p_wait_objects_table_iter;
        return p_player;
    }
    return p_player;
}

static void player_thread_on_start_playing(dxaudio_player_thread_information_block_t * p_tib, HANDLE hEvent)
{
    ResetEvent(hEvent);
    debug_outputln("%4.4u %s : %8.8x", __LINE__, __FILE__, ::GetLastError());
}

static void player_thread_on_stop_playing(dxaudio_player_thread_information_block_t * p_tib, HANDLE hEvent)
{
    ResetEvent(hEvent);
    debug_outputln("%4.4u %s : %8.8x", __LINE__, __FILE__, ::GetLastError());
}

static void player_thread_on_exit(dxaudio_player_thread_information_block_t * p_tib, HANDLE hEvent)
{
    ResetEvent(hEvent);
    debug_outputln("%4.4u %s : %8.8x", __LINE__, __FILE__, ::GetLastError());
    p_tib->e_state_ = PLAYER_EXITTING;
}

static DWORD WINAPI dxaudio_player_thread(void * p_param)
{
    HRESULT hr;
    hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        dxaudio_player_thread_information_block_t  * p_tib;
        struct dsound_data * p_dsound_data = (struct dsound_data*)p_param;
        p_tib = create_player_thread_information_block(p_dsound_data);
        if (NULL != p_dsound_data)
        {
            DWORD dwWaitResult;
            /* Main player loop - here we process notifications from the main thread */
            while (PLAYER_EXITTING != p_tib->e_state_)
            {
                dwWaitResult = WaitForMultipleObjects(COUNTOF_ARRAY(p_tib->wait_objects_table_), 
                        &p_tib->wait_objects_table_[0], FALSE, INFINITE);
                switch (dwWaitResult)
                {
                    case WAIT_TIMEOUT:
                        debug_outputln("%4.4u %s : %8.8x", __LINE__, __FILE__, ::GetLastError());
                        break;
                    case WAIT_FAILED:
                        debug_outputln("%4.4u %s : %8.8x", __LINE__, __FILE__, ::GetLastError());
                        break;
                    default:
                        switch (dwWaitResult-WAIT_OBJECT_0)
                        {
                            case 0:
                                player_thread_on_start_playing(p_tib, p_tib->wait_objects_table_[0]);
                                break;
                            case 1:
                                player_thread_on_stop_playing(p_tib, p_tib->wait_objects_table_[1]);
                                break;
                            case 2:
                                player_thread_on_exit(p_tib, p_tib->wait_objects_table_[2]);
                                break;
                            default:
                                break;
                        }
                        break;
                }
            }
        }
    }
    return SUCCEEDED(hr);    
}

extern "C" DSOUNDPLAY dsoundplayer_create(HWND hWnd, struct receiver_settings const * p_settings, struct fifo_circular_buffer * fifo)
{
    struct dsound_data * p_retval = 
        (struct dsound_data*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct dsound_data));
    if (NULL != p_retval)
    {
        p_retval->fifo_ = fifo;
        p_retval->number_of_chunks_ = p_settings->play_settings_.play_chunks_count_;
        play_settings_copy(&p_retval->play_settings_, &p_settings->play_settings_);
        p_retval->hStartPlay_ = ::CreateEvent(NULL, TRUE, FALSE, NULL);
        if (NULL != p_retval->hStartPlay_)
        {
            p_retval->hStopPlay_ = ::CreateEvent(NULL, TRUE, FALSE, NULL);
            if (NULL != p_retval->hStopPlay_)
            {
                p_retval->hExitPlay_ = ::CreateEvent(NULL, TRUE, FALSE, NULL);
                if (NULL != p_retval->hExitPlay_)
                {
                    p_retval->hPlayerThread_ = ::CreateThread(NULL, 0, dxaudio_player_thread, p_retval, CREATE_SUSPENDED, NULL);
                    if (NULL != p_retval->hPlayerThread_)
                    {
                        ::ResumeThread(p_retval->hPlayerThread_);
                        return (DSOUNDPLAY)(p_retval);
                    }
                    CloseHandle(p_retval->hExitPlay_);
                }
                CloseHandle(p_retval->hStopPlay_); 
            }
            CloseHandle(p_retval->hStartPlay_); 
        }
        HeapFree(GetProcessHeap(), 0, p_retval);
    }
    return (DSOUNDPLAY)p_retval;
}

extern "C" void dsoundplayer_destroy(DSOUNDPLAY handle) 
{
    DWORD dwResult;
    struct dsound_data * p_ds_data = (struct dsound_data*)handle;
    SetEvent(p_ds_data->hExitPlay_);
    dwResult = WaitForSingleObject(p_ds_data->hPlayerThread_, MAX_WAIT_TIMEOUT_FOR_THREAD);
    debug_outputln("%4.4u %s : %8.8x %8.8x", __LINE__, __FILE__, dwResult, WAIT_TIMEOUT);
    CloseHandle(p_ds_data->hPlayerThread_);
    CloseHandle(p_ds_data->hExitPlay_);
    CloseHandle(p_ds_data->hStopPlay_);
    CloseHandle(p_ds_data->hStartPlay_);
    HeapFree(GetProcessHeap(), 0, p_ds_data);
}

extern "C" int dsoundplayer_play(DSOUNDPLAY handle) 
{
    struct dsound_data * p_ds_data = (struct dsound_data*)handle;
    SetEvent(p_ds_data->hStartPlay_);
    return 1;
}

extern "C" void dsoundplayer_pause(DSOUNDPLAY handle) 
{
    struct dsound_data * p_ds_data = (struct dsound_data*)handle;
    SetEvent(p_ds_data->hStopPlay_);
}

extern "C" int dsoundplayer_stop(DSOUNDPLAY handle) 
{
    struct dsound_data * p_ds_data = (struct dsound_data*)handle;
    SetEvent(p_ds_data->hStopPlay_);
    return 1;
}

