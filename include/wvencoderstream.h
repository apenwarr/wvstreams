/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvEncoderStream chains a series of encoders on the input and
 * output ports of the underlying stream to effect on-the-fly data
 * transformations.
 *
 * Notice that the WvEncoderStream's outbuf_delayed_flush flag
 * takes on significant importance when working with encoders that
 * treat explicit buffer flushing in a special manner, eg. as with
 * Gzip encoder.  For Gzip, if this flag were false, each incremental
 * write call would cause the encoder to flush out small poorly
 * compressed chunks.  However, for interactive operation, if the
 * flag were true, output might not be generated until flush() were
 * called (which is seldom used by WvStreams clients since they
 * tend to assume automatic flushing on each write as the default).
 *
 * To set the flag, call delay_output(true).
 * Notice that only flush() [not select()] will cause output to be flushed.
 */
#ifndef __WVENCODERSTREAM_H
#define __WVENCODERSTREAM_H

#include "wvstream.h"
#include "wvstreamclone.h"
#include "wvencoder.h"

class WvEncoderStream : public WvStreamClone
{
    WvBuffer readinbuf;
    WvBuffer readoutbuf;
    WvBuffer writeinbuf;
    WvBuffer writeoutbuf;
public:
    // encoder chains for reading and writing respectively
    WvEncoderChain readchain;
    WvEncoderChain writechain;

    // sets the minimum number of bytes the encoder should try to read
    // at once, to optimize performance of block-oriented protocols
    // if 0, the encoder will only whatever is specified in uread()
    size_t min_readsize;

    WvEncoderStream(WvStream *_cloned);
    virtual ~WvEncoderStream();

    virtual void close();

    // causes data passing through the read chain of encoders to be
    // flushed to the input buffer
    // note: the regular flush() operates on the write chain of encoders
    virtual void flushread();

protected:
    bool pre_select(SelectInfo &si);
    virtual size_t uread(void *buf, size_t size);
    virtual size_t uwrite(const void *buf, size_t size);
    virtual void flush_internal(time_t msec_timeout);

private:
    void checkisok();
    
    // pulls a chunk of specified size from the underlying stream
    void pull(size_t size);

    // pushes a chunk to the underlying stream
    void push(bool flush);
};

#endif // __WVENCODERSTREAM_H
