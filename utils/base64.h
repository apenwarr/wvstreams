/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Functions for encoding and decoding strings in MIME's Base64 notation.
 */
#ifndef __BASE64_H
#define __BASE64_H

#include "wvstring.h"
#include "wvbuffer.h"

class WvBuffer;

/**
 * Base64-encode a buffer.
 */
WvString base64_encode(const void *buf, size_t length);

/**
 * Decode a base64-encoded string into a buffer.
 */
WvBuffer &base64_decode(WvStringParm in, WvBuffer &out);

#endif // __BASE64_H
