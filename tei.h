/* ex: set shiftwidth=4 tabstop=4 expandtab: */

/**
 * @file tei.h
 * @author T.Ostaszewski
 * @date 04-Jan-2012
 * @brief
 * @details
 */
#if !defined TEI_H_385986D7_6A0D_4691_A83A_CBE6A3F07B7F
#define TEI_H_385986D7_6A0D_4691_A83A_CBE6A3F07B7F

#include "winsock_adapter.h"
#define TEI_LENGHT (14)

typedef struct tei {
    uint8_t tei_data_[TEI_LENGHT];
} tei_t;

void get_next_tei(struct tei * p_tei);
void generate_random_tei(struct tei * p_tei);
void tei_2_string(const struct tei * p_tei, char * string, unsigned short string_length);

#endif /* defined TEI_H_385986D7_6A0D_4691_A83A_CBE6A3F07B7F */
