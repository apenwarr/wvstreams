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
    return "unknown error";
}


bool WvEncoder::encode(WvBuffer &inbuf, WvBuffer &outbuf, bool flush)
{
    // deliberately not using isok() and isfinished() here
    bool success = okay && ! finished && (inbuf.used() != 0 || flush);
    if (success)
        success = _encode(inbuf, outbuf, flush);
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


bool WvEncoder::flush(WvStringParm instr, WvBuffer &outbuf)
{
    WvBuffer inbuf;
    inbuf.put(instr);
    bool success = encode(inbuf, outbuf, true);
    return success;
}


bool WvEncoder::flush(WvStringParm instr, WvString &outstr)
{
    WvBuffer inbuf, outbuf;
    inbuf.put(instr);
    bool success = encode(inbuf, outbuf, true);
    outstr.append(outbuf.getstr());
    return success;
}


bool WvEncoder::encode(WvBuffer &inbuf, WvString &outstr, bool flush)
{
    WvBuffer outbuf;
    bool success = encode(inbuf, outbuf, flush);
    outstr.append(outbuf.getstr());
    return success;
}


WvString WvEncoder::strflush(WvStringParm instr, bool ignore_errors)
{
    WvString outstr;
    bool success = flush(instr, outstr);
    return ignore_errors || success ?
        outstr : WvString(WvString::null);
}


WvString WvEncoder::strflush(WvBuffer &inbuf, bool ignore_errors)
{
    WvString outstr;
    bool success = flush(inbuf, outstr);
    return ignore_errors || success ?
        outstr : WvString(WvString::null);
}


bool WvEncoder::flush(const void *inmem, size_t inlen, WvBuffer &outbuf)
{
    // FIXME: optimize using in-place buffers someday
    WvBuffer inbuf;
    inbuf.put(inmem, inlen);
    bool success = encode(inbuf, outbuf, true);
    return success;
}


bool WvEncoder::flush(const void *inmem, size_t inlen, void *outmem,
    size_t *outlen)
{
    // FIXME: optimize using in-place buffers someday
    WvBuffer inbuf;
    inbuf.put(inmem, inlen);
    return encode(inbuf, outmem, outlen, true);
}


bool WvEncoder::encode(WvBuffer &inbuf, void *outmem, size_t *outlen,
    bool flush)
{
    // FIXME: optimize using in-place buffers someday
    WvBuffer outbuf;
    bool success = encode(inbuf, outbuf, true);
    size_t used = outbuf.used();
    if (used > *outlen)
    {
        // uhoh, overflow detected!
        used = *outlen;
        success = false;
    }
    else
        *outlen = used;
    memcpy(outmem, outbuf.get(used), used);
    return success;
}


bool WvEncoder::flush(WvStringParm instr, void *outmem, size_t *outlen)
{
    WvBuffer inbuf;
    inbuf.put(instr);
    return flush(inbuf, outmem, outlen);
}



/***** WvNullEncoder *****/

bool WvNullEncoder::_encode(WvBuffer &in, WvBuffer &out, bool flush)
{
    in.zap();
    return true;
}



/***** WvPassthroughEncoder *****/

bool WvPassthroughEncoder::_encode(WvBuffer &in, WvBuffer &out, bool flush)
{
    total += in.used();
    out.merge(in);
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
