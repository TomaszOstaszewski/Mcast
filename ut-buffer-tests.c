/*!
 * 
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

