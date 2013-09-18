/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file ut-abstract-tone.c
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
 * @date 04-Jan-2012
 * @brief 
 * @details 
 */
#include "pcc.h"
#include <mmsystem.h>
#include "abstract-tone.h"
#include "sender-res.h"

void test_00(void)
{
	struct abstract_tone * p_tone;
	p_tone = abstract_tone_create(EMBEDDED_TEST_TONE, MAKEINTRESOURCE(IDR_0_1));
	assert(p_tone);
	abstract_tone_destroy(p_tone);
}

void test_01(void)
{
	struct abstract_tone * p_tone;
    uint8_t const * p_data;
    size_t data_size = 0;
	p_tone = abstract_tone_create(EMBEDDED_TEST_TONE, MAKEINTRESOURCE(IDR_0_1));
	assert(p_tone);
    p_data = (uint8_t const *)abstract_tone_get_wave_data(p_tone, &data_size);
    assert(p_data);
    assert(0 != data_size);
	abstract_tone_destroy(p_tone);
}

void test_02(void)
{
	struct abstract_tone * p_tone;
	p_tone = abstract_tone_create(EMBEDDED_TEST_TONE, MAKEINTRESOURCE(IDR_0_1));
	assert(p_tone);
	abstract_tone_destroy(p_tone);
}

int main(int argc, char ** argv)
{
	test_00();
	test_01();
	test_02();
	return 0;
}

