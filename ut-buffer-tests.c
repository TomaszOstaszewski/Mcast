/* ex: set shiftwidth=4 tabstop=4 expandtab: */
/**
 * @file ut-buffer-tests.c
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char ** argv)
{
	int16_t read_counter, write_counter;
	uint8_t unsigned_diff, signed_diff;
	srand((unsigned int)time(NULL));
	read_counter = 0, write_counter = 0;
	unsigned_diff = (uint8_t)(write_counter - read_counter), signed_diff = (int8_t)(write_counter - read_counter);
	printf("%s %4.4d : %5.5hd %5.5hd %3.3hhd %3.3hhu\n", __FILE__, __LINE__, (uint16_t)read_counter, (uint16_t)write_counter, unsigned_diff, signed_diff);

	read_counter = 0, write_counter = 255;
	unsigned_diff = (uint8_t)(write_counter - read_counter), signed_diff = (int8_t)(write_counter - read_counter);
	printf("%s %4.4d : %5.5hd %5.5hd %3.3hhd %3.3hhu\n", __FILE__, __LINE__, (uint16_t)read_counter, (uint16_t)write_counter, unsigned_diff, signed_diff);

	read_counter = 254, write_counter = 255;
	unsigned_diff = (uint8_t)(write_counter - read_counter), signed_diff = (int8_t)(write_counter - read_counter);
	printf("%s %4.4d : %5.5hd %5.5hd %3.3hhd %3.3hhu\n", __FILE__, __LINE__, (uint16_t)read_counter, (uint16_t)write_counter, unsigned_diff, signed_diff);

	read_counter = 254, write_counter = 384;
	unsigned_diff = (uint8_t)(write_counter - read_counter), signed_diff = (int8_t)(write_counter - read_counter);
	printf("%s %4.4d : %5.5hd %5.5hd %3.3hhd %3.3hhu\n", __FILE__, __LINE__, (uint16_t)read_counter, (uint16_t)write_counter, unsigned_diff, signed_diff);

	read_counter = 254, write_counter = 512;
	unsigned_diff = (uint8_t)(write_counter - read_counter), signed_diff = (int8_t)(write_counter - read_counter);
	printf("%s %4.4d : %5.5hd %5.5hd %3.3hhd %3.3hhu\n", __FILE__, __LINE__, (uint16_t)read_counter, (uint16_t)write_counter, unsigned_diff, signed_diff);

	read_counter = 2, write_counter = 1;
	unsigned_diff = (uint8_t)(write_counter - read_counter), signed_diff = (int8_t)(write_counter - read_counter);
	printf("%s %4.4d : %5.5hd %5.5hd %3.3hhd %3.3hhu\n", __FILE__, __LINE__, (uint16_t)read_counter, (uint16_t)write_counter, unsigned_diff, signed_diff);

	read_counter = 5, write_counter = 1;
	unsigned_diff = (uint8_t)(write_counter - read_counter), signed_diff = (int8_t)(write_counter - read_counter);
	printf("%s %4.4d : %5.5hd %5.5hd %3.3hhd %3.3hhu\n", __FILE__, __LINE__, (uint16_t)read_counter, (uint16_t)write_counter, unsigned_diff, signed_diff);

	read_counter = 0, write_counter = 0;
	unsigned_diff = (uint8_t)(write_counter - read_counter), signed_diff = (int8_t)(write_counter - read_counter);
	printf("%s %4.4d : %5.5hd %5.5hd %3.3hhd %3.3hhu\n", __FILE__, __LINE__, (uint16_t)read_counter, (uint16_t)write_counter, unsigned_diff, signed_diff);
	++write_counter;
	if ((uint8_t)write_counter == (uint8_t)read_counter)
		++read_counter;
	printf("%s %4.4d : %5.5hd %5.5hd %3.3hhd %3.3hhu\n", __FILE__, __LINE__, (uint16_t)read_counter, (uint16_t)write_counter, unsigned_diff, signed_diff);
	return 0;
}

