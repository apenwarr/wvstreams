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
#include <string.h>

static char * alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
			 "0123456789+/=";

char * base64_encode( const char * str, int length )
/********************************/
{
    int    out_length = ( ( length - 1 ) / 3 + 1 ) * 4;
    int	   in_length  = ( ( length - 1 ) / 3 + 1 ) * 3;
    char * out;
    char * in;
    int    out_pos = 0;
    int	   in_pos  = 0;

    out = new char[ out_length + 1 ];
    in  = new char[ in_length  + 1 ];
    memset( in, '\0', in_length+1 );
    memcpy ( in, str, length*sizeof(char));

    for( ;; ) {
        if (out_pos < out_length)
            (out[ out_pos ] = alphabet[ in[ in_pos ] >> 2]);
        else
            break;

        if (out_pos+1 < out_length)
            (out[ out_pos + 1 ] = alphabet[ ( ( in[ in_pos ] & 0x03 ) << 4 ) | ( in[ in_pos+1 ] >> 4 ) ]);
        else
            break;

    	if (out_pos+2 < out_length)
            (out[ out_pos + 2 ] = alphabet[ ( ( in[ in_pos+1 ] & 0x0f ) << 2 ) | ( in[ in_pos+2 ] >> 6 ) ]);
        else
            break;

    	if (out_pos+3 < out_length)
            (out[ out_pos + 3 ] = alphabet[ in[ in_pos+2 ] & 0x3f ]);
        else
            break;

    	in_pos += 3;
    	out_pos += 4;
    }

    out[ out_length ] = '\0';
    delete in;

    // Now how many '=' signs should we have had at the end?
    switch( length % 3 ) {
    	case 1:
    	    out[ out_length-2 ] = '=';
    	    // FALL THROUGH
    	case 2:
    	    out[ out_length-1 ] = '=';
    	    break;
    	case 0:
    	default:
    	    break;
    }

    return( out );
}

static inline char ofs( char p )
/******************************/
// Takes a base-64 alpha p, and returns its offset in the alphabet, 0-64.
{
    return( strchr( alphabet, p ) - alphabet );
}

char * base64_decode( const char * str, int length )
/********************************/
// No error checking is performed!  Assume str really is a Base64 stream!
{
    char * out;
    int	   out_length;
    int	   out_pos = 0;
    int	   in_pos  = 0;

    // First found out how long the decoded string will be.
    out_length = length / 4 * 3;
    if( str[ length - 1 ] == '=' )
    	out_length--;
    if( str[ length - 2 ] == '=' )
    	out_length--;

    out = new char[ out_length + 1];
    for( ;; ) {
    	if( str[ in_pos ] == '\0' )  break;
	out[ out_pos ] = ( ofs( str[ in_pos ] ) << 2 )
		       | ( ofs( str[ in_pos+1 ] ) >> 4 );
	if( str[ in_pos+2 ] == '=' ) break;
	out[ out_pos + 1 ] = ( ( ofs( str[ in_pos+1 ] ) & 0x0f ) << 4 )
			     | ( ofs( str[ in_pos+2 ] ) >> 2 );
	if( str[ in_pos+3 ] == '=' ) break;
	out[ out_pos + 2 ] = ( ( ofs( str[ in_pos+2 ] ) & 0x03 ) << 6 )
			       | ofs( str[ in_pos+3 ] );
	in_pos  += 4;
	out_pos += 3;
    }

    out[ out_length ] = '\0';
    return( out );
}
