/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * An FFT abstraction.
 */
#ifndef __WVFFT_H
#define __WVFFT_H

#include "wvencoder.h"
#include "wvbuffer.h"

/**
 * Computes the forward FFT transformation of real valued input
 *   to complex output.
 * Input buffer must contain a sequence of 'double' type
 *   values in machine order representing data in the time domain.
 * Output buffer will contain a sequence of values ordered in the
 *   following way.  Let 'n' be the total number of values, and
 *   'i' be an index into the buffer, then:
 *     buf[0]   : the DC coefficient
 *     buf[i]   : the real component of the i'th frequency band
 *     buf[n-i] : the complex component of the i'th frequency band
 *   Hence data for only n/2 of the n frequency bands is output.
 *   This is because the latter bands are merely the complex conjugate
 *   of the former.
 */
struct fftw_plan_struct;
class WvRealToComplexFFTEncoder : public WvEncoder
{
    struct fftw_plan_struct *plan;
    size_t n;
    
public:
    WvRealToComplexFFTEncoder(size_t _n);
    virtual ~WvRealToComplexFFTEncoder();

protected:
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf, bool flush);
    virtual bool _reset();
};


class WvComplexToRealFFTEncoder : public WvEncoder
{
    struct fftw_plan_struct *plan;
    size_t n;
    WvInPlaceBuffer tmpbuf;
    
public:
    WvComplexToRealFFTEncoder(size_t _n);
    virtual ~WvComplexToRealFFTEncoder();

protected:
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf, bool flush);
    virtual bool _reset();
};

#endif // __WVFFT_H
