/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvEncoderStream chains a series of encoders on the input and
 * output ports of the underlying stream to effect on-the-fly data
 * transformations.
 */
#include "wvencoderstream.h"

WvEncoderStream::WvEncoderStream(WvStream *_cloned) : WvStreamClone(_cloned)
{
    is_closing = false;
    is_eof = false;
    min_readsize = 0;
}


WvEncoderStream::~WvEncoderStream()
{
    close();
}


void WvEncoderStream::close()
{
    // we want to finish the encoders even if !isok() since we
    // might just have encountered an EOF condition, and we want
    // to ensure that the remaining data is processed, but this
    // might cause recursion if the encoders set a new error condition
    if (is_closing) return;
    is_closing = true;
    
    // finish encoders
    finish_read();
    finish_write();
    // flush write chain and close the stream
    WvStreamClone::close();
}


bool WvEncoderStream::isok() const
{
    // handle encoder error conditions
    if (! WvStream::isok())
        return false;

    // handle substream error conditions
    // we don't check substream isok() because that is handled
    // during read operations to distinguish EOF from errors
    if (! cloned || cloned->geterr() != 0)
        return false;
        
    // handle deferred EOF condition
    return ! is_eof;
}


bool WvEncoderStream::flush_internal(time_t msec_timeout)
{
    flush_write();

    // flush underlying stream
    while (isok() && writeoutbuf.used())
    {
        WvEncoderStream::flush(msec_timeout);
        if (!msec_timeout || !select(msec_timeout, false, true))
        {
            if (msec_timeout >= 0)
                break;
        }
    }
    
    return !writeoutbuf.used();
}


bool WvEncoderStream::flush_read()
{
    bool success = readchain.flush(readinbuf, readoutbuf);
    checkreadisok();
    inbuf.merge(readoutbuf);
    return success;
}


bool WvEncoderStream::flush_write()
{
    bool success = push(true /*flush*/, false /*finish*/);
    return success;
}


bool WvEncoderStream::finish_read()
{
    bool success = readchain.flush(readinbuf, readoutbuf);
    if (! readchain.finish(readoutbuf))
        success = false;
    checkreadisok();
    inbuf.merge(readoutbuf);
    is_eof = true;
    return success;
}


bool WvEncoderStream::finish_write()
{
    return push(true /*flush*/, true /*finish*/);
}


void WvEncoderStream::pull(size_t size)
{
    if (is_eof)
        return;

    // pull a chunk of unencoded input
    bool finish = false;
    if (! readchain.isfinished() && cloned)
    {
        if (size != 0)
            cloned->read(readinbuf, size);
        if (! cloned->isok())
            finish = true; // underlying stream hit EOF or error
    }

    // encode the input
    readchain.encode(readinbuf, readoutbuf, finish /* flush*/);
    if (finish)
    {
        readchain.finish(readoutbuf);
        if (readoutbuf.used() == 0)
            is_eof = true;
        // otherwise defer EOF until the buffered data has been read
    }
    else if (readoutbuf.used() == 0 && readchain.isfinished())
    {
        // only get EOF when the chain is finished and we have no
        // more data
        is_eof = true;
    }
    checkreadisok();
}


bool WvEncoderStream::push(bool flush, bool finish)
{
    // encode the output
    if (flush)
        writeinbuf.merge(outbuf);
    bool success = writechain.encode(writeinbuf, writeoutbuf, flush);
    if (finish)
        if (! writechain.finish(writeoutbuf))
            success = false;
    checkwriteisok();

    // push encoded output to cloned stream
    size_t size = writeoutbuf.used();
    if (size != 0)
    {
        const unsigned char *writeout = writeoutbuf.get(size);
        size_t len = WvStreamClone::uwrite(writeout, size);
        writeoutbuf.unget(size - len);
    }
    return success;
}


size_t WvEncoderStream::uread(void *buf, size_t size)
{
    pull(min_readsize > size ? min_readsize : size);
    size_t avail = readoutbuf.used();
    if (size > avail)
        size = avail;
    readoutbuf.move(buf, size);
    return size;
}


size_t WvEncoderStream::uwrite(const void *buf, size_t size)
{
    writeinbuf.put(buf, size);
    push(false /*flush*/, false /*finish*/);
    return size;
}


bool WvEncoderStream::pre_select(SelectInfo &si)
{
    bool surething = false;

    // if we have buffered input data and we want to check for
    // readability, then cause a callback to occur that will
    // hopefully ask us for more data via uread()
    if (si.wants.readable)
    {
        pull(0); // try an encode
        if (readoutbuf.used() != 0)
            surething = true;
    }

    // try to push pending encoded output to cloned stream
    // outbuf_delayed_flush condition already handled by uwrite()
    push(false /*flush*/, false /*finish*/);

    // consult the underlying stream
    if (WvStreamClone::pre_select(si))
        surething = true;
    return surething;
}


void WvEncoderStream::checkreadisok()
{
    if (! readchain.isok())
    {
        seterr(WvString("read chain: %s", readchain.geterror()));
        is_eof = true;
    }
}


void WvEncoderStream::checkwriteisok()
{
    if (! writechain.isok())
        seterr(WvString("write chain: %s", writechain.geterror()));
}
