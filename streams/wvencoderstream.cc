/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvEncoderStream chains a series of encoders on the input and
 * output ports of the underlying stream to effect on-the-fly data
 * transformations.
 */
#include "wvencoderstream.h"

WvEncoderStream::WvEncoderStream(WvStream *_cloned) :
    WvStreamClone(_cloned), is_closing(false), min_readsize(0)
{
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
    if (! cloned || cloned->geterr() != 0)
        return false;
        
    // handle deferred EOF condition
    // if (! cloned->isok()) the readchain will be finished at
    //   the next uread() or pre_select().
    if (readchain.isfinished())
        return false;
    return true;
}


void WvEncoderStream::flush_internal(time_t msec_timeout)
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
    return success;
}


bool WvEncoderStream::finish_write()
{
    return push(true /*flush*/, true /*finish*/);
}


void WvEncoderStream::pull(size_t size)
{
    // pull a chunk of unencoded input
    bool finish = false;
    if (! readchain.isfinished() && cloned)
    {
        if (size != 0)
        {
            unsigned char *readin = readinbuf.alloc(size);
            size_t len = cloned->read(readin, size);
            readinbuf.unalloc(size - len);
        }
        if (! cloned->isok())
            finish = true;
    }

    // encode the input
    readchain.encode(readinbuf, readoutbuf, finish);
    if (finish)
        readchain.finish(readoutbuf);
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
    if (size > readoutbuf.used())
        size = readoutbuf.used();
    memcpy(buf, readoutbuf.get(size), size);
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
    // detect EOF and cause read chain to be finished if needed
    pull(0);

    // merge encoded input into stream input buffer
    inbuf.merge(readoutbuf);

    // try to push pending encoded output to cloned stream
    // outbuf_delayed_flush condition already handled by uwrite()
    push(false /*flush*/, false /*finish*/);

    return WvStreamClone::pre_select(si);
}


void WvEncoderStream::checkreadisok()
{
    if (! readchain.isok())
        seterr(WvString("read chain: %s", readchain.geterror()));
}


void WvEncoderStream::checkwriteisok()
{
    if (! writechain.isok())
        seterr(WvString("write chain: %s", writechain.geterror()));
}
