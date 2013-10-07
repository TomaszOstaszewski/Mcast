/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file dsound-recorder.cpp
 * @brief A WAV capture object implementation.
 * @details 
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
#include "circular-buffer-uint16.h"
#include "recorder-settings.h"
#include "perf-counter-itf.h"

/**
 *
 */
#define NOTIFY_MARKS_COUNT (2)

/**
 *
 */
#define MAX_WAIT_TIMEOUT_FOR_THREAD (1000)

/**
 * @brief
 */
typedef enum capture_thread_state {
    CAPTURE_THREAD_IDLE, /*!< Capture thread idling. */
    CAPTURE_THREAD_WORKING, /*!< Capture thread doing capture. */
    CAPTURE_THREAD_EXITTING, /*!< Capture thread exitting. */
} capture_thread_state_t;

/*!
 * @brief The sound player descriptor. 
 * @details Gathers all the variables needed to successfully play chunks of PCM
 * data using DirectSound.
 */
typedef struct dxaudio_recorder_block {
    HANDLE hWatcherThread_;
    HANDLE hSignalStartRec_;
    HANDLE hSignalStopRec_;
    HANDLE hSignalExit_;
    struct recorder_settings * rec_settings_;
    void * context_;
    SEND_ROUTINE callback_;
} dxaudio_recorder_block_t;

typedef struct dxaudio_recorder_thread_information_block {
    IDirectSoundCaptureBuffer8 * p_capture_buffer8_; /*!< . */
    DWORD dw_notify_marks_begin_[NOTIFY_MARKS_COUNT];
    DSBPOSITIONNOTIFY notify_marks_[NOTIFY_MARKS_COUNT];
    HANDLE wait_object_handles_[3 + NOTIFY_MARKS_COUNT];
    capture_thread_state_t e_state_;
    IDirectSoundCapture8 * p_capture8_; /*!< */
} dxaudio_recorder_thread_information_block_t;

/**
 * 
 */
static HRESULT set_notification_positions(DSCBUFFERDESC const * p_buf_desc, 
    dxaudio_recorder_thread_information_block_t * p_tib, 
    IDirectSoundCaptureBuffer8 * p_capture_buffer8)
{
    HRESULT hr;
    size_t idx;
    hr = E_FAIL;
    for (idx = 0; idx < NOTIFY_MARKS_COUNT; ++idx)
    {
        p_tib->dw_notify_marks_begin_[idx] = ((p_buf_desc->dwBufferBytes*idx)/NOTIFY_MARKS_COUNT); 
        p_tib->notify_marks_[idx].dwOffset = ((p_buf_desc->dwBufferBytes*(idx+1))/NOTIFY_MARKS_COUNT) - 1; 
        p_tib->notify_marks_[idx].hEventNotify = ::CreateEvent(NULL, TRUE, FALSE, NULL);
        if (NULL == p_tib->notify_marks_[idx].hEventNotify)
            break;
        debug_outputln("%4.4u %s : %5u %5u %8.8x", __LINE__, __FILE__, 
                p_tib->dw_notify_marks_begin_[idx],     
                p_tib->notify_marks_[idx].dwOffset,
                p_tib->notify_marks_[idx].hEventNotify);
    } 
    if (idx == NOTIFY_MARKS_COUNT)
    {
        /* We set up to NOTIFY_MARKS_COUNT notification positions */
        IDirectSoundNotify8 * p_notify8 = NULL;
        hr = p_capture_buffer8->QueryInterface(IID_IDirectSoundNotify8, (void **)&p_notify8);
        if (SUCCEEDED(hr))
        {
            hr = p_notify8->SetNotificationPositions(NOTIFY_MARKS_COUNT, p_tib->notify_marks_);
            if (FAILED(hr))
            {
                debug_outputln("%4.4u %s %8.8x", __LINE__, __FILE__, hr);
            }
            p_notify8->Release();
        }
    }    
    else
    {
        while (idx-- >0)
        {
            ::CloseHandle(p_tib->notify_marks_[idx].hEventNotify);
        }
    }
    return hr;
}

/**
 * 
 */
