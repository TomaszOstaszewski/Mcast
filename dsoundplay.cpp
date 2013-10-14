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
#include "directSound-caps.h"

/*!
 * @brief Maximum supported number of buffer chunks.
 */
#define MAX_CHUNKS_COUNT (16)

/*!
 * Number of positions in the replay buffer, upon reaching which, the notification will be send to the player thread.
 */
#define NOTIFY_OBJECTS_COUNT (2)

/** 
 * @brief 
 */
#define MAX_WAIT_TIMEOUT_FOR_THREAD (1000)

/**
 * @brief Player state.
 */
typedef enum e_player_state {
    PLAYER_IDLE, /*!< Player idling */
    PLAYER_PLAYING, /*!< Player replaying WAV file. */
    PLAYER_EXITTING /*!< Player about to exit. */
} e_player_state_t;

struct dsound_data;

typedef struct dxaudio_player_thread_information_block {
    struct fifo_circular_buffer * fifo_;	/*!< A fifo queue - from that queue we fetch the data and feed to the buffers.*/
    LPDIRECTSOUNDBUFFER8 p_secondary_sound_buffer_; /*!< The DirectSound secondary buffer. */
    volatile e_player_state_t e_state_;
    HANDLE wait_objects_array_[3+NOTIFY_OBJECTS_COUNT]; /*!< Handles of the notification marks plus 3 events for start, stop, and exit */
    struct perf_counter * counter1_;
    _int64 total, avg, freq;
    DSBPOSITIONNOTIFY notification_array_[NOTIFY_OBJECTS_COUNT];
    struct dsound_data * p_dsound_data;
    LPDIRECTSOUNDBUFFER p_primary_sound_buffer_; /*!< The DirectSound primary buffer. */
    LPDIRECTSOUND8 p_direct_sound_8_; /*!< The DirectSound Object. */
} dxaudio_player_thread_information_block_t;

/*!
 * @brief The sound player descriptor. 
 * @details Gathers all the variables needed to successfully play chunks of PCM
 * data using DirectSound.
 */
struct dsound_data {
    struct fifo_circular_buffer * fifo_; /*!< A fifo queue - from that queue we fetch the data and feed to the buffers.*/
    struct play_settings play_settings_; /*!< Settings for our player (how many bytes per buffer, timer frequency).*/
    struct receiver_settings receiver_settings_;
    size_t nSingleBufferSize_; /*!< Size of a single buffer. */
    size_t number_of_chunks_; 
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
static HRESULT create_buffers(LPDIRECTSOUND8 p_ds, 
    WAVEFORMATEX * p_wfex,
    size_t number_of_chunks,
    size_t single_buffer_size,
    LPDIRECTSOUNDBUFFER * pp_primary_sound_buffer, 
    LPDIRECTSOUNDBUFFER8 * pp_secondary_sound_buffer)
{
    HRESULT hr = E_INVALIDARG;
    DSBUFFERDESC bufferDesc;
    LPDIRECTSOUNDBUFFER lpDSB = NULL;
    if (NULL == *pp_primary_sound_buffer && NULL == *pp_secondary_sound_buffer)
    {
        ZeroMemory(&bufferDesc, sizeof(bufferDesc));
        bufferDesc.dwSize = sizeof(bufferDesc);
        bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER; 
        bufferDesc.guid3DAlgorithm = DS3DALG_DEFAULT;
        /* All others must be null for primary buffer */
        hr = p_ds->CreateSoundBuffer(&bufferDesc, pp_primary_sound_buffer, NULL);
        if (SUCCEEDED(hr))
        {
            hr = (*pp_primary_sound_buffer)->SetFormat(p_wfex);
            if (SUCCEEDED(hr))
            {
                get_buffer_caps("Primary: ", *pp_primary_sound_buffer);
                /* Secondary buffer */
                bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME /* The buffer has volume control capability. */ 
                    | DSBCAPS_CTRLPAN /* The buffer has pan control capability. */ 
                    | DSBCAPS_CTRLFREQUENCY /* The buffer has frequency control capability. */ 
                    | DSBCAPS_GLOBALFOCUS /* With this flag set, an application using DirectSound can continue      */
                    /* to play its buffers if the user switches focus to another application, */
                    /* even if the new application uses DirectSound.                          */ 
                    | DSBCAPS_CTRLPOSITIONNOTIFY; /* The buffer has position notification capability. */
                /* multiple buffering - our buffer is composed of multiple chunks */
                /* while one chunk is being played, the other one is available for change */
                bufferDesc.dwBufferBytes = number_of_chunks * single_buffer_size;
                bufferDesc.lpwfxFormat = p_wfex;
                hr = p_ds->CreateSoundBuffer(&bufferDesc, &lpDSB, NULL);
                if (SUCCEEDED(hr))
                {
                    hr = lpDSB->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)pp_secondary_sound_buffer);
                    if (SUCCEEDED(hr))
                    {
                        get_buffer_caps("Secondary: ", *pp_secondary_sound_buffer);
                        lpDSB->Release();
                        return hr;
                    }
                    lpDSB->Release(); 
                }
                else
                {
                    debug_outputln("%s %4.4u : %x", __FILE__, __LINE__, hr);
                }
            }
            else
            {
                debug_outputln("%s %4.4u : %8.8x", __FILE__, __LINE__, hr);
            }
            (*pp_primary_sound_buffer)->Release();
        }
        else
        {
            debug_outputln("%s %4.4u : %x", __FILE__, __LINE__, hr);
        }
    } 
    else
    {
        debug_outputln("%s %4.4u : %p %p", __FILE__, __LINE__, *pp_primary_sound_buffer, *pp_secondary_sound_buffer);
        return E_INVALIDARG;
    }
    return hr;
}

