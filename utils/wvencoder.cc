/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A top-level data encoder class.  See wvencoder.h.
 */
#include "wvencoder.h"

/***** WvEncoder *****/

WvEncoder::WvEncoder() : okay(true), finished(false)
{
}


WvEncoder::~WvEncoder()
{
}


WvString WvEncoder::geterror() const
{
    if (isok())
        return WvString::null;
    if (!! errstr)
        return errstr;
    WvString message = _geterror();
    if (!! message)
        return message;
    return "unknown encoder error";
}


bool WvEncoder::encode(WvBuffer &inbuf, WvBuffer &outbuf,
    bool flush, bool _finish)
{
    // deliberately not using isok() and isfinished() here
    bool success = okay && ! finished && (inbuf.used() != 0 || flush);
    if (success)
        success = _encode(inbuf, outbuf, flush);
    if (_finish)
        success = finish(outbuf) && success;
    return success;
}


bool WvEncoder::finish(WvBuffer &outbuf)
{
    // deliberately not using isok() and isfinished() here
    bool success = okay && ! finished;
    if (success)
        success = _finish(outbuf);
    setfinished();
    return success;
}


bool WvEncoder::reset()
{
    // reset local state
    okay = true;
    finished = false;
    errstr = WvString::null;
    // attempt to reset the encoder
    bool success = _reset();
    if (! success)
    {
        if (okay)
            seterror("reset not supported by encoder");
    }
    return success;
}


bool WvEncoder::flush(WvStringParm instr, WvBuffer &outbuf, bool finish)
{
    WvConstStringBuffer inbuf(instr);
    bool success = encode(inbuf, outbuf, true, finish);
    return success;
}


bool WvEncoder::flush(WvStringParm instr, WvString &outstr, bool finish)
{
    WvConstStringBuffer inbuf(instr);
    WvDynamicBuffer outbuf;
    bool success = encode(inbuf, outbuf, true, finish);
    outstr.append(outbuf.getstr());
    return success;
}


bool WvEncoder::encode(WvBuffer &inbuf, WvString &outstr,
    bool flush, bool finish)
{
    WvDynamicBuffer outbuf;
    bool success = encode(inbuf, outbuf, flush, finish);
    outstr.append(outbuf.getstr());
    return success;
}


WvString WvEncoder::strflush(WvStringParm instr, bool finish)
{
    WvString outstr;
    flush(instr, outstr, finish);
    return outstr;
}


WvString WvEncoder::strflush(WvBuffer &inbuf, bool finish)
{
    WvString outstr;
    flush(inbuf, outstr, finish);
    return outstr;
}


WvString WvEncoder::strflush(const void *inmem, size_t inlen, bool finish)
{
    WvConstInPlaceBuffer inbuf(inmem, inlen);
    return strflush(inbuf, finish);
}


bool WvEncoder::flush(const void *inmem, size_t inlen,
    WvBuffer &outbuf, bool finish)
{
    WvConstInPlaceBuffer inbuf(inmem, inlen);
    bool success = encode(inbuf, outbuf, true, finish);
    return success;
}


bool WvEncoder::flush(const void *inmem, size_t inlen,
    void *outmem, size_t *outlen, bool finish)
{
    WvConstInPlaceBuffer inbuf(inmem, inlen);
    return encode(inbuf, outmem, outlen, true, finish);
}


bool WvEncoder::encode(WvBuffer &inbuf, void *outmem, size_t *outlen,
    bool flush, bool finish)
{
    WvInPlaceBuffer outbuf(outmem, 0, *outlen);
    bool success = encode(inbuf, outbuf, true, finish);
    *outlen = outbuf.used();
    return success;
}


bool WvEncoder::flush(WvStringParm instr, void *outmem, size_t *outlen,
    bool finish)
{
    WvConstStringBuffer inbuf(instr);
    return flush(inbuf, outmem, outlen, finish);
}



/***** WvNullEncoder *****/

bool WvNullEncoder::_encode(WvBuffer &in, WvBuffer &out, bool flush)
{
    in.zap();
    return true;
}


bool WvNullEncoder::_reset()
{
    return true;
}



/***** WvPassthroughEncoder *****/

WvPassthroughEncoder::WvPassthroughEncoder()
{
    _reset();
}


bool WvPassthroughEncoder::_encode(WvBuffer &in, WvBuffer &out, bool flush)
{
    total += in.used();
    out.merge(in);
    return true;
}


bool WvPassthroughEncoder::_reset()
{
    total = 0;
    return true;
}



/***** WvEncoderChain *****/

WvEncoderChain::WvEncoderChain()
{
}