static dxaudio_recorder_thread_information_block_t * create_audio_thred_information_block(
  struct recorder_settings const * p_settings, 
  void * context, SEND_ROUTINE p_send_routine)
{
    dxaudio_recorder_thread_information_block_t * p_retval = NULL;
    p_retval = (dxaudio_recorder_thread_information_block_t*)
        ::HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(dxaudio_recorder_thread_information_block_t));
    if (NULL != p_retval)
    {
        HRESULT hr;
        /* Get the reference to fifo queue. */
        /* 1st step - create the DirectSound capture object and get its interface */
        hr = DirectSoundCaptureCreate8(recorder_settings_get_guid(p_settings), &p_retval->p_capture8_, NULL);
        if (SUCCEEDED(hr))
        {
            IDirectSoundCaptureBuffer * p_capture_buffer = NULL;
            DSCBUFFERDESC dsc_buffer_desc = { 0 };
            WAVEFORMATEX wfx;
            /* 2nd step - create the DirectSound capture object and get its interface */
            ::CopyMemory(&wfx, recorder_settings_get_waveformatex(p_settings), sizeof(WAVEFORMATEX));
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
                    hr = set_notification_positions(&dsc_buffer_desc, p_retval, p_retval->p_capture_buffer8_);
                    if (SUCCEEDED(hr))
                    {
                        debug_outputln("%4.4u %s", __LINE__, __FILE__);
                        p_capture_buffer->Release();
                        return p_retval;
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

/**
 *
 */
static void destroy_audio_thread_information_block(dxaudio_recorder_thread_information_block_t * p_tib)
{
    size_t idx;
    for (idx = 0; idx < NOTIFY_MARKS_COUNT; ++idx)
    {
        ::CloseHandle(p_tib->notify_marks_[idx].hEventNotify);
    } 
    p_tib->p_capture_buffer8_->Release();
    p_tib->p_capture8_->Release();
    ::HeapFree(GetProcessHeap(), 0, p_tib);
}

static void recorder_thread_on_start_recording(dxaudio_recorder_thread_information_block_t * p_tib, HANDLE hEvent)
{
    HRESULT hr;
    ::ResetEvent(hEvent);
    hr = p_tib->p_capture_buffer8_->Start(DSCBSTART_LOOPING);
    debug_outputln("%4.4u %s : %8.8x", __LINE__, __FILE__, hr, ::GetLastError());
}

static void recorder_thread_on_stop_recording(dxaudio_recorder_thread_information_block_t * p_tib, HANDLE hEvent)
{
    HRESULT hr;
    ::ResetEvent(hEvent);
    hr = p_tib->p_capture_buffer8_->Stop();
    debug_outputln("%4.4u %s : %8.8x", __LINE__, __FILE__, hr, ::GetLastError());
}

static void recorder_thread_on_exit(dxaudio_recorder_thread_information_block_t * p_tib, HANDLE hEvent)
{
    HRESULT hr;
    ::ResetEvent(hEvent);
    debug_outputln("%4.4u %s : %8.8x", __LINE__, __FILE__, ::GetLastError());
    hr = p_tib->p_capture_buffer8_->Stop();
    p_tib->e_state_ = CAPTURE_THREAD_EXITTING;
}

static DWORD WINAPI recorder_thread(LPVOID param)
{
    HRESULT hr;
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr))
    {
        struct dxaudio_recorder_block * p_recorder;
        dxaudio_recorder_thread_information_block_t * p_tib;
        HANDLE * p_wait_handles;
        size_t idx;

        p_recorder = (struct dxaudio_recorder_block *)param;
        p_tib = create_audio_thred_information_block(p_recorder->rec_settings_, 
                p_recorder->context_, p_recorder->callback_);
        assert(NULL != p_tib);
        if (NULL != p_tib)
        {
            p_wait_handles = &p_tib->wait_object_handles_[0];
            assert(NULL != p_recorder);
            assert(NULL != p_recorder->hSignalStartRec_);
            assert(NULL != p_recorder->hSignalExit_);
            *p_wait_handles = p_recorder->hSignalStartRec_;
            ++p_wait_handles;
            *p_wait_handles = p_recorder->hSignalStopRec_;
            ++p_wait_handles;
            *p_wait_handles = p_recorder->hSignalExit_;
            ++p_wait_handles;
            for (idx  = 0; idx < NOTIFY_MARKS_COUNT; ++idx, ++p_wait_handles)
            {
                assert(NULL != p_tib->notify_marks_[idx].hEventNotify);
                *p_wait_handles = p_tib->notify_marks_[idx].hEventNotify;
            }
            while (CAPTURE_THREAD_EXITTING != p_tib->e_state_)
            {
                DWORD dwWaitResult;
                dwWaitResult = ::WaitForMultipleObjects(COUNTOF_ARRAY(p_tib->wait_object_handles_), 
                        &p_tib->wait_object_handles_[0], FALSE, INFINITE);
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
                                recorder_thread_on_start_recording(p_tib, p_tib->wait_object_handles_[0]);
                                break;
                            case 1:
                                recorder_thread_on_stop_recording(p_tib, p_tib->wait_object_handles_[1]);
                                break;
                            case 2:
                                recorder_thread_on_exit(p_tib, p_tib->wait_object_handles_[2]);
                                break;
                            default:
                                for (idx = 3; idx < 3+NOTIFY_MARKS_COUNT; ++idx)
                                {
                                    if (dwWaitResult - WAIT_OBJECT_0 == idx)
                                    {
                                        LPVOID p_ptr_1 = NULL, p_ptr_2 = NULL;
                                        DWORD dw_offset_1 = 0, dw_offset_2 = 0;
                                        /* Acknowledge notification */
                                        ::ResetEvent(p_tib->wait_object_handles_[idx]);
                                        /* Retrive data just captured */
                                        hr = p_tib->p_capture_buffer8_->Lock(p_tib->dw_notify_marks_begin_[idx-3],
                                                p_tib->notify_marks_[idx-3].dwOffset - p_tib->dw_notify_marks_begin_[idx-3] + 1,
                                                &p_ptr_1, &dw_offset_1,
                                                &p_ptr_2, &dw_offset_2,
                                                0);
                                        /* Call callback method */
                                        (p_recorder->callback_)(p_recorder->context_, p_ptr_1, dw_offset_1);
                                        /* Unlock buffer, let the capture to continue */
                                        p_tib->p_capture_buffer8_->Unlock(p_ptr_1, dw_offset_1, p_ptr_2, dw_offset_2);
                                    }
                                }
                                break;
                        } 
                        break;
                }
            }
            destroy_audio_thread_information_block(p_tib);
        }
        else
        {
            debug_outputln("%4.4u %s", __LINE__, __FILE__);
        }
        CoUninitialize();
    }
    else
    {
        debug_outputln("%4.4u %s : 0x%8.8x", __LINE__, __FILE__, hr);
    }
    return 0;
}

