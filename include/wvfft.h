/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * An FFT abstraction.
 */
#ifndef __WVFFT_H
#define __WVFFT_H

#include "wvtypedencoder.h"
#include "wvbuffer.h"

/**
 * Computes the forward FFT transformation of real valued input
 *   to complex output.  The result is unnormalized.
 * Input buffer must contain a sequence of 'double' type
 *   values in machine order representing samples in the time domain.
 * Output buffer will contain a sequence of 'double' type
 *   values in machine order representing data in the frequency domain.
 *
 *   The data is ordered as follows:
 *     Let 'n' be the total number of values.
 *     Let 'i' be an index into the buffer.
 *
 *     buf[0]   : the DC coefficient
 *     buf[i]   : the real component of the i'th frequency band
 *     buf[n-i] : the complex component of the i'th frequency band
 *
 *   Hence data for only n/2 of the n frequency bands is output.
 *   This is because the latter bands are merely the complex conjugate
 *   of the former.
 */
struct fftw_plan_struct;
class WvRealToComplexFFTEncoder :
    public WvTypedEncoder<double, double>
{
public:
    enum WindowFunction {
        WND_NONE,  // no windowing
        WND_BOXCAR // after one FFT step, returns half of the sample
                   // to the input buffer to be processed again in
                   // the next step
    };
    WvRealToComplexFFTEncoder(size_t _n,
        WindowFunction _wnd = WND_NONE);
    virtual ~WvRealToComplexFFTEncoder();

protected:
    /**
     * If not flushing, only processes at most one block of data.
     */
    virtual bool _typedencode(IBuffer &inbuf, OBuffer &outbuf, bool flush);
    virtual bool _reset();

private:
    struct fftw_plan_struct *plan;
    size_t n;
    WindowFunction wnd;
};


/**
 * Computes the backward FFT transformation of complex valued input
 *   to real output.  The result is unnormalized.
 * Input buffer must contain a sequence of 'double' type
 *   values in machine order representing data in the frequency domain.
 *   The data must be organized in the same fashion as that returned
 *   by WvRealToComplexFFTEncoder.
 * Output buffer will contain a sequence of 'double' type
 *   values in machine order representing samples in the time domain.
 */
class WvComplexToRealFFTEncoder :
    public WvTypedEncoder<double, double>
{
    struct fftw_plan_struct *plan;
    size_t n;
    WvInPlaceBufferBase<double> tmpbuf;
    
public:
    WvComplexToRealFFTEncoder(size_t _n);
    virtual ~WvComplexToRealFFTEncoder();

protected:
    /**
     * If not flushing, only processes at most one block of data.
     */
    virtual bool _typedencode(IBuffer &inbuf, OBuffer &outbuf, bool flush);
    virtual bool _reset();
};


/**
 * Computes a power spectrum from complex values input.
 * Input buffer must contain a sequence of 'double' type
 *   values in machine order representing data in the frequency domain.
 *   The data must be organized in the same fashion as that returned
 *   by WvRealToComplexFFTEncoder.
 * Output buffer will contain a sequence of 'double' type
 *   values in machine order representing the power spectrum.
 *   Only the power coefficients for (n/2)+1 bands are output.
 *
 *   The data is ordered as follows:
 *     Let 'n' be the total number of values.
 *     Let 'i' be an index into the buffer.
 *
 *     buf[0] : the squared DC coefficient
 *     buf[i] : the squared power of the i'th band
 */
class WvPowerSpectrumEncoder :
    public WvTypedEncoder<double, double>
{
    size_t n, half, mid;

public:
    WvPowerSpectrumEncoder(size_t _n);

protected:
    /**
     * If not flushing, only processes at most one block of data.
     */
    virtual bool _typedencode(IBuffer &inbuf, OBuffer &outbuf, bool flush);
    virtual bool _reset();
};

#endif // __WVFFT_H
