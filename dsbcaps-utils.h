/**
 *
 */

#if !defined DSCBCAPS_UTILS_H_7E22DE32_F500_4BD5_8820_1AE28831497F
#define DSCBCAPS_UTILS_H_7E22DE32_F500_4BD5_8820_1AE28831497F

#include <windows.h>
#include <dsound.h>

/**
 * @brief Helper routine, gets the device capabilities.
 * @param[in] lpdsb pointer to the direct sound buffer, either primary or secondary.
 * @return returns S_OK if succeeded, any other value indicates an error.
 * @sa http://bit.ly/zP10oa
 */
int get_buffer_caps(LPDIRECTSOUNDBUFFER lpdsb);

#endif /*if defined DSCBCAPS_UTILS_H_7E22DE32_F500_4BD5_8820_1AE28831497F */
