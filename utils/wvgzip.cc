/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * gzip encoder/decoders based on zlib.
 */
#include "wvgzip.h"
#include <zlib.h>

#include <assert.h>

#define ZBUFSIZE 10240

WvGzip::WvGzip(GzipMode _mode) :
    okay(true), tmpbuf(ZBUFSIZE), mode(_mode)
{
    zstr = new z_stream;
    memset(zstr, 0, sizeof(*zstr));
    zstr->zalloc = Z_NULL;
    zstr->zfree = Z_NULL;
    zstr->opaque = NULL;
    
    int retval;
    if (mode == Compress)
	retval = deflateInit(zstr, Z_DEFAULT_COMPRESSION);
    else
	retval = inflateInit(zstr);
    
    if (retval != Z_OK)
    {
	okay = false;
	return;
    }
    
    zstr->next_in = zstr->next_out = NULL;
    zstr->avail_in = zstr->avail_out = 0;
}


WvGzip::~WvGzip()
{
    if (mode == Compress)
        deflateEnd(zstr);
    else
        inflateEnd(zstr);

    delete zstr;
}


bool WvGzip::isok() const
{
    return okay;
}


bool WvGzip::encode(WvBuffer &in, WvBuffer &out, bool flush)
{
    if (! okay) return false;
    assert(zstr->avail_in == 0);

    if (in.used() == 0)
    {
        if (! flush) return true; // nothing to do!
        zstr->avail_in = 0;
        zstr->next_in = (Bytef*)""; // so it's not NULL
    }
    else
    {
        zstr->avail_in = in.used();
        zstr->next_in = in.get(in.used());
    }    

    int retval;
    do
    {
	if (zstr->avail_out == 0)
	{
	    tmpbuf.zap();
	    assert(tmpbuf.free() == ZBUFSIZE);
	    zstr->avail_out = tmpbuf.free();
	    zstr->next_out = tmpbuf.alloc(tmpbuf.free());
	}

	tmpbuf.alloc(tmpbuf.free());
	if (mode == Compress)
	    retval = deflate(zstr, flush ? Z_SYNC_FLUSH : Z_NO_FLUSH);
	else
	    retval = inflate(zstr, flush ? Z_SYNC_FLUSH : Z_NO_FLUSH);
	tmpbuf.unalloc(zstr->avail_out);
        
	size_t tmpused = tmpbuf.used();
	out.put(tmpbuf.get(tmpused), tmpused);
    } while (retval == Z_OK && zstr->avail_out == 0);

    if (retval != Z_OK && retval != Z_STREAM_END &&
        retval != Z_BUF_ERROR)
        okay = false;
    return okay;
}
