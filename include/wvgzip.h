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

struct z_stream_s;

/**
 * An encoder implementing Gzip encryption and decryption.
 * 
 * When compressing:
 * 
 *  - On flush(), the encoded data stream is synchronized such that
 *     all data compressed up to this point can be fully decompressed.
 *     
 *  - On finish(), the encoded data stream is finalized an a Gzip
 *     end of data marker is emitted.
 * 
 * 
 * When decompressing:
 * 
 *  - The encoder will transition to isfinished() == true on its own
 *     if a Gzip end of data marker is detected in the input.  After
 *     this point, no additional data can be decompressed.
 * 
 * 
 */
class WvGzipEncoder : public WvEncoder
{
public:
    enum Mode {
        Deflate, /*!< Compress using deflate */
        Inflate  /*!< Decompress using inflate */
    };
    
    /**
     * Creates a Gzip encoder.
     *
     * "mode" is the compression mode
     */
    WvGzipEncoder(Mode mode);
    virtual ~WvGzipEncoder();
    
protected:
    virtual bool _encode(WvBuf &inbuf, WvBuf &outbuf, bool flush);
    virtual bool _finish(WvBuf &outbuf);
    virtual bool _reset();

private:
    struct z_stream_s *zstr;
    WvInPlaceBuf tmpbuf;
    Mode mode;

    void init();
    void close();
    void prepare(WvBuf *inbuf);
    bool process(WvBuf &outbuf, bool flush, bool finish);
};


#endif // __WVGZIP_H
