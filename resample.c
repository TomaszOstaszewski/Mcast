#include <stdio.h>
#include  "std-int.h"

uint16_t resample_16_bit_11025kHz_to_8kHz(uint16_t const * in_samples, 
        uint16_t count_in_samples,
        uint16_t * out_samples, 
        uint16_t count_out_samples)
{
    /**
     * @brief Value 320 is taken from 441*25 = 11025. Is the relative prime factor of 8000 and 11025.
     */
#define INPUT_SAMPLE 441 
    /**
     * @brief Value 320 is taken from 320*25 = 8000. Is the relative prime factor of 8000 and 11025.
     */
#define OUTPUT_SAMPLE 320
    int16_t error = 0;
    uint16_t input_idx;
    uint16_t output_idx;
    for (input_idx = 0, output_idx = 0; 
            output_idx < count_out_samples && input_idx < count_in_samples; 
            ++input_idx)
    {
        out_samples[output_idx] = in_samples[input_idx];
        if (2 * error < INPUT_SAMPLE - 2 * OUTPUT_SAMPLE)
        {
            error = error + OUTPUT_SAMPLE; 
        }
        else
        {
            error = error + OUTPUT_SAMPLE - INPUT_SAMPLE; 
            ++output_idx;
        }
    }
    return output_idx;

}

int main(int argc, char ** argv)
{
    uint16_t input[220];
    uint16_t output[161];
    uint16_t out_samples;
    size_t idx;
    memset(input, 0, sizeof(input));
    memset(input, 0, sizeof(output));
    
    for (idx = 0; idx < sizeof(input); ++idx)
        input[idx] = idx;
    out_samples = resample_16_bit_11025kHz_to_8kHz(input, sizeof(input), output, sizeof(output));    
    for (idx = 0; idx < sizeof(output); ++idx)
    {
        printf("%d,", output[idx]);
        if (idx % 25)
            printf("\n");
    }
    printf("Completed.");
    return 0;
}

