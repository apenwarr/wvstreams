/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Code to serialize and deserialize objects to/from WvBufs.
 * See wvserialize.h.
 */
#include "wvserialize.h"

template <>
WvString _wv_deserialize<WvString>(WvBuf &buf)
{
    unsigned int len = buf.strchr('\0');
    if (buf.used() < len)
	return WvString();
    else
	return (const char *)buf.get(len);
}