/**
 * @brief Configures the replay buffer so that at appropriate positions of replay, a notification is sent.
 * @details 
 * @param[in] p_secondary_buffer
 * @param[in] notify_array
 * @param[in] notify_array_count
 * @return 
 */
static HRESULT set_play_notifications(LPDIRECTSOUNDBUFFER8 p_secondary_buffer, 
 DSBPOSITIONNOTIFY * notify_array, 
 size_t notify_array_count)
{
    HRESULT hr;
    LPDIRECTSOUNDNOTIFY8 p_notify_itf = NULL;
    hr = p_secondary_buffer->QueryInterface(IID_IDirectSoundNotify8, (LPVOID*)&p_notify_itf);
    if (SUCCEEDED(hr))
    { 
        hr = p_notify_itf->SetNotificationPositions(notify_array_count, notify_array);
        p_notify_itf->Release();
#if 0
        debug_outputln("%4.4u %s : 0x%8.8x %u", __LINE__, __FILE__, hr, notify_array_count);
        for (size_t idx = 0; idx < notify_array_count; ++idx)
        {
            debug_outputln("%4.4u %s : 0x%8.8x %u", __LINE__, __FILE__, notify_array[idx].hEventNotify, notify_array[idx].dwOffset);
        }    
#endif
    }
    return hr;
}

