/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * An abstraction for encoders that manipulate typed buffers.
 */
#ifndef __WVTYPEDENCODER_H
#define __WVTYPEDENCODER_H

#include "wvbufferbase.h"
#include "wvencoder.h"

/**
 * This template facilitates the creation and use of encoders
 * that manipulate typed buffers.
 * 
 * A typed encoder accepts both typed and untyped buffers, but
 * is implementated in terms of typed buffers.  Untyped buffers
 * are automatically wrapped into the required form before being
 * passed on to the implementation.
 * 
 * This type is designed to function as a statically bound mixin
 * to make it easier to incorporate typed encoders into untyped
 * encoder hierarchies.  This is somewhat ugly, but necessary.
 * 
 *
 * @param IT the input buffer datatype
 * @param OT the output buffer datatype
 * @param S the WvEncoder supertype
 * @see WvEncoder
 */
template<class IT, class OT, class S = WvEncoder>
class WvTypedEncoder : public S
{
public:
    typedef IT IType;
    typedef OT OType;
    typedef WvBufferBase<IType> IBuffer;
    typedef WvBufferBase<OType> OBuffer;
    typedef WvBufferViewBase<IType> IBufferView;
    typedef WvBufferViewBase<OType> OBufferView;

    /**
     * Typed variant of encode().
     * @see encode(WvBuffer&, WvBuffer&, bool, bool)
     */
    bool encode(IBuffer &inbuf, OBuffer &outbuf, bool flush = false,
        bool finish = false)
    {
        WvBufferView inview(inbuf);
        WvBufferView outview(outbuf);
        return S::encode(inview, outview, flush, finish);
    }

    /**
     * Typed variant of flush().
     * @see flush(WvBuffer, WvBuffer, bool)
     */
    bool flush(IBuffer &inbuf, OBuffer &outbuf, bool finish = false)
    {
        WvBufferView inview(inbuf);
        WvBufferView outview(outbuf);
        return S::flush(inview, outview, finish);
    }

    /**
     * Typed variant of finish().
     * @see finish(WvBuffer)
     */
    bool finish(OBuffer &outbuf)
    {
        WvBufferView outview(outbuf);
        return S::finish(outview);
    }
    
    bool encode(WvBuffer &inbuf, WvBuffer &outbuf,
        bool flush = false, bool finish = false)
    {
        return S::encode(inbuf, outbuf, flush, finish);
    }
    bool flush(WvBuffer &inbuf, WvBuffer &outbuf,
        bool finish = false)
    {
        return S::flush(inbuf, outbuf, finish);
    }
    bool finish(WvBuffer &outbuf)
    {
        return S::finish(outbuf);
    }

protected:
    /**
     * Typed variant of _encode().
     * @see _encode(WvBuffer&, WvBuffer&, bool)
     */
    virtual bool _typedencode(IBuffer &inbuf, OBuffer &outbuf,
        bool flush) = 0;
    
    /**
     * Typed variant of _finish().
     * @see _finish(WvBuffer&)
     */
    virtual bool _typedfinish(OBuffer &outbuf)
        { return true; }

    /**
     * Wrapper implementation of _encode().
     */
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf,
        bool flush)
    {
        IBufferView inview(inbuf);
        OBufferView outview(outbuf);
        return _typedencode(inview, outview, flush);
    }
    
    /**
     * Wrapper implementation of _finish().
     */
    virtual bool _finish(WvBuffer &outbuf)
    {
        OBufferView outview(outbuf);
        return _typedfinish(outview);
    }
};

/**
 * Partial template specialization for unsigned char output
 * buffer type to avoid compilation errors.
 *
 * @param IType the input buffer datatype
 */
template<class IT, class S>
class WvTypedEncoder<IT, unsigned char, S> : public S
{
public:
    typedef IT IType;
    typedef unsigned char OType;
    typedef WvBufferBase<IType> IBuffer;
    typedef WvBufferBase<OType> OBuffer;
    typedef WvBufferViewBase<IType> IBufferView;
    typedef WvBufferViewBase<OType> OBufferView;

    /**
     * Typed variant of encode().
     * @see encode(WvBuffer&, WvBuffer&, bool, bool)
     */
    bool encode(IBuffer &inbuf, OBuffer &outbuf, bool flush = false,
        bool finish = false)
    {
        WvBufferView inview(inbuf);
        return S::encode(inview, outbuf, flush, finish);
    }

    /**
     * Typed variant of flush().
     * @see flush(WvBuffer, WvBuffer, bool)
     */
    bool flush(IBuffer &inbuf, OBuffer &outbuf, bool finish = false)
    {
        WvBufferView inview(inbuf);
        return S::flush(inview, outbuf, finish);
    }
    
    bool encode(WvBuffer &inbuf, WvBuffer &outbuf,
        bool flush = false, bool finish = false)
    {
        return S::encode(inbuf, outbuf, flush, finish);
    }
    bool flush(WvBuffer &inbuf, WvBuffer &outbuf,
        bool finish = false)
    {
        return S::flush(inbuf, outbuf, finish);
    }

protected:
    /**
     * Typed variant of _encode().
     * @see _encode(WvBuffer&, WvBuffer&, bool)
     */
    virtual bool _typedencode(IBuffer &inbuf, OBuffer &outbuf,
        bool flush) = 0;
    
    /**
     * Typed variant of _finish().
     * @see _finish(WvBuffer&)
     */
    virtual bool _typedfinish(OBuffer &outbuf)
        { return true; }
    
    /**
     * Wrapper implementation of _encode().
     */
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf,
        bool flush)
    {
        IBufferView inview(inbuf);
        return _typedencode(inview, outbuf, flush);
    }
    
    /**
     * Wrapper implementation of _finish().
     */
    virtual bool _finish(WvBuffer &outbuf)
    {
        return _typedfinish(outbuf);
    }
};


/**
 * Partial template specialization for unsigned char input
 * and output buffer types to avoid compilation errors.
 */
template<class S>
class WvTypedEncoder<unsigned char, unsigned char, S> : public S
{
public:
    typedef unsigned char IType;
    typedef unsigned char OType;
    typedef WvBufferBase<IType> IBuffer;
    typedef WvBufferBase<OType> OBuffer;
    typedef WvBufferViewBase<IType> IBufferView;
    typedef WvBufferViewBase<OType> OBufferView;

protected:
    /**
     * Typed variant of _encode().
     * @see _encode(WvBuffer&, WvBuffer&, bool)
     */
    virtual bool _typedencode(IBuffer &inbuf, OBuffer &outbuf,
        bool flush) = 0;
    
    /**
     * Typed variant of _finish().
     * @see _finish(WvBuffer&)
     */
    virtual bool _typedfinish(OBuffer &outbuf)
        { return true; }

    /**
     * Wrapper implementation of _encode().
     */
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf,
        bool flush)
    {
        return _typedencode(inbuf, outbuf, flush);
    }
    
    /**
     * Wrapper implementation of _finish().
     */
    virtual bool _finish(WvBuffer &outbuf)
    {
        return _typedfinish(outbuf);
    }
};

#endif // __WVTYPEDENCODER
