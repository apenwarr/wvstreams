/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * gzip encoder/decoders based on zlib.
 */
#ifndef __WVGZIP_H
#define __WVGZIP_H

#include "wvencoder.h"

struct z_stream_s;

class WvGzipEncoder : public WvEncoder
{
public:
    enum GzipMode { Compress, Decompress };
    
    WvGzipEncoder(GzipMode _mode);
    virtual ~WvGzipEncoder();
    
    virtual bool isok() const;
    virtual bool encode(WvBuffer &in, WvBuffer &out, bool flush);

private:
    bool okay;
    struct z_stream_s *zstr;
    WvMiniBuffer tmpbuf;
    GzipMode mode;
};

#endif // __WVGZIP_H
