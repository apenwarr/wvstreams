/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Gzip encoder/decoder based on zlib.
 */
#include "wvgzip.h"
#include <zlib.h>
#include <assert.h>

#define ZBUFSIZE 10240

/***** GzipEncoder *****/

WvGzipEncoder::WvGzipEncoder(Mode _mode) :
    tmpbuf(ZBUFSIZE), mode(_mode)
{
    zstr = new z_stream;
    memset(zstr, 0, sizeof(*zstr));
    zstr->zalloc = Z_NULL;
    zstr->zfree = Z_NULL;
    zstr->opaque = NULL;
    
    int retval;
    if (mode == Deflate)
	retval = deflateInit(zstr, Z_DEFAULT_COMPRESSION);
    else
	retval = inflateInit(zstr);
    
    if (retval != Z_OK)
    {
        seterror("error %s initializing gzip %s", retval,
            mode == Deflate ? "compressor" : "decompressor");
        return;
    }
    zstr->next_in = zstr->next_out = NULL;
    zstr->avail_in = zstr->avail_out = 0;
}


WvGzipEncoder::~WvGzipEncoder()
{
    if (mode == Deflate)
        deflateEnd(zstr);
    else
        inflateEnd(zstr);

    delete zstr;
}


bool WvGzipEncoder::_encode(WvBuffer &inbuf, WvBuffer &outbuf, bool flush)
{
    prepare(& inbuf);
    bool success = process(outbuf, flush, false);
    if (zstr->avail_in != 0)
    {
        // unget unused data
        inbuf.unget(zstr->avail_in);
        zstr->avail_in = 0;
    }
    return success;
}


bool WvGzipEncoder::_finish(WvBuffer &outbuf)
{
    prepare(NULL);
    return process(outbuf, false, true);
}


void WvGzipEncoder::prepare(WvBuffer *inbuf)
{
    assert(zstr->avail_in == 0);
    if (inbuf && inbuf->used() != 0)
    {
        zstr->avail_in = inbuf->used();
        zstr->next_in = inbuf->get(inbuf->used());
    }
    else
    {
        zstr->avail_in = 0;
        zstr->next_in = (Bytef*)""; // so it's not NULL
    }
}


bool WvGzipEncoder::process(WvBuffer &outbuf, bool flush, bool finish)
{
    int flushmode = finish ? Z_FINISH :
        flush ? Z_SYNC_FLUSH : Z_NO_FLUSH;
    int retval;
    do
    {
        // process the next chunk
        zstr->avail_out = tmpbuf.free();
	zstr->next_out = tmpbuf.alloc(tmpbuf.free());
	if (mode == Deflate)
	    retval = deflate(zstr, flushmode);
	else
	    retval = inflate(zstr, flushmode);
	tmpbuf.unalloc(zstr->avail_out);

        // consume pending output
	size_t tmpused = tmpbuf.used();
	outbuf.put(tmpbuf.get(tmpused), tmpused);
        tmpbuf.zap();
    } while (retval == Z_OK);

    if (retval == Z_STREAM_END)
        setfinished();
    else if (retval != Z_OK && retval != Z_BUF_ERROR)
    {
        seterror("error %s during gzip %s", retval,
            mode == Deflate ? "compression" : "decompression");
        return false;
    }
    return true;
}


/***** GzipStream *****/

WvGzipStream::WvGzipStream(WvStream *_cloned,
    WvGzipEncoder::Mode readmode, WvGzipEncoder::Mode writemode) :
    WvEncoderStream(_cloned)
{
    readchain.append(new WvGzipEncoder(readmode), true);
    writechain.append(new WvGzipEncoder(writemode), true);
}