static void dump_dscaps(LPDIRECTSOUND8 p_ds8)
{
    HRESULT hr;
    DSCAPS caps;
    ZeroMemory(&caps, sizeof(caps));
    caps.dwSize = sizeof(caps);
    hr = p_ds8->GetCaps(&caps);
    if (SUCCEEDED(hr))
    {
        char buffer[2048];
        get_caps_desc(&caps, buffer, sizeof(buffer));
        debug_outputln("%4.4u %s : %s", __LINE__, __FILE__, buffer);
    }
    else
    {
        debug_outputln("%4.4u %s : 0x%8.8x", __LINE__, __FILE__, hr);
    }
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
            HeapFree(GetProcessHeap(), 0, p_tib);
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
    HRESULT hr;
    hr = DirectSoundCreate8(&DSDEVID_DefaultVoicePlayback, &p_ds_data->p_direct_sound_8_, NULL);
    if (SUCCEEDED(hr))
    {
        dump_dscaps(p_ds_data->p_direct_sound_8_);
        if (NULL == hwnd)
        {
            hwnd = GetForegroundWindow();
        }
        if (NULL == hwnd)
        {
            hwnd = GetDesktopWindow();
        }
        /* Quoting MSDN: */
        /* "[..]After creating a device object, you must set the cooperative level  */
        /* for the device by using the IDirectSound8::SetCooperativeLevel method.  */
        /* Unless you do this, no sounds will be heard." */
        hr = p_ds_data->p_direct_sound_8_->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
        if (SUCCEEDED(hr))
        {
            CopyMemory(&p_ds_data->p_dsound_data->wfe_, p_WFE, sizeof(WAVEFORMATEX));
            p_ds_data->p_dsound_data->wfe_.cbSize = sizeof(WAVEFORMATEX);
            p_ds_data->p_dsound_data->nSingleBufferSize_ = p_ds_data->p_dsound_data->play_settings_.play_chunk_size_in_bytes_;
            debug_outputln("%4.4u %s : %u", __LINE__, __FILE__, p_ds_data->p_dsound_data->play_settings_.play_chunk_size_in_bytes_);
            hr = create_buffers(p_ds_data->p_direct_sound_8_, 
                    &p_ds_data->p_dsound_data->wfe_, 
                    p_ds_data->p_dsound_data->number_of_chunks_,
                    p_ds_data->p_dsound_data->nSingleBufferSize_,
                    &p_ds_data->p_primary_sound_buffer_,
                    &p_ds_data->p_secondary_sound_buffer_
                    );
            if (SUCCEEDED(hr))
            {
                hr = set_play_notifications(p_ds_data->p_secondary_sound_buffer_, 
                        &p_ds_data->notification_array_[0], COUNTOF_ARRAY(p_ds_data->notification_array_));
                return hr;
            }
            else 
            {
                debug_outputln("%s %4.4u : %x", __FILE__, __LINE__, hr);
            }
        }
        else 
        {
            debug_outputln("%s %4.4u : %x", __FILE__, __LINE__, hr);
        }
        p_ds_data->p_direct_sound_8_->Release();
        p_ds_data->p_direct_sound_8_ = NULL;
    }
    else
    {
        debug_outputln("%4.4u %s : 0x%8.8x", __LINE__, __FILE__, hr);
    }
    return hr;
}

/**
 * @brief Main play routine - fill the buffer to be replayed by DirectSound subsystem.
 * @details This routine copies the received playback data from the 'userspace' to the
 * DirectSound provided buffer. This buffer will be then replayed by the DirectSound subsystem if such need
 * arises. 
 * @param[in] p_buffer - pointer to the secondary buffer into which data will be replayed.
 * @param[in] p_fifo - pointer to the FIFO queue from which data will be fetched.
 * @param[in] idx - index of the part of the DirectSound chunk into which copy data.
 * @return returns S_OK on success, any other result indicates a failure.
 */
static HRESULT fill_buffer(LPDIRECTSOUNDBUFFER8 p_buffer, fifo_circular_buffer * p_fifo, size_t idx)
{
    LPVOID lpvWrite1;
    DWORD dwLength1;
    HRESULT hr;
    DWORD dwOffset;
    dwOffset = idx * 1024;
    hr = p_buffer->Lock(dwOffset, // Offset at which to start lock.
            1024, // Size of lock;
            (LPVOID*)&lpvWrite1, // Gets address of first part of lock.
            &dwLength1, // Gets size of first part of lock.
            NULL, /* Second part not needed as we will never wrap around - we lock equal buffer chunks */
            NULL,
            0); // Flag.
    if (SUCCEEDED(hr))
    {
        size_t size;
        size = 1024;
        /* Copy as many items as you can, no more than chunk size, into the buffer */
        fifo_circular_buffer_fetch_item(p_fifo, (uint8_t*)lpvWrite1, &size);
        /* Fill the remaining part, if any, with zeros */
        ZeroMemory((uint8_t*)lpvWrite1+size, dwLength1-size);
        /* Return modified buffer to DirectSound */
        hr = p_buffer->Unlock(lpvWrite1, dwLength1, NULL, 0);
    }
    else
    {
        debug_outputln("%4.4u %s : 0x%8.8x", __FILE__, __LINE__, hr);
    }
    return hr;
}

/**
 * @brief
 * @details
 * @param[in] 
 * @return
 */
