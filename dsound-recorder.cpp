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
#include "dsound-recorder.h"
#include "wave_utils.h"
#include "circular-buffer-uint8.h"
#include "recorder-settings.h"
#include "perf-counter-itf.h"

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

#define NOTIFY_MARKS_COUNT (2)

#define MAX_THREAD_WAIT_TIMEOUT_MS (1000)

#define MAX_WAIT_TIMEOUT_FOR_THREAD (2*MAX_THREAD_WAIT_TIMEOUT_MS)

/*!
 * @brief The sound player descriptor. 
 * @details Gathers all the variables needed to successfully play chunks of PCM
 * data using DirectSound.
 */
struct dxaudio_recorder {
    IDirectSoundCapture8 * p_capture8_; /*!< */
    IDirectSoundCaptureBuffer8 * p_capture_buffer8_; /*!< . */
    struct fifo_circular_buffer * fifo_; /*!< A fifo queue - from that queue we fetch the data and feed to the buffers.*/
    DWORD dw_notify_marks_begin_[NOTIFY_MARKS_COUNT];
    DSBPOSITIONNOTIFY notify_marks_[NOTIFY_MARKS_COUNT];
    HANDLE hWatcherThread_;
    HANDLE hWatcherThreadRunning_;
};

static DWORD WINAPI recorder_thread(LPVOID param)
{
    struct dxaudio_recorder * p_recorder = (struct dxaudio_recorder *)param;
    HANDLE  h_wait_table[1 + NOTIFY_MARKS_COUNT];
    size_t idx;
    DWORD runLoop = 0xffffffff;
    assert(NULL != p_recorder);
    assert(NULL != p_recorder->hWatcherThreadRunning_);
    h_wait_table[0] = p_recorder->hWatcherThreadRunning_;
    for (idx  = 0; idx < NOTIFY_MARKS_COUNT; ++idx)
    {
        assert(NULL != p_recorder->notify_marks_[idx].hEventNotify);
        h_wait_table[idx+1] = p_recorder->notify_marks_[idx].hEventNotify;
    }
    for (;runLoop;)
    {
        DWORD dwWaitResult;
        dwWaitResult = WaitForMultipleObjects(COUNTOF_ARRAY(h_wait_table), &h_wait_table[0], FALSE, INFINITE);
        switch (dwWaitResult)
        {
            /* Handle WAIT_TIMEOUT */
            case WAIT_TIMEOUT:
                debug_outputln("%4.4u %s : %8.8x", __LINE__, __FILE__, ::GetLastError());
                break;
            /* Handle WAIT_FAILED */
            case WAIT_FAILED:
                debug_outputln("%4.4u %s : %8.8x", __LINE__, __FILE__, ::GetLastError());
                break;
            /* Handle WAIT_OBJECT_0 which indicates that we shall exit now. */
            case WAIT_OBJECT_0:
                /* Ack interrupt */
                ResetEvent(p_recorder->hWatcherThreadRunning_);
                debug_outputln("%4.4u %s : %8.8x", __LINE__, __FILE__, ::GetLastError());
                runLoop = 0;
                break;
            /* All the rest is a notification from our recorder, that the buffer is now ready to be processed */
            /* We ignore WAIT_ABANDONED_0..N, as we wait for event objects, which do not have 'ABANDONED' state */
            default:
                for (idx = 1; idx <= NOTIFY_MARKS_COUNT; ++idx)
                {
                    if (dwWaitResult - WAIT_OBJECT_0 == idx)
                    {
                        size_t items_pushed;
                        LPVOID p_ptr_1 = NULL, p_ptr_2 = NULL;
                        DWORD dw_offset_1 = 0, dw_offset_2 = 0;
                        /* Acknowledge notification */
                        ResetEvent(p_recorder->notify_marks_[idx-1].hEventNotify);
                        /* Retrive data just captured */
                        debug_outputln("%4.4u %s :" " %u %u", __LINE__, __FILE__, 
                                dwWaitResult, idx); 
                        p_recorder->p_capture_buffer8_->Lock(p_recorder->dw_notify_marks_begin_[idx-1],
                                p_recorder->notify_marks_[idx-1].dwOffset - p_recorder->dw_notify_marks_begin_[idx-1] + 1,
                                &p_ptr_1, &dw_offset_1,
                                &p_ptr_2, &dw_offset_2,
                                0);
                        items_pushed = fifo_circular_buffer_push_item(p_recorder->fifo_, (uint8_t*) p_ptr_1, dw_offset_1);
                        p_recorder->p_capture_buffer8_->Unlock(p_ptr_1, dw_offset_1, p_ptr_2, dw_offset_2);
                        debug_outputln("%4.4u %s :" "%u" " %u %u %p %p %u %u", __LINE__, __FILE__, 
                                items_pushed,
                                p_recorder->dw_notify_marks_begin_[idx-1],
                                p_recorder->notify_marks_[idx-1].dwOffset - p_recorder->dw_notify_marks_begin_[idx-1] + 1,
                                p_ptr_1, p_ptr_2, dw_offset_1, dw_offset_2);
                    }
                } 
                break;
        }
    }
    debug_outputln("%4.4u %s", __LINE__, __FILE__);
    return 0;
}

