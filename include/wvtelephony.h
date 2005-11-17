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
