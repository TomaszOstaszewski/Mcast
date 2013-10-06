/**
 *
 */
#include "pcc.h"
#include "dsbcaps-utils.h"
#include "debug_helpers.h"

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
    /* For that one we don't have a define unless we compile under Vista */
#if _WIN32_WINNT >= 0x0600
    { DSBCAPS_TRUEPLAYPOSITION, "DSBCAPS_TRUEPLAYPOSITION" },
#endif
};

/**
 * @brief Helper routine, gets the device capabilities.
 * @param[in] lpdsb pointer to the direct sound buffer, either primary or secondary.
 * @return returns 1 if succeeded, any other value indicates an error.
 * @sa http://bit.ly/zP10oa
 */
int get_buffer_caps(LPDIRECTSOUNDBUFFER lpdsb)
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
    return SUCCEEDED(hr);
}

