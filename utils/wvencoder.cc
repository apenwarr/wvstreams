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
    okay = true;
    finished = false;
}


WvEncoder::~WvEncoder()
{
}


WvString WvEncoder::geterror() const
{
    if (isok())
        return WvString::null;
    if (!!errstr)
        return errstr;
    WvString message = _geterror();
    if (!!message)
        return message;
    return "unknown encoder error";
}


bool WvEncoder::encode(WvBuf &inbuf, WvBuf &outbuf,
		       bool flush, bool _finish)
{
    // deliberately not using isok() and isfinished() here
    bool success = okay && !finished && (inbuf.used() != 0 || flush);
    if (success)
        success = _encode(inbuf, outbuf, flush);
    if (_finish)
        success = finish(outbuf) && success;
    return success;
}


bool WvEncoder::finish(WvBuf &outbuf)
{
    // deliberately not using isok() and isfinished() here
    bool success = okay && !finished;
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
    if (!success)
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
    last_run = NULL;
}


WvEncoderChain::~WvEncoderChain()
{
}


bool WvEncoderChain::_isok() const
{
    ChainElemList::Iter it(const_cast<ChainElemList&>(encoders));
    for (it.rewind(); it.next(); )
        if (!it->enc->isok())
            return false;
    return true;
}


bool WvEncoderChain::_isfinished() const
{
    ChainElemList::Iter it(const_cast<ChainElemList&>(encoders));
    for (it.rewind(); it.next(); )
        if (it->enc->isfinished())
            return true;
    return false;
}


WvString WvEncoderChain::_geterror() const
{
    ChainElemList::Iter it(const_cast<ChainElemList&>(encoders));
    for (it.rewind(); it.next(); )
    {
        WvString message = it->enc->geterror();
        if (!!message) return message;
    }
    return WvString::null;
}


// NOTE: In this function we deliberately ignore deep isok() and
//       isfinished() results to allow addition/removal of
//       individual broken encoders while still processing data
//       through as much of the chain as possible.
bool WvEncoderChain::do_encode(WvBuf &in, WvBuf &out, ChainElem *start_after,
			       bool flush, bool finish)
{
    bool success = true;
    WvBuf *tmpin = &in;
    ChainElemList::Iter it(encoders);
    it.rewind();
    if (start_after) it.find(start_after);
    last_run = start_after;
    for (; it.cur() && it.next(); )
    {
        if (!it->enc->encode(*tmpin, it->out, flush))
            success = false;
        if (finish && !it->enc->finish(it->out))
            success = false;
	last_run = it.ptr();
        tmpin = &it->out;
    }
    out.merge(*tmpin);
    return success;
}


bool WvEncoderChain::_encode(WvBuf &in, WvBuf &out, bool flush)
{
    return do_encode(in, out, NULL, flush, false);
}


bool WvEncoderChain::_finish(WvBuf &out)
{
    WvNullBuf empty;
    return do_encode(empty, out, NULL, true, true);
}


bool WvEncoderChain::continue_encode(WvBuf &in, WvBuf &out)
{
    //fprintf(stderr, "continue_encode(%d,%d,%p)\n",
    //	    in.used(), out.used(), last_run);
    return do_encode(in, out, last_run, false, false);
}


bool WvEncoderChain::_reset()
{
    bool success = true;
    ChainElemList::Iter it(encoders);
    for (it.rewind(); it.next(); )
    {
        it->out.zap();
        if (!it->enc->reset())
            success = false;
    }
    return success;
}


void WvEncoderChain::append(WvEncoder *enc, bool autofree)
{
    encoders.append(new ChainElem(enc, autofree), true);
}


void WvEncoderChain::prepend(WvEncoder *enc, bool autofree)
{
    encoders.prepend(new ChainElem(enc, autofree), true);
}


bool WvEncoderChain::get_autofree(WvEncoder *enc) const
{
    ChainElemList::Iter i(encoders);
    for (i.rewind(); i.next(); )
	if (i->enc == enc && i.get_autofree())
	    return true;
    return false;
}


void WvEncoderChain::set_autofree(WvEncoder *enc, bool autofree)
{
    ChainElemList::Iter i(encoders);
    if (autofree)
    {
	// Ensure only the first matching encoder has autofree set
	bool first = true;
	for (i.rewind(); i.next(); )
	{
	    if (i->enc == enc)
	    {
		if (first)
		{
		    i.set_autofree(true);
		    first = false;
		}
		else
		    i.set_autofree(false);
	    }
	}
    }
    else
    {
	// Clear autofree for all matching encoders
	for (i.rewind(); i.next(); )
	    if (i->enc == enc)
		i.set_autofree(false);
    }
}


void WvEncoderChain::unlink(WvEncoder *enc)
{
    ChainElemList::Iter it(encoders);
    for (it.rewind(); it.next(); )
        if (it->enc == enc)
            it.xunlink();
}


void WvEncoderChain::zap()
{
    encoders.zap();
}