WvEncoderChain::~WvEncoderChain()
{
}


bool WvEncoderChain::_isok() const
{
    WvEncoderChainElemListBase::Iter it(
        const_cast<WvEncoderChainElemListBase&>(encoders));
    for (it.rewind(); it.next(); )
    {
        WvEncoderChainElem *encelem = it.ptr();
        if (! encelem->enc->isok())
            return false;
    }
    return true;
}


bool WvEncoderChain::_isfinished() const
{
    WvEncoderChainElemListBase::Iter it(
        const_cast<WvEncoderChainElemListBase&>(encoders));
    for (it.rewind(); it.next(); )
    {
        WvEncoderChainElem *encelem = it.ptr();
        if (encelem->enc->isfinished())
            return true;
    }
    return false;
}


WvString WvEncoderChain::_geterror() const
{
    WvEncoderChainElemListBase::Iter it(
        const_cast<WvEncoderChainElemListBase&>(encoders));
    for (it.rewind(); it.next(); )
    {
        WvEncoderChainElem *encelem = it.ptr();
        WvString message = encelem->enc->geterror();
        if (!! message)
            return message;
    }
    return WvString::null;
}


// NOTE: In this function we deliberately ignore deep isok() and
//       isfinished() results to allow addition/removal of
//       individual broken encoders while still processing data
//       through as much of the chain as possible.
bool WvEncoderChain::_encode(WvBuffer &in, WvBuffer &out, bool flush)
{
    if (encoders.isempty())
        return passthrough.encode(in, out, flush);

    // iterate over all encoders in the list
    bool success = true;
    WvEncoderChainElemListBase::Iter it(encoders);
    it.rewind();
    it.next();
    for (WvBuffer *tmpin = & in;;)
    {
        // merge pending output and select an output buffer
        WvEncoderChainElem *encelem = it.ptr();
        bool hasnext = it.next();
        WvBuffer *tmpout;
        if (! hasnext)
        {
            out.merge(encelem->out);
            tmpout = & out;
        }
        else
            tmpout = & encelem->out;

        // encode
        if (! encelem->enc->encode(*tmpin, *tmpout, flush))
            success = false;

        if (! hasnext)
            break;
        tmpin = & encelem->out;
    }
    return success;
}


// NOTE: In this function we deliberately ignore deep isok() and
//       isfinished() results to allow addition/removal of
//       individual broken encoders while still processing data
//       through as much of the chain as possible.
bool WvEncoderChain::_finish(WvBuffer &out)
{
    if (encoders.isempty())
        return true;
    
    // iterate over all encoders in the list
    bool success = true;
    WvEncoderChainElemListBase::Iter it(encoders);
    it.rewind();
    it.next();
    bool needs_flush = false;
    for (WvBuffer *tmpin = NULL;;)
    {
        // merge pending output and select an output buffer
        WvEncoderChainElem *encelem = it.ptr();
        bool hasnext = it.next();
        WvBuffer *tmpout;
        if (! hasnext)
        {
            out.merge(encelem->out);
            tmpout = & out;
        }
        else
            tmpout = & encelem->out;

        // do we need to flush first due to new input?
        size_t oldused = tmpout->used();
        if (needs_flush)
        {
            if (! encelem->enc->flush(*tmpin, *tmpout))
                success = false;
            needs_flush = true;
        }
        
        // tell the encoder to finish
        if (! encelem->enc->finish(*tmpout))
            success = false;
            
        // check whether any new data was generated
        if (oldused != tmpout->used())
            needs_flush = true;

        if (! hasnext)
            break;
        tmpin = & encelem->out;
    }
    return success;
}


bool WvEncoderChain::_reset()
{
    bool success = true;
    WvEncoderChainElemListBase::Iter it(encoders);
    for (it.rewind(); it.next(); )
    {
        WvEncoderChainElem *encelem = it.ptr();
        encelem->out.zap();
        if (! encelem->enc->reset())
            success = false;
    }
    return success;
}


void WvEncoderChain::append(WvEncoder *enc, bool auto_free)
{
    encoders.append(new WvEncoderChainElem(enc, auto_free), true);
}


void WvEncoderChain::prepend(WvEncoder *enc, bool auto_free)
{
    encoders.prepend(new WvEncoderChainElem(enc, auto_free), true);
}


void WvEncoderChain::unlink(WvEncoder *enc)
{
    WvEncoderChainElemListBase::Iter it(encoders);
    for (it.rewind(); it.next(); )
    {
        WvEncoderChainElem *encelem = it.ptr();
        if (encelem->enc == enc)
            it.xunlink();
    }
}

void WvEncoderChain::zap()
{
    encoders.zap();
}
