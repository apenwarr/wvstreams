/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2000 Net Integration Technologies, Inc.
 * 
 * Various useful string-based utilities.
 *
 */
#include "strutils.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>


char * terminate_string( char * string, char c )
/**********************************************/
// Add character c to the end of a string after removing crlf's.
// NOTE: You need a buffer that's at least one character bigger than the
// current length of the string, including the terminating NULL.
{
    char * p;

    if( string == NULL ) {
    	return( NULL );
    }

    p = string + strlen( string ) - 1;
    while( *p == '\r' || *p == '\n' ) {
    	p--;
    }
    *(++p) = c;
    *(++p) = 0;

    return( string );
}

char * trim_string( char * string )
/*********************************/
// Trims spaces off the front and end of strings.  Modifies the string.
// Specifically DOES allow string==NULL; returns NULL in that case.
{
    char * p;
    char * q;

    if( string == NULL )
    	return( NULL );

    p = string;
    q = string + strlen(string) - 1;

    while( isspace( *p ) )
    	p++;
    while( q >= p && isspace( *q ) )
    	*(q--) = 0;

    return( p );
}

void replace_char( void * _string, char c1, char c2, int length )
/**************************************************************/
// Searches _string (up to length bytes), replacing any occurrences of c1
// with c2.
{
    char *string = (char *)_string;
    for( int i=0; i < length; i++ )
    	if( *(string+i) == c1 )
    	    *(string+i) = c2;
}

char * strlwr( char * string )
/****************************/
{
    char *	p = string;
    while( *p ) {
    	*p = tolower( *p );
    	p++;
    }

    return( string );
}

bool is_word( char * string )
/***************************/
// true if all the characters in "string" are isalnum().
{
    char *	p;
    for( p = string; *p != 0; p++ ) {
    	if( isalnum( *p ) == 0 ) {
    	    return( false );
    	}
    }
    return( true );
}

// produce a hexadecimal dump of the data buffer in 'buf' of length 'len'.
// it is formatted with 16 bytes per line; each line has an address offset,
// hex representation, and printable representation.
WvString hexdump_buffer(unsigned char *buf, size_t len)
{
    size_t count, count2, top;
    WvString out;

    out.setsize(len / 16 * 80 + 80);
    char *cptr = out.edit();
    
    for (count = 0; count < len; count+=16)
    {
	top = len-count < 16 ? len-count : 16;
	cptr += sprintf(cptr, "[%03X] ", (unsigned int)count);
	
	// dump hex values
	for (count2 = 0; count2 < top; count2++)
	{
	    if (count2 && !(count2 % 4))
		*cptr++ = ' ';
	    cptr += sprintf(cptr, "%02X", buf[count+count2]);
	}
	
	// print horizontal separation
	for (count2 = top; count2 < 16; count2++)
	{
	    if (count2 && !(count2 % 4))
	    {
		strcat(cptr, "   ");
		cptr += 3;
	    }
	    else
	    {
		strcat(cptr, "  ");
		cptr += 2;
	    }
	}
	
	*cptr++ = ' ';
	
	// dump character representation
	for (count2 = 0; count2 < top; count2++)
	    cptr += sprintf(cptr, "%c",
			    (isprint(buf[count+count2])
			     ? buf[count+count2] : '.'));
	*cptr++ = '\n';
    }
    *cptr = 0;
    return out;
}


// return true if the character is a newline.
bool isnewline(char c)
{
    return c=='\n' || c=='\r';
}


// eg: hexify(foo, "ABCDEF", 4) will set foo to "41424344".
void hexify(char *obuf, unsigned char *ibuf, size_t len)
{
    while (len > 0)
    {
	sprintf(obuf, "%02x", *ibuf++);
	obuf += 2;
	len--;
    }
}


// eg: unhexify(foo, "41424344") sets foo to "ABCD".
void unhexify(unsigned char *obuf, char *ibuf)
{
    char lookup[] = "0123456789abcdef", *c, *c2;
    
    if (strlen(ibuf) % 1)  // odd number of bytes in a hex string?  No.
	return;
    
    while (*ibuf != 0)
    {
	c = strchr(lookup, *ibuf);
	c2 = strchr(lookup, *(ibuf+1));
	if (!c || !c2) return;
	
	*obuf++ = ((c - lookup) << 4) | (c2 - lookup);
	ibuf += 2;
    }
}


