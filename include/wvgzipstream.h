/* -*- Mode: C++ -*-
 *
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.

 * A stream implementing Gzip encryption and decryption.
 * See WvGzipEncoder for details.
 *
 * By default, written data is "deflated", read data is "inflated".
 */
#ifndef __WVGZIPSTREAM_H
#define __WVGZIPSTREAM_H

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