extern "C" dxaudio_recorder_t dxaudio_recorder_create(
        struct recorder_settings const * p_settings, 
        void * context, SEND_ROUTINE p_send_routine)
{
    struct dxaudio_recorder_block * p_retval = NULL;
    p_retval = (struct dxaudio_recorder_block *)::HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct dxaudio_recorder_block));
    if (NULL != p_retval)
    {
        p_retval->rec_settings_ = recorder_settings_get_default();
        recorder_settings_copy(p_retval->rec_settings_, p_settings);
        p_retval->callback_ = p_send_routine;
        p_retval->context_ = context;
        p_retval->hSignalStartRec_ = ::CreateEvent(NULL, TRUE, FALSE, NULL);
        if (NULL != p_retval->hSignalStartRec_)
        {
            p_retval->hSignalExit_ = ::CreateEvent(NULL, TRUE, FALSE, NULL);
            if (NULL != p_retval->hSignalExit_)
            {
                p_retval->hSignalStopRec_ = ::CreateEvent(NULL, TRUE, FALSE, NULL);
                if (NULL != p_retval->hSignalStopRec_)
                {
                    p_retval->hWatcherThread_ = ::CreateThread(NULL, 0, recorder_thread, p_retval, CREATE_SUSPENDED, NULL);
                    if (NULL != p_retval->hWatcherThread_)
                    {
                        ::ResumeThread(p_retval->hWatcherThread_);
                        return p_retval;
                    }
                    ::CloseHandle(p_retval->hSignalStopRec_);
                }
                ::CloseHandle(p_retval->hSignalExit_);
            }
            ::CloseHandle(p_retval->hSignalStartRec_);
        } 
    }
    return p_retval;
}

extern "C" void dxaudio_recorder_destroy(dxaudio_recorder_t handle) 
{
    DWORD dwWaitResult;
    struct dxaudio_recorder_block * p_recorder = (struct dxaudio_recorder_block* )handle;
    assert(NULL != p_recorder);
    /* Notify watcher thread that we are about to exit */
    ::SetEvent(p_recorder->hSignalExit_);
    /* Wait for notification ack */
    dwWaitResult = ::WaitForSingleObject(p_recorder->hWatcherThread_, MAX_WAIT_TIMEOUT_FOR_THREAD);
    debug_outputln("%4.4u %s : %8.8x %8.8x", __LINE__, __FILE__, dwWaitResult, WAIT_TIMEOUT);
    /* Cleanup actions */
    ::CloseHandle(p_recorder->hSignalStartRec_);
    ::CloseHandle(p_recorder->hSignalStopRec_);
    ::CloseHandle(p_recorder->hSignalExit_);
    ::CloseHandle(p_recorder->hWatcherThread_);
    /* Close all the event handles */
    ::HeapFree(GetProcessHeap(), 0, p_recorder);
    debug_outputln("%4.4u %s", __LINE__, __FILE__);
}

extern "C" int dxaudio_recorder_start(dxaudio_recorder_t handle)
{
    int result = 0;
    ::SetEvent(handle->hSignalStartRec_);
    return result;
}

extern "C" int dxaudio_recorder_stop(dxaudio_recorder_t handle) 
{
    int result = 0;
    ::SetEvent(handle->hSignalStopRec_);
    return result;
}