static dxaudio_player_thread_information_block_t * create_player_thread_information_block(struct dsound_data * p_data)
{
    dxaudio_player_thread_information_block_t * p_player = (dxaudio_player_thread_information_block_t *)
        HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(dxaudio_player_thread_information_block_t));
    if (NULL != p_player)
    {
        HANDLE * p_wait_objects_table_iter;
        p_wait_objects_table_iter = &p_player->wait_objects_array_[0];
        /* Initialize wait handles table, first 3 events are reseverd */
        /* for communication with this worker thread.                 */
        *p_wait_objects_table_iter = p_data->hStartPlay_;
        ++p_wait_objects_table_iter;
        *p_wait_objects_table_iter = p_data->hStopPlay_;
        ++p_wait_objects_table_iter;
        *p_wait_objects_table_iter = p_data->hExitPlay_;
        ++p_wait_objects_table_iter;
        {
            size_t idx;
            for (idx = 0; idx < NOTIFY_OBJECTS_COUNT; ++idx, ++p_wait_objects_table_iter)
            {
                /* Indicate at which positions of play buffer shall a thread be notified. */
                p_player->notification_array_[idx].dwOffset = idx * 1024;
                p_player->notification_array_[idx].hEventNotify = ::CreateEvent(NULL, TRUE, FALSE, NULL);
                assert(NULL != p_player->notification_array_[idx].hEventNotify);
                /* Add a notification event to array of all notification events */
                *p_wait_objects_table_iter = p_player->notification_array_[idx].hEventNotify;
            }
        }
        p_player->p_dsound_data = p_data;
        p_player->fifo_ = p_data->fifo_;
        init_ds_data(p_data->hWnd_, &p_data->receiver_settings_.wfex_, p_player); 
        return p_player;
    }
    return p_player;
}

/**
 * @brief Handler of the 'PLAY' command 
 * @details
 * @param[in]
 * @param[in]
 */
static void player_thread_on_start_playing(dxaudio_player_thread_information_block_t * p_tib, HANDLE hEvent)
{
    HRESULT hr;
    /* Acknowledge interrupt */
    ::ResetEvent(hEvent);

    hr = p_tib->p_secondary_sound_buffer_->Play(0, 0, DSBPLAY_LOOPING); 
    debug_outputln("%4.4u %s : 0x%8.8x 0x%8.8x", __LINE__, __FILE__, hr, S_OK);
}

/**
 * @brief Handler of the 'STOP' command 
 * @details
 * @param[in]
 * @param[in]
 */
static void player_thread_on_stop_playing(dxaudio_player_thread_information_block_t * p_tib, HANDLE hEvent)
{
    HRESULT hr;
    /* Acknowledge interrupt */
    ::ResetEvent(hEvent);

    hr = p_tib->p_secondary_sound_buffer_->Stop();
    debug_outputln("%4.4u %s : 0x%8.8x 0x%8.8x", __LINE__, __FILE__, hr, S_OK);
}

/**
 * @brief Handler of the 'EXIT' command 
 * @details
 * @param[in]
 * @param[in]
 */
static void player_thread_on_exit(dxaudio_player_thread_information_block_t * p_tib, HANDLE hEvent)
{
    HRESULT hr;
    /* Acknowledge interrupt */
    ::ResetEvent(hEvent);
    hr = p_tib->p_secondary_sound_buffer_->Stop();
    debug_outputln("%4.4u %s : 0x%8.8x 0x%8.8x", __LINE__, __FILE__, hr, S_OK);
    p_tib->e_state_ = PLAYER_EXITTING;
}

