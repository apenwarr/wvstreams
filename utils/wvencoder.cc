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


bool WvEncoder::encode(WvBuf &inbuf, WvBuf &outbuf,
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


bool WvEncoder::finish(WvBuf &outbuf)
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


bool WvEncoder::flushstrbuf(WvStringParm instr, WvBuf &outbuf,
    bool finish)
{
    WvConstStringBuffer inbuf(instr);
    bool success = encode(inbuf, outbuf, true, finish);
    return success;
}


bool WvEncoder::flushstrstr(WvStringParm instr, WvString &outstr,
    bool finish)
{
    WvConstStringBuffer inbuf(instr);
    WvDynBuf outbuf;
    bool success = encode(inbuf, outbuf, true, finish);
    outstr.append(outbuf.getstr());
    return success;
}


bool WvEncoder::encodebufstr(WvBuf &inbuf, WvString &outstr,
    bool flush, bool finish)
{
    WvDynBuf outbuf;
    bool success = encode(inbuf, outbuf, flush, finish);
    outstr.append(outbuf.getstr());
    return success;
}


WvString WvEncoder::strflushstr(WvStringParm instr, bool finish)
{
    WvString outstr;
    flushstrstr(instr, outstr, finish);
    return outstr;
}


WvString WvEncoder::strflushbuf(WvBuf &inbuf, bool finish)
{
    WvString outstr;
    flushbufstr(inbuf, outstr, finish);
    return outstr;
}


bool WvEncoder::flushmembuf(const void *inmem, size_t inlen,
    WvBuf &outbuf, bool finish)
{
    WvConstInPlaceBuf inbuf(inmem, inlen);
    bool success = encode(inbuf, outbuf, true, finish);
    return success;
}


bool WvEncoder::flushmemmem(const void *inmem, size_t inlen,
    void *outmem, size_t *outlen, bool finish)
{
    WvConstInPlaceBuf inbuf(inmem, inlen);
    return encodebufmem(inbuf, outmem, outlen, true, finish);
}


bool WvEncoder::encodebufmem(WvBuf &inbuf,
    void *outmem, size_t *outlen, bool flush, bool finish)
{
    WvInPlaceBuf outbuf(outmem, 0, *outlen);
    bool success = encode(inbuf, outbuf, true, finish);
    *outlen = outbuf.used();
    return success;
}


bool WvEncoder::flushstrmem(WvStringParm instr,
    void *outmem, size_t *outlen, bool finish)
{
    WvConstStringBuffer inbuf(instr);
    return flushbufmem(inbuf, outmem, outlen, finish);
}


WvString WvEncoder::strflushmem(const void *inmem, size_t inlen, bool finish)
{
    WvConstInPlaceBuf inbuf(inmem, inlen);
    return strflushbuf(inbuf, finish);
}


/***** WvNullEncoder *****/

bool WvNullEncoder::_encode(WvBuf &in, WvBuf &out, bool flush)
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


bool WvPassthroughEncoder::_encode(WvBuf &in, WvBuf &out, bool flush)
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
bool WvEncoderChain::_encode(WvBuf &in, WvBuf &out, bool flush)
{
    if (encoders.isempty())
        return passthrough.encode(in, out, flush);

    // iterate over all encoders in the list
    bool success = true;
    WvEncoderChainElemListBase::Iter it(encoders);
    it.rewind();
    it.next();
    for (WvBuf *tmpin = & in;;)
    {
        // merge pending output and select an output buffer
        WvEncoderChainElem *encelem = it.ptr();
        bool hasnext = it.next();
        WvBuf *tmpout;
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
bool WvEncoderChain::_finish(WvBuf &out)
{
    if (encoders.isempty())
        return true;
    
    // iterate over all encoders in the list
    bool success = true;
    WvEncoderChainElemListBase::Iter it(encoders);
    it.rewind();
    it.next();
    bool needs_flush = false;
    for (WvBuf *tmpin = NULL;;)
    {
        // merge pending output and select an output buffer
        WvEncoderChainElem *encelem = it.ptr();
        bool hasnext = it.next();
        WvBuf *tmpout;
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


void WvEncoderChain::append(WvEncoder *enc, bool autofree)
{
    encoders.append(new WvEncoderChainElem(enc, autofree), true);
}


void WvEncoderChain::prepend(WvEncoder *enc, bool autofree)
{
    encoders.prepend(new WvEncoderChainElem(enc, autofree), true);
}

bool WvEncoderChain::get_autofree(WvEncoder *enc)
{
    WvEncoderChainElemListBase::Iter i(encoders);
    for (i.rewind(); i.next(); )
    {
	if ((i.ptr()->enc == enc) && (i.cur()->auto_free))
	    return true;
    }
    return false;
}

void WvEncoderChain::set_autofree(WvEncoder *enc, bool autofree)
{
    WvEncoderChainElemListBase::Iter i(encoders);
    if (autofree)
    {
	// Ensure only the first encoder has autofree set
	bool first = true;
	for (i.rewind(); i.next(); )
	{
	    if (i.ptr()->enc == enc)
	    {
		if (first)
		{
		    i.cur()->auto_free = true;
		    first = false;
		}
		else
		    i.cur()->auto_free = false;
	    }
	}
    }
    else
    {
	// Clear autofree for all encoders
	for (i.rewind(); i.next(); )
	    if (i.ptr()->enc == enc)
		i.cur()->auto_free = false;
    }
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
