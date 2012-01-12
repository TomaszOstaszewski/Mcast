/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/*!
 * @file dsoundplay.cpp
 * @author T. Ostaszewski
 * @date 30-Nov-2010
 * @brief DirectSound WAV playing file.
 */
#include "pcc.h"
#include "debug_helpers.h"
#include "dsoundplay.h"
#include "wave_utils.h"
#include "fifo-circular-buffer.h"
#include "input-buffer.h"

/*!
 * @brief
 */
struct dsound_data {
    WAVEFORMATEX wfe_;
    LPDIRECTSOUND8          p_direct_sound_8_;              /*!< */
    LPDIRECTSOUNDBUFFER     p_primary_sound_buffer_;        /*!< */
    LPDIRECTSOUNDBUFFER8    p_secondary_sound_buffer_;      /*!< */
    MMRESULT timer_;                                        /*!< */
    BOOL buf_1_filled_;                                     /*!< */
    BOOL buf_2_filled_;                                     /*!< */
    size_t                  nHalfBufferSize_;               /*!< */
    struct fifo_circular_buffer * fifo_;
};

/**
 * @brief Helper routine, gets the device capabilities.
 * @param[in] lpdsb
 * @return returns S_OK if succeeded, any other value indicates an error.
 * @sa http://bit.ly/zP10oa
 */
static HRESULT get_buffer_caps(LPDIRECTSOUNDBUFFER8 lpdsb)
{
    DSBCAPS caps;
    ZeroMemory(&caps, sizeof(DSBCAPS));
    caps.dwSize = sizeof(DSBCAPS);
    HRESULT hr = lpdsb->GetCaps(&caps);
    if (SUCCEEDED(hr))
    {
        debug_outputln("%s %5.5d : %8.8u %8.8x %8.8u %8.8u %8.8u"
                ,__FILE__, __LINE__
                ,caps.dwSize
                ,caps.dwFlags
                ,caps.dwBufferBytes
                ,caps.dwUnlockTransferRate
                ,caps.dwPlayCpuOverhead
                );    
    }
    else
    {
        debug_outputln("%s %5.5d : %8.8x", __FILE__, __LINE__, hr);    
    }
    return hr;
}

/**
 * @brief Creates the buffer
 * @param[in] dwOffset 
 * @param[in] req_size
 * @param[in] p_buffer - 
 * @param[in] p_input_buffer_desc
 * @return
 */
static HRESULT create_buffers(LPDIRECTSOUND8 p_direct_sound_8, 
        LPDIRECTSOUNDBUFFER * pp_primary_buffer, 
        LPDIRECTSOUNDBUFFER8 * pp_secondary_buffer, 
        WAVEFORMATEX * p_wfe, 
        size_t half_buffer_size)
{
    HRESULT hr;
    DSBUFFERDESC bufferDesc;
    LPDIRECTSOUNDBUFFER lpDSB = NULL;
    if (NULL == pp_primary_buffer || NULL == pp_secondary_buffer || NULL != *pp_primary_buffer || NULL != *pp_secondary_buffer)
    {
        debug_outputln("%s %5.5d : %p %p %p %p", __FILE__, __LINE__, pp_primary_buffer, pp_secondary_buffer, *pp_primary_buffer, *pp_secondary_buffer);
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
        debug_outputln("%s %5.5d : %x", __FILE__, __LINE__, hr);
        goto error;
    }
    hr = (*pp_primary_buffer)->SetFormat(p_wfe);
    if (FAILED(hr))
    {
        debug_outputln("%s %5.5d : %8.8x", __FILE__, __LINE__, hr);
        goto error;
    }
    /* Secondary buffer */
    bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
    bufferDesc.dwBufferBytes = 2*half_buffer_size; /* double buffering - one buffer being played, whereas the other is being filled in */
    bufferDesc.lpwfxFormat = p_wfe;
    hr = p_direct_sound_8->CreateSoundBuffer(&bufferDesc, &lpDSB, NULL);
    if (FAILED(hr))
    {
        debug_outputln("%s %5.5d : %x", __FILE__, __LINE__, hr);
        goto error;
    }
    hr = lpDSB->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)pp_secondary_buffer);
    if (FAILED(hr))
    {
        debug_outputln("%s %5.5d : %x", __FILE__, __LINE__, hr);
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

/**
 * @brief 
 * @param[in] dwOffset 
 * @param[in] req_size
 * @param[in] p_buffer - 
 * @param[in] p_input_buffer_desc
 * @return
 */
static HRESULT fill_buffer(DWORD dwOffset, DWORD req_size, LPDIRECTSOUNDBUFFER8 p_buffer, struct buffer_desc * p_input_buffer_desc)
{
    uint8_t *lpvWrite1, *lpvWrite2;
    DWORD dwLength1, dwLength2;
    HRESULT hr;
    hr = p_buffer->Lock(
            dwOffset,               // Offset at which to start lock.
            req_size,               // Size of lock;
            (LPVOID*)&lpvWrite1,    // Gets address of first part of lock.
            &dwLength1,             // Gets size of first part of lock.
            (LPVOID*)&lpvWrite2,    // Address of wraparound not needed. 
            &dwLength2,             // Size of wraparound not needed.
            0);                     // Flag.
    if (SUCCEEDED(hr))
    {
        if (NULL != lpvWrite2 || 0 != dwLength2)
        {
            debug_outputln("%s %5.5d : %p %u", __FILE__, __LINE__, lpvWrite2, dwLength2);
        }
        else
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
        }
        hr = p_buffer->Unlock(
                lpvWrite1,   // Address of lock start.
                dwLength1,   // Size of lock.
                lpvWrite2,   // Wraparound 
                dwLength2);  // Wraparound size 
    }
    return hr;
}

