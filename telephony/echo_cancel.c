
#include "wvtelephony.h"
#include <stdio.h>

#define SQ(x) ((x)*(x))
#define ROUND_DIV(a,b) (((a) + ((b)/2))/(b))
 
void echo_cancel(const short *out_buf,
        const short *in,
        size_t block_size,
        int *filter_q16,
        size_t num_taps,
        short *echo_cancelled_in)
{
    const short *cur_out = &out_buf[block_size-1];
    const short *cur_in = &in[block_size-1];
    short *cur_in_ec = &echo_cancelled_in[block_size-1];
    
    const short *out_p;
    int out_norm_sq_p16; /* The value is * 2^16 */

    /* Calculate initial norm squared of output vector
     */
    out_norm_sq_p16 = 0;
    for (out_p = &out_buf[block_size+num_taps-1];
            out_p != &out_buf[block_size-1];
            --out_p)
    {
        out_norm_sq_p16 += SQ(ROUND_DIV((int) *out_p, 1<<8));
    }

    do
    {
        int *filter_q16_p;
        int echo_est_q16;
        short echo_est;
        
        /* Caculate echo estimate for this sample using output data
         */
        echo_est_q16 = 0;
        for (out_p = &cur_out[num_taps-1],
                filter_q16_p = &filter_q16[num_taps-1];
                out_p != cur_out;
                --out_p, --filter_q16_p)
        {
            echo_est_q16 += (int) *out_p * *filter_q16_p;
        }
        echo_est = ROUND_DIV(echo_est_q16, 1<<16);
        
        /* Echo cancelled input is simply input minus echo estimate
         * Round echo_est_q16 to nearest int when convering to short
         * This can also be interpreted as the error term, which
         * is used for the NLMS correction below
         */
        *cur_in_ec = *cur_in - echo_est;

        /* Update norm squared of output vector
         */
        out_norm_sq_p16 += SQ(ROUND_DIV((int) cur_out[0], 1<<8))
            - SQ(ROUND_DIV((int) cur_out[num_taps], 1<<8));

        /* Update filter tap weights using NLMS correction
         */
        if (out_norm_sq_p16 != 0)
        {
            for (out_p = &cur_out[num_taps-1],
                    filter_q16_p = &filter_q16[num_taps-1];
                    out_p != cur_out;
                    --out_p, --filter_q16_p)
            {
                *filter_q16_p += 
                    ROUND_DIV((int) *cur_in_ec * (int) *out_p, out_norm_sq_p16);
            }
        }
    } while (cur_out--, cur_in_ec--, cur_in-- != in);
}

