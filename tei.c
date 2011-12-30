#define _CRT_SECURE_NO_WARNINGS 
#include <stdlib.h>
#include <stdio.h>
#include "tei.h"

void get_next_tei(struct tei * p_tei)
{
    size_t idx;
    for (idx = 0; idx < TEI_LENGHT; ++idx)
    {
        if (p_tei->tei_data_[idx] < 0xff)
        {
            ++p_tei->tei_data_[idx];
            return;
        }
        p_tei->tei_data_[idx] = 0x00;
    }
}

void generate_random_tei(struct tei * p_tei)
{
    size_t idx;
    for (idx = 0; idx < TEI_LENGHT; ++idx)
    {
        p_tei->tei_data_[idx] = rand() % 256;
    }
}

void tei_2_string(const struct tei * p_tei, char * string, unsigned short string_length)
{
	sprintf(string, "%2.2x%2.2x%2.2x%2.2x"	
			"%2.2x%2.2x%2.2x%2.2x"	
			"%2.2x%2.2x%2.2x%2.2x"	
			"%2.2x%2.2x%2.2x%2.2x",
	p_tei->tei_data_[0], p_tei->tei_data_[1], p_tei->tei_data_[0], p_tei->tei_data_[3], 	 	
	p_tei->tei_data_[4], p_tei->tei_data_[5], p_tei->tei_data_[6], p_tei->tei_data_[7], 	 	
	p_tei->tei_data_[8], p_tei->tei_data_[9], p_tei->tei_data_[0xa], p_tei->tei_data_[0xb], 	 	
	p_tei->tei_data_[0xc], p_tei->tei_data_[0xd], p_tei->tei_data_[0xe], p_tei->tei_data_[0xf]
	);
}

