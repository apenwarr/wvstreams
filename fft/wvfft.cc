/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * An FFT abstraction.
 */
#include "wvfft.h"
#include <rfftw.h>

/***** WvRealToComplexFFTEncoder *****/

WvRealToComplexFFTEncoder::WvRealToComplexFFTEncoder(size_t _n) :
    n(_n)
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
    while ((len = inbuf.usedopt()) != 0)
    {
        len /= sizeof(double);
        if (len < n)
        {
            len = inbuf.used();
            len /= sizeof(double);
            if (len < n)
                return false;
        }
        size_t howmany = len / n;
        size_t total = howmany * n * sizeof(double);
        double *dataout = reinterpret_cast<double*>(
            outbuf.alloc(total));
        double *datain = const_cast<double*>(
            reinterpret_cast<const double*>(inbuf.get(total)));
        rfftw(plan, howmany, datain, 1, 1, dataout, 1, 1);
    }
    return true;
}


bool WvRealToComplexFFTEncoder::_reset()
{
    return true;
}


/***** WvComplexToRealFFTEncoder *****/

WvComplexToRealFFTEncoder::WvComplexToRealFFTEncoder(size_t _n) :
    n(_n), tmpbuf(n * sizeof(double))
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
        len /= sizeof(double);
        if (len < n)
            return false;
        size_t total = n * sizeof(double);
        double *dataout = reinterpret_cast<double*>(
            outbuf.alloc(total));
        tmpbuf.zap();
        tmpbuf.merge(inbuf, total);
        double *datain = reinterpret_cast<double*>(tmpbuf.ptr());
        rfftw_one(plan, datain, dataout);
    }
    return true;
}


bool WvComplexToRealFFTEncoder::_reset()
{
    return true;
}
