/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Functions for encoding and decoding strings in MIME's Base64 notation.
 *
 * Base 64 is pretty easy.  The input is processed in groups of three bytes.
 * These 24 bits are split into 4 groups of 6 bits.  Each group of six bits
 * is represented by one character in the base64 alphabet, in the encoded
 * output.  The alphabet is as follows:
 * 	ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=
 * Where 'A' through '/' represent 000000 through 011111, the 64 different
 * combinations.  The '=' (100000) is padding and has no value when decoded.
 */
#include "base64.h"
#include "wvbuffer.h"
#include <string.h>

static char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
			"0123456789+/=";

WvString base64_encode(const void *buf, size_t length)
{
    WvDynBuf in, out;
    size_t out_length = length * 4/3 + ((length%3) != 0);
    size_t out_pos = 0, in_pos = 0;
    
    in.put(buf, length);
    in.put("\0\0", 3); // add some extra bytes for the algorithm
    unsigned char *iptr = in.get(in.used());

    for (in_pos = out_pos = 0; ; in_pos += 3, out_pos += 4)
    {
        if (out_pos < out_length)
            out.putch(alphabet[iptr[0] >> 2]);
        else
            break;

        if (out_pos+1 < out_length)
            out.putch(alphabet[((iptr[0] & 0x03) << 4) | (iptr[1] >> 4)]);
	else
	    break;
	
	if (out_pos+2 < out_length)
            out.putch(alphabet[((iptr[1] & 0x0f) << 2) | (iptr[2] >> 6)]);
	else
	    break;
	
	if (out_pos+3 < out_length)
	    out.putch(alphabet[iptr[2] & 0x3f]);
	else
	    break;
	
	iptr += 3;
    }

    // Now how many '=' signs should we have had at the end?
    switch (length % 3) 
    {
    case 1:
	out.putch('=');
	// FALL THROUGH
    case 2:
	out.putch('=');
	break;
    case 0:
    default:
	break;
    }
    
    return out.getstr();
}


// Takes a base-64 alpha p, and returns its offset in the alphabet, 0-64.
static inline char ofs(char p)
{
    char *ptr = strchr(alphabet, p);
    if (ptr)
	return ptr - alphabet;
    else
	return 0;
}


// No error checking is performed!  Assume str really is a Base64 stream!
WvBuf &base64_decode(WvStringParm str, WvBuf &out)
{
    size_t length = strlen(str);
    int	out_length, out_pos = 0, in_pos = 0;

    // First find out how long the decoded string will be.
    out_length = length / 4 * 3;
    if (str[length - 1] == '=')
    	out_length--;
    if (str[length - 2] == '=')
    	out_length--;

    for (;;)
    {
    	if (str[in_pos] == '\0')  break;
	out.putch((ofs(str[in_pos]) << 2) | (ofs(str[in_pos+1]) >> 4));
	if (str[in_pos+2] == '=') break;
	out.putch(((ofs(str[in_pos+1]) & 0x0f ) << 4)
		  | (ofs(str[in_pos+2]) >> 2));
	if (str[in_pos+3] == '=') break;
	out.putch(((ofs(str[in_pos+2]) & 0x03) << 6) | ofs(str[in_pos+3]));
	in_pos  += 4;
	out_pos += 3;
    }
    
    return out;
}
