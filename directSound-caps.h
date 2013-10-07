/**
 * 
 */
#if !defined DIRECTSOUND_CAPS_H_9D295AAF_600E_4AE2_8394_4DDDFC5B044E
#define DIRECTSOUND_CAPS_H_9D295AAF_600E_4AE2_8394_4DDDFC5B044E

#include <windows.h>
#include <DSound.h>

#if defined __cplusplus
extern "C" {
#endif

size_t get_caps_desc(DSCAPS const * p_caps, char * p_buffer, size_t idx);

#if defined __cplusplus
}
#endif

#endif /* if !defined DIRECTSOUND_CAPS_H_9D295AAF_600E_4AE2_8394_4DDDFC5B044E */
