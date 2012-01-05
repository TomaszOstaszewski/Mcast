/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file std-int.h
 * @author T.Ostaszewski 
 * @date 04-Jan-2012
 * @brief Universal wrapper for <stdint.h> which is not available on Microsoft Visual prior to 2010.
 * @details Insted of including standard <stdint.h>, just include this file. The right version of the
 * stdint.h file (either vendor provided or home made wrapper) will be included, depending on the
 * compiler version you are using. Therefore, insted of doing something like:
 * @code
 * #if ...determine if the MSVC version is less then VC2010...
 * #	include "vc2003stdint.h"
 * #else
 * #	include <stdint.h>
 * #endif
 * @endcode
 * you write
 * @code
 * #include "std-int.h"
 * @endcode
 * and the matter is handled automatically.
 */
#if !defined STD_INT_H_809467C6_E3E1_479D_95E9_98B4CB49D026
#define STD_INT_H_809467C6_E3E1_479D_95E9_98B4CB49D026

#if defined _MSC_VER && _MSC_VER < 1600
#if defined USE_OWN_STDINT_H 
/* VC2003 does not have <stdint.h>.
 * Use the one provided by Mike Acton 
 */
#include "vc2003stdint.h"
#endif
#else
/* VC2010 has its own <stdint.h> */
#include <stdint.h>
#endif

#endif /* STD_INT_H_809467C6_E3E1_479D_95E9_98B4CB49D026 */