static DWORD WINAPI dxaudio_player_thread(void * p_param)
{
    HRESULT hr;
    hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr))
    {
        dxaudio_player_thread_information_block_t  * p_tib;
        struct dsound_data * p_dsound_data = (struct dsound_data*)p_param;
        p_tib = create_player_thread_information_block(p_dsound_data);
        if (NULL != p_dsound_data)
        {
            size_t idx;
            if (SUCCEEDED(hr))
            {
                DWORD dwWaitResult;
                /* Main player loop - here we process notifications from the main thread */
                while (PLAYER_EXITTING != p_tib->e_state_)
                {
                    dwWaitResult = ::WaitForMultipleObjects(COUNTOF_ARRAY(p_tib->wait_objects_array_), 
                            &p_tib->wait_objects_array_[0], FALSE, INFINITE);
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
                                    player_thread_on_start_playing(p_tib, p_tib->wait_objects_array_[0]);
                                    break;
                                case 1:
                                    player_thread_on_stop_playing(p_tib, p_tib->wait_objects_array_[1]);
                                    break;
                                case 2:
                                    player_thread_on_exit(p_tib, p_tib->wait_objects_array_[2]);
                                    break;
                                default:
                                    for (idx = 3; idx < 3 + NOTIFY_OBJECTS_COUNT; ++idx)
                                    {
                                        if (dwWaitResult-WAIT_OBJECT_0 == idx) 
                                        {
                                            DWORD dw_read_cursor, dw_write_cursor;
                                            ::ResetEvent(p_tib->wait_objects_array_[idx]);
                                            hr = p_tib->p_secondary_sound_buffer_->GetCurrentPosition(&dw_read_cursor, &dw_write_cursor);   
                                            if (SUCCEEDED(hr))
                                            {
                                                fill_buffer(p_tib->p_secondary_sound_buffer_, 
                                                    p_tib->fifo_, (idx - 3 + 1)%2);
                                            }
                                            else
                                            {
                                                debug_outputln("%4.4u %s : 0x%8.8x", __LINE__, __FILE__, hr);
                                            }
                                        }
                                    }
                                    break;
                            }
                            break;
                    }
                }
            }
            else
            {
                debug_outputln("%4.4u %s : 0x%8.8x", __LINE__, __FILE__, hr);
            }
            p_tib->p_secondary_sound_buffer_->Release();
            p_tib->p_primary_sound_buffer_->Release();
            p_tib->p_direct_sound_8_->Release();
            for (idx = 3; idx < 3 + NOTIFY_OBJECTS_COUNT; ++idx)
            {
                ::CloseHandle(p_tib->wait_objects_array_[idx]);
            }
            ::HeapFree(GetProcessHeap(), 0, p_tib);
        }
        ::CoUninitialize();
    }
    return 0;    
}

extern "C" DSOUNDPLAY dsoundplayer_create(HWND hWnd, struct receiver_settings const * p_settings, struct fifo_circular_buffer * fifo)
{
    struct dsound_data * p_retval = 
        (struct dsound_data*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct dsound_data));
    if (NULL != p_retval)
    {
        p_retval->fifo_ = fifo;
        p_retval->hWnd_ = hWnd;
        receiver_settings_copy(&p_retval->receiver_settings_, p_settings);
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
                    p_retval->hPlayerThread_ = ::CreateThread(NULL, 0, 
                        dxaudio_player_thread, p_retval, CREATE_SUSPENDED, NULL);
                    if (NULL != p_retval->hPlayerThread_)
                    {
                        ::ResumeThread(p_retval->hPlayerThread_);
                        return (DSOUNDPLAY)(p_retval);
                    }
                    CloseHandle(p_retval->hExitPlay_);
                }
                ::CloseHandle(p_retval->hStopPlay_); 
            }
            ::CloseHandle(p_retval->hStartPlay_); 
        }
        ::HeapFree(GetProcessHeap(), 0, p_retval);
    }
    return (DSOUNDPLAY)p_retval;
}

extern "C" void dsoundplayer_destroy(DSOUNDPLAY handle) 
{
    DWORD dwResult;
    /* Signal the replay thread that we want to exit */
    ::SetEvent(handle->hExitPlay_);
    /* Wait for replay thread exit */
    dwResult = ::WaitForSingleObject(handle->hPlayerThread_, MAX_WAIT_TIMEOUT_FOR_THREAD);
    debug_outputln("%4.4u %s : %8.8x %8.8x", __LINE__, __FILE__, dwResult, WAIT_TIMEOUT);
    ::CloseHandle(handle->hPlayerThread_);
    ::CloseHandle(handle->hExitPlay_);
    ::CloseHandle(handle->hStopPlay_);
    ::CloseHandle(handle->hStartPlay_);
    ::HeapFree(GetProcessHeap(), 0, handle);
}

extern "C" int dsoundplayer_play(DSOUNDPLAY handle) 
{
    /* Signal that we want to start replay */ 
    ::SetEvent(handle->hStartPlay_);
    return 1;
}

extern "C" void dsoundplayer_pause(DSOUNDPLAY handle) 
{
    /* Signal that we want to stop */ 
    ::SetEvent(handle->hStopPlay_);
}

extern "C" int dsoundplayer_stop(DSOUNDPLAY handle) 
{
    /* Signal that we want to stop */ 
    ::SetEvent(handle->hStopPlay_);
    return 1;
}

