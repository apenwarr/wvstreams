/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * An FFT abstraction.
 */
#include "wvfft.h"
#include <rfftw.h>

/***** WvRealToComplexFFTEncoder *****/

WvRealToComplexFFTEncoder::WvRealToComplexFFTEncoder(size_t _n,
    WvRealToComplexFFTEncoder::WindowFunction _wnd) :
    n(_n), nbytes(n * sizeof(double)), wnd(_wnd)
{
    plan = rfftw_create_plan(n, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);
}


WvRealToComplexFFTEncoder::~WvRealToComplexFFTEncoder()
{
    rfftw_destroy_plan(plan);
}


bool WvRealToComplexFFTEncoder::_encode(WvBuffer &inbuf,
    WvBuffer &outbuf, bool flush)
{
    size_t len;
    while ((len = inbuf.optgettable()) != 0)
    {
        if (len < nbytes)
        {
            len = inbuf.used();
            if (len < nbytes)
                return ! flush;
        }
        size_t avail = outbuf.free();
        if (len > avail)
            len = avail;
        size_t howmany = len / nbytes;
        if (howmany == 0)
            return ! flush; // not enough space
        if (wnd != WND_NONE || ! flush)
            howmany = 1;
        
        size_t total = howmany * nbytes;
        double *dataout = reinterpret_cast<double*>(
            outbuf.alloc(total));
        double *datain = const_cast<double*>(
            reinterpret_cast<const double*>(inbuf.get(total)));
        rfftw(plan, howmany, datain, 1, 1, dataout, 1, 1);

        if (wnd == WND_BOXCAR)
            inbuf.unget((total + 1) / 2);

        // if not flushing, process at most one block at a time
        if (! flush) break;
    }
    return true;
}


bool WvRealToComplexFFTEncoder::_reset()
{
    return true;
}


/***** WvComplexToRealFFTEncoder *****/

WvComplexToRealFFTEncoder::WvComplexToRealFFTEncoder(size_t _n) :
    n(_n), nbytes(n * sizeof(double)),
    tmpbuf(nbytes)
{
    plan = rfftw_create_plan(_n, FFTW_COMPLEX_TO_REAL, FFTW_ESTIMATE);
}


WvComplexToRealFFTEncoder::~WvComplexToRealFFTEncoder()
{
    rfftw_destroy_plan(plan);
}


bool WvComplexToRealFFTEncoder::_encode(WvBuffer &inbuf,
    WvBuffer &outbuf, bool flush)
{
    size_t len;
    while ((len = inbuf.used()) != 0)
    {
        if (len < nbytes)
            return ! flush;
        if (outbuf.free() < nbytes)
            return ! flush;
        double *dataout = reinterpret_cast<double*>(
            outbuf.alloc(nbytes));
        tmpbuf.zap();
        tmpbuf.merge(inbuf, nbytes);
        double *datain = reinterpret_cast<double*>(tmpbuf.ptr());
        rfftw_one(plan, datain, dataout);

        // if not flushing, process at most one block at a time
        if (! flush) break;
    }
    return true;
}


bool WvComplexToRealFFTEncoder::_reset()
{
    return true;
}


/***** WvPowerSpectrumEncoder *****/

WvPowerSpectrumEncoder::WvPowerSpectrumEncoder(size_t _n) :
    n(_n), half(_n / 2 + 1), mid((_n + 1) / 2),
    nbytes(n * sizeof(double)), halfbytes(half * sizeof(double))
{
}


bool WvPowerSpectrumEncoder::_encode(WvBuffer &inbuf, WvBuffer &outbuf,
    bool flush)
{
    size_t len;
    while ((len = inbuf.used()) != 0)
    {
        if (len < nbytes)
            return ! flush;
        if (outbuf.free() < halfbytes)
            return ! flush;
        const double *datain = reinterpret_cast<const double*>(
            inbuf.get(nbytes));
        double *dataout = reinterpret_cast<double*>(
            outbuf.alloc(halfbytes));
        // compute power spectrum
        dataout[0] = datain[0] * datain[0];
        for (size_t i = 1; i < mid; ++i)
        {
            dataout[i] = datain[i] * datain[i] +
                datain[n - i] * datain[n - 1];
        }
        if ((n & 1) == 0)
            dataout[half - 1] = datain[half - 1] * datain[half - 1];

        // if not flushing, process at most one block at a time
        if (! flush) break;   
    }
    return true;
}


bool WvPowerSpectrumEncoder::_reset()
{
    return true;
}
