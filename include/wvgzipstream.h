/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * A Gzip stream.
 */
#ifndef __WVGZIPSTREAM_H
#define __WVGZIPSTREAM_H

#include "wvgzip.h"

/**
 * A stream implementing Gzip compression and decompression.
 * <p>
 * By default, written data is compressed using WvGzipEncoder::Deflate,
 * read data is decompressed using WvGzipEncoder::Inflate.
 * </p>
 * @see WvGzipEncoder
 */
class WvGzipStream : public WvEncoderStream
{
public:
    WvGzipStream(WvStream *_cloned,
		 WvGzipEncoder::Mode readmode = WvGzipEncoder::Inflate,
		 WvGzipEncoder::Mode writemode = WvGzipEncoder::Deflate)
	{
	    readchain.append(new WvGzipEncoder(readmode), true);
	    writechain.append(new WvGzipEncoder(writemode), true);
	}
    virtual ~WvGzipStream() { }
};


#endif /* __WVGZIPSTREAM_H */