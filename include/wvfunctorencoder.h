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
 *   const IType data = ...;
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

protected:
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf, bool flush)
    {
        size_t count = inbuf.used() / sizeof(IType);
        if (count != 0)
        {
            const unsigned char *indataraw =
                inbuf.get(count * sizeof(IType));
            unsigned char *outdataraw = outbuf.alloc(count * sizeof(OType));
            for (;;)
            {
                // FIXME: possible unaligned data problems on some CPUs
                const IType *indata =
                    reinterpret_cast<const IType*>(indataraw);
                OType *outdata = reinterpret_cast<OType*>(outdataraw);
                *outdata = f(*indata);
                if (--count == 0) break;
                indataraw += sizeof(IType);
                outdataraw += sizeof(OType);
            }
        }
        if (flush && inbuf.used() != 0)
            return false; // insufficient data to flush
        return true;
    }
    virtual bool _reset()
    {
        // Assume most functor encoders will be stateless and therefore
        // support reset() implicitly.
        // If this is not the case, then override this method for
        // particular subclasses to return false.
        return true;
    }
};

#endif // __WVFUNCTORENCODER_H
