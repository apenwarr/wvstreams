/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * An FFT abstraction.
 */
#ifndef __WVFFT_H
#define __WVFFT_H

#include "wvtypedencoder.h"
#include "wvbuffer.h"

struct fftw_plan_struct;

/**
 * Computes the forward FFT transformation of real valued input
 * to <em>unnormalized</em> complex output.
 * <p>
 * Input buffer must contain a sequence of 'double' type
 * values in machine order representing samples in the time domain.
 * </p><p>
 * Output buffer will contain a sequence of 'double' type
 * values in machine order representing data in the frequency domain.
 * The data is ordered as follows:<pre>
 *    Let 'n' be the total number of values.
 *    Let 'i' be an index into the buffer.
 *
 *    buf[0]   : the DC coefficient
 *    buf[i]   : the real component of the i'th frequency band
 *    buf[n-i] : the complex component of the i'th frequency band
 * </pre>
 *
 * Hence data for only n/2 of the n frequency bands is output.
 * This is because the latter bands are merely the complex conjugate
 * of the former.
 * </p><p>
 * Supports reset().
 * </p>
 */
class WvRealToComplexFFTEncoder :
    public WvTypedEncoder<double, double>
{
public:
    enum WindowFunction {
        WND_NONE,  /*!< No windowing */
        WND_BOXCAR /*!< After each FFT step, returns half of the sample
                        to the input buffer to be processed again in
                        the next step. */
    };

    /**
     * Creates a forward real-to-complex FFT encoder.
     *
     * @param n the number of values per block
     * @param wnd the window function
     */
    WvRealToComplexFFTEncoder(size_t n,
        WindowFunction wnd = WND_NONE);
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
 * Computes the inverse FFT transformation of complex valued input
 * to <em>unnormalized</em> real output.
 * <p>
 * Input buffer must contain a sequence of 'double' type
 * values in machine order representing data in the frequency domain.
 * The data must be organized in the same fashion as that output
 * by WvRealToComplexFFTEncoder.
 * </p><p>
 * Output buffer will contain a sequence of 'double' type
 * values in machine order representing samples in the time domain.
 * </p><p>
 * Supports reset().
 * </p>
 */
class WvComplexToRealFFTEncoder :
    public WvTypedEncoder<double, double>
{
    struct fftw_plan_struct *plan;
    size_t n;
    WvInPlaceBufferBase<double> tmpbuf;
    
public:
    /**
     * Creates an inverse complex-to-real FFT encoder encoder.
     *
     * @param n the number of values per block
     */
    WvComplexToRealFFTEncoder(size_t n);
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
 * <p>
 * Input buffer must contain a sequence of 'double' type
 * values in machine order representing data in the frequency domain.
 * The data must be organized in the same fashion as that output
 * by WvRealToComplexFFTEncoder.
 * </p><p>
 * Output buffer will contain a sequence of 'double' type
 * values in machine order representing the power spectrum.
 * Only the power coefficients for (n/2)+1 bands are output.
 * The data is ordered as follows:<pre>
 *   Let 'n' be the total number of values.
 *   Let 'i' be an index into the buffer.
 *
 *   buf[0] : the squared DC coefficient
 *   buf[i] : the squared power of the i'th band
 * </pre>
 * </p><p>
 * Supports reset().
 * </p>
 */
class WvPowerSpectrumEncoder :
    public WvTypedEncoder<double, double>
{
    size_t n, half, mid;

public:
    /**
     * Creates a power spectrum encoder.
     *
     * @param n the number of values per block
     */
    WvPowerSpectrumEncoder(size_t n);

protected:
    /**
     * If not flushing, only processes at most one block of data.
     */
    virtual bool _typedencode(IBuffer &inbuf, OBuffer &outbuf, bool flush);
    virtual bool _reset();
};

#endif // __WVFFT_H
