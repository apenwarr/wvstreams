/* -*- Mode: C -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * Telephony routines: echo cancellation, dc offset removal, automatic
 * gain control, etc.
 *
 * This is all implemented in plain old C so that it cooperates will with
 * kernel drivers.
 */
#ifndef TELEPHONY_H
#define TELEPHONY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h> /* To get size_t */

/*!
 * \fn void echo_cancel(const short *out_buf,
 *                      const short *in,
 *                      size_t block_size,
 *                      int *filter_q16,
 *                      size_t filter_size,
 *                      short *echo_cancelled_in)
 * \brief Fixed-point NLMS adaptive echo canceller
 * \author Peter Zion
 * \date 10 Dec 2003
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
 * \code
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
 * \endcode
 *
 * \param out_buf
 *        The buffered output sanmples.  An array of shorts of
 *        length at least block_size + num_taps
 * \param in
 *        The input samples.  An array of shorts of length at least block_size
 * \param block_size
 *        The number of output and input samples in this block.  Must be at
 *        least 1.
 * \param filter_q16
 *        The filter tap weights. An array of ints in Q16 format of length
 *        num_taps.  It should initially be zero and will be updated by this
 *        funtion.
 * \param num_taps
 *        The number of taps in the filter.  Should be at least enough to
 *        capture the echo!  Must be at least 1.
 * \param echo_cancelled_in
 *        The resulting input samples after the echo has been cancelled.  An
 *        array of shorts of length at least block_size.
 * 
 * \note The computational order of the function is O(block_size * num_taps)
 * \note If the output and/or input signals have a DC offset, it should
 *       be removed before calling this function!
 *
 * \bug This code assumes that the short time is 16 bit signed and the
 *      int type is 32 bit signed.
 * \bug Overflow is possible if there is a DC offset present in the output
 *      or input
 */
extern void echo_cancel(const short *out_buf,
        const short *in,
        size_t block_size,
        int *filter_q16,
        size_t filter_size,
        short *echo_cancelled_in);

#ifdef __cplusplus
};
#endif
#endif
