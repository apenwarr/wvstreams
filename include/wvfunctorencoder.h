/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Provides an encoder for applying a functor to data extracted
 * from a buffer and stored in another.
 * Assumes binary input is in machine order.
 */
#ifndef __WVFUNCTORENCODER_H
#define __WVFUNCTORENCODER_H

/**
 * OType specifies the output data type.
 * IType specifies the input data type.
 * Functor specifies the functor type which must have an operator()
 * with a signature compatible with invocations of the form:
 *   IType data = ...;
 *   OType result = func(data);
 *
 * The best way to use this monster is to subclass with friendly
 * names for the implementations that are needed.  For maximum
 * performance, define the functor as a struct that provides an
 * operator() inline.
 */
template<class OType, class IType, class Functor>
class WvFunctorEncoder : public WvEncoder
{
protected:
    typedef OType OT;
    typedef IType IT;
    typedef Functor FT;
    Functor f;
public:
    WvFunctorEncoder(const Functor &f) : f(f) { }
    virtual ~WvFunctorEncoder() { }

    virtual bool encode(WvBuffer &inbuf, WvBuffer &outbuf, bool flush)
    {
        // this will be nice and fast as most of the things
        // the loop uses will be inlined!
        while (inbuf.used() >= sizeof(IType))
        {
            unsigned char *indataraw = inbuf.get(sizeof(IType));
            IType indata(*reinterpret_cast<IType*>(indataraw));
            OType outdata(f(indata));
            outbuf.put(& outdata, sizeof(OType));
        }
        if (flush && inbuf.used() != 0)
            return false; // insufficient data to flush
        return true;
    }
};

#endif // __WVFUNCTORENCODER_H
