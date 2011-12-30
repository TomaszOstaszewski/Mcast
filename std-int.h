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

