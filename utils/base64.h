/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 *
 * Functions for encoding and decoding strings in MIME's Base64 notation.
 *
 */

#ifndef __BASE64_H
#define __BASE64_H

/**
 * Base 64 encode str
 */
extern char * base64_encode( char * str );
/**
 * Decode Base64 encoded str
 */
extern char * base64_decode( char * str );

#endif // __BASE64_H