/*!
 * @brief
 */
static void play_data_chunk(struct dsound_data * p_ds_data) 
{
    DWORD dwRead, dwWrite;
    HRESULT hr;
    hr = p_ds_data->p_secondary_sound_buffer_->GetCurrentPosition(&dwRead, &dwWrite);
    struct data_item data;
    struct buffer_desc buf_desc;
    data.data_ = (uint8_t*)alloca(p_ds_data->nHalfBufferSize_);
    data.count_ = p_ds_data->nHalfBufferSize_;
    buf_desc.p_begin_ = data.data_;
    buf_desc.nMaxOffset_ = p_ds_data->nHalfBufferSize_;
    buf_desc.nCurrentOffset_ = 0;
    if (SUCCEEDED(hr))
    {
        /* Check if both write & read cursor is in the 1st buffer and we have not yet filled 2nd buffer */
        if (dwWrite < p_ds_data->nHalfBufferSize_ && dwRead < p_ds_data->nHalfBufferSize_ && FALSE == p_ds_data->buf_2_filled_)
        {
            /* Both read and write cursor in 1st buffer, 2nd buffer not yet filled - fill it now. */
            //debug_outputln("%s %5.5d : %8.8u %8.8u", __FILE__, __LINE__, dwRead, dwWrite);
            p_ds_data->buf_2_filled_ = TRUE, p_ds_data->buf_1_filled_ = FALSE;
            if (fifo_circular_buffer_get_items_count(p_ds_data->fifo_)>0)
            {
                fifo_circular_buffer_fetch_item(p_ds_data->fifo_, &data);
            }
            else
            {
                memset(data.data_, 0, sizeof(uint8_t)*data.count_);
            }
            hr = fill_buffer(p_ds_data->nHalfBufferSize_, p_ds_data->nHalfBufferSize_, p_ds_data->p_secondary_sound_buffer_, &buf_desc);
        }
        /* Check if both write & read cursor is in the 2nd buffer and we have not yet filled 1st buffer */
        else if (dwWrite > p_ds_data->nHalfBufferSize_ && dwRead > p_ds_data->nHalfBufferSize_ && FALSE == p_ds_data->buf_1_filled_)
        {
            /* Both read and write cursor in 2nd buffer, 1st buffer not yet filled - fill it now. */
            //debug_outputln("%s %5.5d : %8.8u %8.8u", __FILE__, __LINE__, dwRead, dwWrite);
            p_ds_data->buf_1_filled_ = TRUE, p_ds_data->buf_2_filled_ = FALSE;
            if (fifo_circular_buffer_get_items_count(p_ds_data->fifo_)>0)
            {
                fifo_circular_buffer_fetch_item(p_ds_data->fifo_, &data);
            }
            else
            {
                memset(data.data_, 0, sizeof(uint8_t)*data.count_);
            }
            hr = fill_buffer(0, p_ds_data->nHalfBufferSize_, p_ds_data->p_secondary_sound_buffer_, &buf_desc);
        }
        else
		{
			/* read cursor in 1st buffer, write cursor in 2nd buffer 
			 * or
			 * read cursor in 2nd buffer, write cursor in 1st buffer 
			 * in either case - don't do anything. 
			 * Rationale:
			 * 1/ We are to close to buffer boundaries to undertake any action without risk of overwritting buffer already being played.
			 * 2/ The buffer to be filled with data might have well been filled with data already.
			 */ 
		}
    }   
    else
    {
        //debug_outputln("%s %d : %8.8x", __FILE__, __LINE__, hr);
    }
    return;
}

/*!
 * @brief
 */