extern "C" dxaudio_recorder_t dxaudio_recorder_create(
 struct recorder_settings const * p_settings, 
 struct fifo_circular_buffer * p_fifo)
{
    struct dxaudio_recorder * p_retval = NULL;
    p_retval = (struct dxaudio_recorder *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct dxaudio_recorder));
    if (NULL != p_retval)
    {
        HRESULT hr;
        /* Get the reference to fifo queue. */
        p_retval->fifo_= p_fifo;
        assert(NULL != p_retval->fifo_);
        /* 1st step - create the DirectSound capture object and get its interface */
        hr = DirectSoundCaptureCreate8(recorder_settings_get_guid(p_settings), &p_retval->p_capture8_, NULL);
        if (SUCCEEDED(hr))
        {
            IDirectSoundCaptureBuffer * p_capture_buffer = NULL;
            DSCBUFFERDESC dsc_buffer_desc = { 0 };
            WAVEFORMATEX wfx;
            /* 2nd step - create the DirectSound capture object and get its interface */
            CopyMemory(&wfx, recorder_settings_get_waveformatex(p_settings), sizeof(WAVEFORMATEX));
            dsc_buffer_desc.dwSize = sizeof(DSCBUFFERDESC);
            dsc_buffer_desc.dwBufferBytes = recorder_settings_get_samples_buffer_size(p_settings);
            dsc_buffer_desc.lpwfxFormat = &wfx;
            hr = p_retval->p_capture8_->CreateCaptureBuffer(&dsc_buffer_desc, &p_capture_buffer, NULL);
            if (SUCCEEDED(hr))
            {
                /* 3rd step - query for IDirectSoundCaptureBuffer8 interface */
                hr = p_capture_buffer->QueryInterface(IID_IDirectSoundCaptureBuffer8, (void **)&p_retval->p_capture_buffer8_);
                if (SUCCEEDED(hr))
                {
                    /* 4th step - set notification positions */
                    /* We set up to NOTIFY_MARKS_COUNT notification positions */
                    IDirectSoundNotify8 * p_notify8 = NULL;
                    hr = p_retval->p_capture_buffer8_->QueryInterface(IID_IDirectSoundNotify8, (void **)&p_notify8);
                    if (SUCCEEDED(hr))
                    {
                        size_t idx;
                        for (idx = 0; idx < NOTIFY_MARKS_COUNT; ++idx)
                        {
                            p_retval->dw_notify_marks_begin_[idx] = ((dsc_buffer_desc.dwBufferBytes*idx)/NOTIFY_MARKS_COUNT); 
                            p_retval->notify_marks_[idx].dwOffset = ((dsc_buffer_desc.dwBufferBytes*(idx+1))/NOTIFY_MARKS_COUNT) - 1; 
                            p_retval->notify_marks_[idx].hEventNotify = ::CreateEvent(NULL, TRUE, FALSE, NULL);

                            debug_outputln("%4.4u %s : %u %u %8.8x", __LINE__, __FILE__, 
                                p_retval->dw_notify_marks_begin_[idx],     
                                p_retval->notify_marks_[idx].dwOffset,
                                p_retval->notify_marks_[idx].hEventNotify);
                        } 
                        hr = p_notify8->SetNotificationPositions(NOTIFY_MARKS_COUNT, p_retval->notify_marks_);
                        if (SUCCEEDED(hr))
                        {
                            p_retval->hWatcherThreadRunning_ = ::CreateEvent(NULL, TRUE, FALSE, NULL);
                            if (NULL != p_retval->hWatcherThreadRunning_)
                            {
                                p_retval->hWatcherThread_ = ::CreateThread(NULL, 0, recorder_thread, p_retval, 0, NULL);
                                if (NULL != p_retval->hWatcherThread_)
                                {
                                    p_notify8->Release();
                                    p_capture_buffer->Release();
                                    return p_retval;
                                }
                                CloseHandle(p_retval->hWatcherThreadRunning_);
                            } 
                            debug_outputln("%4.4u %s %8.8x", __LINE__, __FILE__, hr);
                        }
                        debug_outputln("%4.4u %s %8.8x", __LINE__, __FILE__, hr);
                        p_notify8->Release();
                    }
                    debug_outputln("%4.4u %s %8.8x", __LINE__, __FILE__, hr);
                    p_retval->p_capture_buffer8_->Release();
                }
                debug_outputln("%4.4u %s %8.8x", __LINE__, __FILE__, hr);
                p_capture_buffer->Release();
            }
            debug_outputln("%4.4u %s %8.8x", __LINE__, __FILE__, hr);
            p_retval->p_capture8_->Release();
        }
        debug_outputln("%4.4u %s %8.8x", __LINE__, __FILE__, hr);
        HeapFree(GetProcessHeap(), 0, p_retval);
        p_retval = NULL;
    }
    debug_outputln("%4.4u %s %8.8x", __LINE__, __FILE__, ::GetLastError());
    return p_retval;
}

