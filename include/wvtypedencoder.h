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
 * that manipulate typed buffers.  Such encoders accept both
 * typed and untyped buffers, but are implementated in terms
 * of typed buffers only where untyped buffers are automatically
 * wrapped into the required form.
 *
 * @param IType the input buffer datatype
 * @param OType the output buffer datatype
 * @see WvEncoder
 */
template<class IType, class OType>
class WvTypedEncoder : public WvEncoder
{
public:
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
        return WvEncoder::encode(inview, outview, flush, finish);
    }

    /**
     * Typed variant of flush().
     * @see flush(WvBuffer, WvBuffer, bool)
     */
    bool flush(IBuffer &inbuf, OBuffer &outbuf, bool finish = false)
    {
        WvBufferView inview(inbuf);
        WvBufferView outview(outbuf);
        return WvEncoder::flush(inview, outview, finish);
    }

    /**
     * Typed variant of finish().
     * @see finish(WvBuffer)
     */
    bool finish(OBuffer &outbuf)
    {
        WvBufferView outview(outbuf);
        return WvEncoder::finish(outview);
    }
    
    inline bool encode(WvBuffer &inbuf, WvBuffer &outbuf,
        bool flush = false, bool finish = false)
    {
        return WvEncoder::encode(inbuf, outbuf, flush, finish);
    }
    inline bool flush(WvBuffer &inbuf, WvBuffer &outbuf,
        bool finish = false)
    {
        return WvEncoder::flush(inbuf, outbuf, finish);
    }
    inline bool finish(WvBuffer &outbuf)
    {
        return WvEncoder::finish(outbuf);
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
     * Wrapper implementations of _encode().
     */
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf,
        bool flush)
    {
        IBufferView inview(inbuf);
        OBufferView outview(outbuf);
        return _typedencode(inview, outview, flush);
    }
    
    /**
     * Wrapper implementations of _finish().
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
 */
template<class IType>
class WvTypedEncoder<IType, unsigned char> : public WvEncoder
{
public:
    typedef WvBufferBase<IType> IBuffer;
    typedef WvBuffer OBuffer;
    typedef WvBufferViewBase<IType> IBufferView;
    typedef WvBufferView OBufferView;

    /**
     * Typed variant of encode().
     * @see encode(WvBuffer&, WvBuffer&, bool, bool)
     */
    bool encode(IBuffer &inbuf, OBuffer &outbuf, bool flush = false,
        bool finish = false)
    {
        WvBufferView inview(inbuf);
        return WvEncoder::encode(inview, outbuf, flush, finish);
    }

    /**
     * Typed variant of flush().
     * @see flush(WvBuffer, WvBuffer, bool)
     */
    bool flush(IBuffer &inbuf, OBuffer &outbuf, bool finish = false)
    {
        WvBufferView inview(inbuf);
        return WvEncoder::flush(inview, outbuf, finish);
    }
    
    inline bool encode(WvBuffer &inbuf, WvBuffer &outbuf,
        bool flush = false, bool finish = false)
    {
        return WvEncoder::encode(inbuf, outbuf, flush, finish);
    }
    inline bool flush(WvBuffer &inbuf, WvBuffer &outbuf,
        bool finish = false)
    {
        return WvEncoder::flush(inbuf, outbuf, finish);
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
     * Wrapper implementations of _encode().
     */
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf,
        bool flush)
    {
        IBufferView inview(inbuf);
        return _typedencode(inview, outbuf, flush);
    }
    
    /**
     * Wrapper implementations of _finish().
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
template<>
class WvTypedEncoder<unsigned char, unsigned char> : public WvEncoder
{
public:
    typedef WvBuffer IBuffer;
    typedef WvBuffer OBuffer;
    typedef WvBufferView IBufferView;
    typedef WvBufferView OBufferView;

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
     * Wrapper implementations of _encode().
     */
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf,
        bool flush)
    {
        return _typedencode(inbuf, outbuf, flush);
    }
    
    /**
     * Wrapper implementations of _finish().
     */
    virtual bool _finish(WvBuffer &outbuf)
    {
        return _typedfinish(outbuf);
    }
};

#endif // __WVTYPEDENCODER
