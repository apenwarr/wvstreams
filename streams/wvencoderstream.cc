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
    WvStreamClone(_cloned), min_readsize(0)
{
}


WvEncoderStream::~WvEncoderStream()
{
    close();
}


void WvEncoderStream::close()
{
    // flush read chain
    flushread();
    // flush write chain and close the stream
    WvStreamClone::close();
}


void WvEncoderStream::flush_internal(time_t msec_timeout)
{
    // flush outbuf through encoder
    writeinbuf.merge(outbuf);
    push(true /*flush*/);
    assert(outbuf.used() == 0);
 
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


void WvEncoderStream::flushread()
{
    // flush read chain then merge the input buffers
    readchain.encode(readinbuf, readoutbuf, true /*flush*/);
    checkisok();
    inbuf.merge(readoutbuf);
}


void WvEncoderStream::pull(size_t size)
{
    // pull a chunk of unencoded input
    if (size != 0)
    {
        unsigned char *readin = readinbuf.alloc(size);
        size_t len = WvStreamClone::uread(readin, size);
        readinbuf.unalloc(size - len);
    }

    // encode the input
    bool flush = cloned && ! cloned->isok();
    readchain.encode(readinbuf, readoutbuf, flush);
    checkisok();
}


void WvEncoderStream::push(bool flush)
{
    // encode the output
    writechain.encode(writeinbuf, writeoutbuf, flush);
    checkisok();

    // push encoded output to cloned stream
    size_t size = writeoutbuf.used();
    if (size != 0)
    {
        const unsigned char *writeout = writeoutbuf.get(size);
        size_t len = WvStreamClone::uwrite(writeout, size);
        writeoutbuf.unget(size - len);
    }
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
    push(false /*flush*/);
    return size;
}


bool WvEncoderStream::pre_select(SelectInfo &si)
{
    // merge encoded input to stream input buffer
    inbuf.merge(readoutbuf);

    // try to push pending encoded output to cloned stream
    // outbuf_delayed_flush condition already handled by uwrite()
    push(false /*flush*/);
    return WvStreamClone::pre_select(si);
}


void WvEncoderStream::checkisok()
{
    bool readisok = readchain.isok();
    bool writeisok = writechain.isok();
    if (! readisok)
        if (! writeisok)
            seterr("Error occurred in read and write encoder chains");
        else
            seterr("Error occurred in read encoder chain");
    else if (! writeisok)
        seterr("Error occurred in write encoder chain");
}
