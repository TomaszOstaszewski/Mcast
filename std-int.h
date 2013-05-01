/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file std-int.h
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
 * @author T.Ostaszewski
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
 * @date 04-Jan-2012
 */
#if !defined STD_INT_H_809467C6_E3E1_479D_95E9_98B4CB49D026
#define STD_INT_H_809467C6_E3E1_479D_95E9_98B4CB49D026

#if defined _MSC_VER && _MSC_VER < 1500
/* VC2003 does not have <stdint.h>.
 * Use the one provided by Mike Acton 
 */
#include "vc2003stdint.h"
#elif defined _MSC_VER && _MSC_VER < 1600
/* VC2010 has its own <stdint.h> */
#include "vc2009stdint.h"
#else
#include <stdint.h>
#endif

#endif /* STD_INT_H_809467C6_E3E1_479D_95E9_98B4CB49D026 */

