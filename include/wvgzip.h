/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * Gzip encoder/decoder based on zlib.
 */
#ifndef __WVGZIP_H
#define __WVGZIP_H

#include "wvencoder.h"
#include "wvencoderstream.h"

struct z_stream_s;

/**
 * An encoder implementing Gzip encryption and decryption.
 * <p>
 * When compressing:
 * <ul>
 * <li>On flush(), the encoded data stream is synchronized such that
 *     all data compressed up to this point can be fully decompressed.
 *     </li>
 * <li>On finish(), the encoded data stream is finalized an a Gzip
 *     end of data marker is emitted.</li>
 * </ul>
 * </p><p>
 * When decompressing:
 * <ul>
 * <li>The encoder will transition to isfinished() == true on its own
 *     if a Gzip end of data marker is detected in the input.  After
 *     this point, no additional data can be decompressed.</li>
 * </ul>
 * </p>
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
     * @param mode the compression mode
     */
    WvGzipEncoder(Mode mode);
    virtual ~WvGzipEncoder();
    
protected:
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf, bool flush);
    virtual bool _finish(WvBuffer &outbuf);

private:
    struct z_stream_s *zstr;
    WvInPlaceBuffer tmpbuf;
    Mode mode;

    void prepare(WvBuffer *inbuf);
    bool process(WvBuffer &outbuf, bool flush, bool finish);
};


#endif // __WVGZIP_H