extern "C" void dxaudio_recorder_destroy(dxaudio_recorder_t handle) 
{
    size_t idx;
    struct dxaudio_recorder * p_recorder = (struct dxaudio_recorder* )handle;
    assert(NULL != p_recorder);
    /* Notify watcher thread that we are about to exit */
    SetEvent(p_recorder->hWatcherThreadRunning_);
    /* Wait for notification ack */
    WaitForSingleObject(p_recorder->hWatcherThread_, MAX_WAIT_TIMEOUT_FOR_THREAD);
    CloseHandle(p_recorder->hWatcherThread_);
    CloseHandle(p_recorder->hWatcherThreadRunning_);
    /* Cleanup actions */
    /* Close all the event handles */
    for (idx = 0; idx < NOTIFY_MARKS_COUNT; ++idx)
    {
        CloseHandle(p_recorder->notify_marks_[idx].hEventNotify);
    } 
    p_recorder->p_capture8_->Release();
    p_recorder->p_capture_buffer8_->Release();
    HeapFree(GetProcessHeap(), 0, p_recorder);
    debug_outputln("%4.4u %s", __LINE__, __FILE__);
}

extern "C" int dxaudio_recorder_start(dxaudio_recorder_t handle)
{
    int result = 0;
    HRESULT hr;
    assert(NULL != handle);
    hr = handle->p_capture_buffer8_->Start(DSCBSTART_LOOPING);
    if (SUCCEEDED(hr))
        result = 0;
    else
        debug_outputln("%4.4u %s %8.8x", __LINE__, __FILE__, hr);
    return result;
}

extern "C" int dxaudio_recorder_stop(dxaudio_recorder_t handle) 
{
    int result = 0;
    HRESULT hr;
    assert(NULL != handle);
    hr = handle->p_capture_buffer8_->Stop();
    if (SUCCEEDED(hr))
        result = 0;
    else
        debug_outputln("%4.4u %s %8.8x", __LINE__, __FILE__, hr);
    return result;
}