static void CALLBACK sTimerCallback(UINT uTimerID, UINT uMsg, DWORD dwUser, 
        DWORD dw1 /* reserved - do not use */, 
        DWORD dw2 /* reserved * - do not use */) 
{
    struct dsound_data * p_ds_data = (struct dsound_data *)dwUser;
    play_data_chunk(p_ds_data);
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
 * debug_outputln("%s %5.5d", __FILE__, __LINE__);
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
        debug_outputln("%s %5.5d : %x", __FILE__, __LINE__, hr);
        goto error_0;
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
        debug_outputln("%s %5.5d : %x", __FILE__, __LINE__, hr);
        goto error_0;
    }
    CopyMemory(&p_ds_data->wfe_, p_WFE, sizeof(WAVEFORMATEX));
    p_ds_data->nHalfBufferSize_ = p_ds_data->wfe_.nAvgBytesPerSec/8; /* One 20th of a second will be played at time. */
    dump_waveformatex(p_WFE);
    debug_outputln("%s %5.5d : %8.8u", __FILE__, __LINE__, p_ds_data->nHalfBufferSize_);
    hr = create_buffers(p_ds_data->p_direct_sound_8_, &p_ds_data->p_primary_sound_buffer_, &p_ds_data->p_secondary_sound_buffer_, &p_ds_data->wfe_, p_ds_data->nHalfBufferSize_);
    if (FAILED(hr))
    {
        debug_outputln("%s %5.5d : %x", __FILE__, __LINE__, hr);
        goto error_3;
    }
    return hr;
error_3:
    p_ds_data->p_secondary_sound_buffer_->Release();    
    p_ds_data->p_secondary_sound_buffer_ = NULL;    
    p_ds_data->p_primary_sound_buffer_->Release();
    p_ds_data->p_primary_sound_buffer_ = NULL;
error_0:
    p_ds_data->p_direct_sound_8_->Release();
    p_ds_data->p_direct_sound_8_ = NULL;
    return hr;
}

extern "C" DSOUNDPLAY dsoundplayer_create(HWND hWnd, WAVEFORMATEX const * p_WFE, struct fifo_circular_buffer * fifo)
{
    HRESULT hr;
    struct dsound_data * p_retval = (struct dsound_data*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct dsound_data));
    debug_outputln("%s %5.5d", __FILE__, __LINE__);
    if (NULL != p_retval)
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
        hr = init_ds_data(hWnd, p_WFE, p_retval);
        debug_outputln("%s %5.5d : %8.8x", __FILE__, __LINE__, hr);
        if (SUCCEEDED(hr))
        {
            debug_outputln("%s %5.5d", __FILE__, __LINE__);
            /* Prefetch frist data and fill the buffer */
            p_retval->fifo_ = fifo;
            return (DSOUNDPLAY)(p_retval);
        }
    }
    debug_outputln("%s %5.5d", __FILE__, __LINE__);
    HeapFree(GetProcessHeap(), 0, p_retval);
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

extern "C" void dsoundplayer_play(DSOUNDPLAY handle) 
{
    HRESULT hr;
    struct dsound_data * p_ds_data = (struct dsound_data*)handle;
    LPDIRECTSOUNDBUFFER8 lpdsbStatic = p_ds_data->p_secondary_sound_buffer_;
    p_ds_data->timer_ = timeSetEvent(1, 1, &sTimerCallback, (DWORD)p_ds_data, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
    if (NULL == p_ds_data->timer_)
    {
        debug_outputln("%s %5.5d", __FILE__, __LINE__);
        return;
    }   
    lpdsbStatic->SetCurrentPosition(0);
    hr = lpdsbStatic->Play(
            0,  // Unused.
            0,  // Priority for voice management.
            DSBPLAY_LOOPING); // Flags.
    if (FAILED(hr))
    {
        debug_outputln("%s %5.5d : %8.8x", __FILE__, __LINE__, hr);
    }
    debug_outputln("%s %5.5d : %8.8x", __FILE__, __LINE__, hr);
}

extern "C" void dsoundplayer_pause(DSOUNDPLAY handle) 
{
}

extern "C" void dsoundplayer_stop(DSOUNDPLAY handle) 
{
    struct dsound_data * p_ds_data = (struct dsound_data*)handle;
    LPDIRECTSOUNDBUFFER8 lpdsbStatic = p_ds_data->p_secondary_sound_buffer_;
    HRESULT hr = lpdsbStatic->Stop();
    if (FAILED(hr))
    {
        debug_outputln("%s %5.5d : %8.8x", __FILE__, __LINE__, hr);
    }
    MMRESULT res = timeKillEvent(p_ds_data->timer_);
    p_ds_data->buf_1_filled_ = FALSE;
    p_ds_data->buf_2_filled_ = FALSE;
    switch (res)
    {
        case TIMERR_NOERROR:
            break;
        case MMSYSERR_INVALPARAM:
            debug_outputln("%s %5.5d", __FILE__, __LINE__);
            break;
        default:
            debug_outputln("%s %5.5d", __FILE__, __LINE__);
            break;
    }
    debug_outputln("%s %5.5d : %8.8x %u", __FILE__, __LINE__, hr, res);
}

