/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A top-level data encoder class.  See wvencoder.h.
 */
#include "wvencoder.h"

/***** WvEncoder *****/

WvEncoder::WvEncoder()
{
}


WvEncoder::~WvEncoder()
{
}


bool WvEncoder::isok() const
{
    return true;
}


bool WvEncoder::encode(WvStringParm instr, WvBuffer &outbuf)
{
    WvBuffer inbuf;
    inbuf.put(instr);
    bool success = encode(inbuf, outbuf, true);
    return success;
}


bool WvEncoder::encode(WvStringParm instr, WvString &outstr)
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


WvString WvEncoder::strencode(WvStringParm instr, bool ignore_errors)
{
    WvString outstr;
    bool success = encode(instr, outstr);
    return ignore_errors || success ?
        outstr : WvString(WvString::null);
}


WvString WvEncoder::strencode(WvBuffer &inbuf, bool ignore_errors)
{
    WvString outstr;
    bool success = encode(inbuf, outstr);
    return ignore_errors || success ?
        outstr : WvString(WvString::null);
}


/***** WvNullEncoder *****/

bool WvNullEncoder::encode(WvBuffer &in, WvBuffer &out, bool flush)
{
    in.zap();
    return true;
}


/***** WvPassthroughEncoder *****/

bool WvPassthroughEncoder::encode(WvBuffer &in, WvBuffer &out, bool flush)
{
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


bool WvEncoderChain::isok() const
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


bool WvEncoderChain::encode(WvBuffer &in, WvBuffer &out, bool flush)
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

        if (! encelem->enc->encode(*tmpin, *tmpout, flush))
            success = false;

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
