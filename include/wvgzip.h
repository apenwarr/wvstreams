/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Gzip encoder/decoder based on zlib.
 */
#ifndef __WVGZIP_H
#define __WVGZIP_H

#include "wvencoder.h"
#include "wvencoderstream.h"

/**
 * An encoder implementing Gzip encryption and decryption.
 */
struct z_stream_s;
class WvGzipEncoder : public WvEncoder
{
public:
    enum Mode {
        Deflate, // compress using deflate
        Inflate  // decompress using inflate
    };
    
    WvGzipEncoder(Mode _mode);
    virtual ~WvGzipEncoder();
    
    virtual bool isok() const;
    virtual bool encode(WvBuffer &in, WvBuffer &out, bool flush);

private:
    bool okay;
    struct z_stream_s *zstr;
    WvMiniBuffer tmpbuf;
    Mode mode;
};


/**
 * A stream implementing Gzip encryption and decryption.
 * See WvGzipEncoder for details.
 *
 * By default, written data is "deflated", read data is "inflated".
 */
class WvGzipStream : public WvEncoderStream
{
public:
    WvGzipStream(WvStream *_cloned,
        WvGzipEncoder::Mode readmode = WvGzipEncoder::Inflate,
        WvGzipEncoder::Mode writemode = WvGzipEncoder::Deflate);
    virtual ~WvGzipStream() { }
};


#endif // __WVGZIP_H
