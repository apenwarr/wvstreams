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

class WvGzip : public WvEncoder
{
    struct z_stream_s *zstr;
    WvMiniBuffer tmpbuf;
    
public:
    bool okay;
    
    enum GzipMode { Compress, Decompress } mode;
    
    WvGzip(GzipMode _mode);
    virtual ~WvGzip();
    
    virtual bool isok() const;
    
    virtual size_t do_encode(const unsigned char *in, size_t insize,
			     bool flush);
};


#endif // __WVGZIP_H
