/* -*- Mode: C -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * NLMS echo canceller
 *
 * This is all implemented in plain old C and fixed-point arithmetic so that
 * it cooperates will with kernel drivers.
 */

#include "wvtelephony.h"
#include <stdio.h>

#define SQ(x) ((x)*(x))
#define ROUND_DIV(a,b) (((a) + ((b)/2))/(b))
 
/**
 * @brief Fixed-point NLMS adaptive echo canceller
 * @author Peter Zion
 * @date 10 Dec 2003
 *
 * This is a fixed-point normalized least mean squares adaptive echo canceller.
 * It assumes
 * that the input stream is a FIR filter of the output stream, and adapively
 * builds the filter.  In this way, we only need to know the maximum number of
 * taps of the filter (corresponding the the maximum lag of the echo) and
 * the echo canceller should automatically adjust itself to any changes in the
 * echo FIR.
 *
 * The input data is assumed to be signed 16-bit PCM samples.   The filter 
 * coefficients are in fixed point Q16 format -- each int tap weight
 * is actually *2^-16.
 *
 * Sample usage:
 *
 * @code
 * #include <string.h> // For memmove
 * 
 * #define NUM_TAPS 256
 * #define BLOCK_SIZE 64
 * 
 * extern int end_now(void);
 * extern void play_record_local(const short *play, short *record,
 *     size_t block_size);
 * extern void play_record_remote(const short *play, short *record,
 *     size_t block_size);
 * 
 * short out_buf[BLOCK_SIZE+NUM_TAPS];
 * int filter_q16[NUM_TAPS];
 *
 * memset(out_buf, 0, sizeof (out_buf));
 * memset(filter_q16, 0, sizeof (filter_q16)); // The filter tap weights must
 *                                             // initially be zero!!
 * 
 * while (!end_now())
 * {
 *     short in[BLOCK_SIZE];
 *     short echo_cancelled_in[BLOCK_SIZE];
 *     
 *     // Output BLOCK_SIZE samples to the speaker from outBuf and input 
 *     // BLOCK_SIZE samples into inBuf from the microphone.
 *     play_record_local(outBuf, in, BLOCK_SIZE);
 *
 *     // Cancel the echo of out present on in using the outBuf buffered
 *     // samples.  Adaptively updates the filter tap weights as it goes.
 *     echo_cancel(out_buf, in, BLOCK_SIZE, filter_q16, NUM_TAPS,
 *         echo_cancelled_in);
 *
 *     // Shift samples in preparation to get more data...
 *     memmove (&out_buf[BLOCK_SIZE], &out_buf[0],
 *         sizeof (out_buf) - sizeof (in));
 *     
 *     // Send the echo cancelled input and receive to/from remote source
 *     play_record_remote(echo_cancelled_in, out, BLOCK_SIZE);
 * }
 * @endcode
 *
 * @param out_buf
 *        The buffered output sanmples.  An array of shorts of
 *        length at least block_size + num_taps
 * @param in
 *        The input samples.  An array of shorts of length at least block_size
 * @param block_size
 *        The number of output and input samples in this block.  Must be at
 *        least 1.
 * @param filter_q16
 *        The filter tap weights. An array of ints in Q16 format of length
 *        num_taps.  It should initially be zero and will be updated by this
 *        funtion.
 * @param num_taps
 *        The number of taps in the filter.  Should be at least enough to
 *        capture the echo!  Must be at least 1.
 * @param echo_cancelled_in
 *        The resulting input samples after the echo has been cancelled.  An
 *        array of shorts of length at least block_size.
 * 
 * @note The computational order of the function is O(block_size * num_taps)
 * @note If the output and/or input signals have a DC offset, it should
 *       be removed before calling this function!
 *
 * @bug This code assumes that the short type is 16 bit signed and the
 *      int type is 32 bit signed.
 * @bug Overflow is possible (but unlikely), in particular if there is a DC
 *      offset present in the output or input
 */
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

