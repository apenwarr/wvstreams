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
    zstr->msg = NULL;
    
    int retval;
    if (mode == Deflate)
	retval = deflateInit(zstr, Z_DEFAULT_COMPRESSION);
    else
	retval = inflateInit(zstr);
    
    if (retval != Z_OK)
    {
        seterror("error %s initializing gzip %s: %s", retval,
            mode == Deflate ? "compressor" : "decompressor",
            zstr->msg ? zstr->msg : "unknown");
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


bool WvGzipEncoder::_encode(WvBuf &inbuf, WvBuf &outbuf, bool flush)
{
    bool success;
    for (;;)
    {
        prepare(& inbuf);
        bool alldata = inbuf.used() == 0;
        success = process(outbuf, flush && alldata, false);
        if (zstr->avail_in != 0)
        {
            // unget unused data
            inbuf.unget(zstr->avail_in);
            zstr->avail_in = 0;
        }
        if (! success)
            return false;
        if (alldata)
            return true;
    }
}


bool WvGzipEncoder::_finish(WvBuf &outbuf)
{
    prepare(NULL);
    return process(outbuf, false, true);
}


void WvGzipEncoder::prepare(WvBuf *inbuf)
{
    assert(zstr->avail_in == 0);
    if (inbuf && inbuf->used() != 0)
    {
        size_t avail = inbuf->optgettable();
        zstr->avail_in = avail;
        zstr->next_in = const_cast<Bytef*>(
            (const Bytef*)inbuf->get(avail));
    }
    else
    {
        zstr->avail_in = 0;
        zstr->next_in = (Bytef*)""; // so it's not NULL
    }
}


bool WvGzipEncoder::process(WvBuf &outbuf, bool flush, bool finish)
{
    int flushmode = finish ? Z_FINISH :
        flush ? Z_SYNC_FLUSH : Z_NO_FLUSH;
    int retval;
    do
    {
        // process the next chunk
        tmpbuf.zap();
        zstr->avail_out = tmpbuf.free();
	zstr->next_out = tmpbuf.alloc(tmpbuf.free());
	if (mode == Deflate)
	    retval = deflate(zstr, flushmode);
	else
	    retval = inflate(zstr, flushmode);
	tmpbuf.unalloc(zstr->avail_out);

        // consume pending output
        outbuf.merge(tmpbuf);
    } while (retval == Z_OK);

    if (retval == Z_STREAM_END)
        setfinished();
    else if (retval != Z_OK && retval != Z_BUF_ERROR)
    {
        seterror("error %s during gzip %s: %s", retval,
            mode == Deflate ? "compression" : "decompression",
            zstr->msg ? zstr->msg : "unknown");
        return false;
    }
    return true;
}

