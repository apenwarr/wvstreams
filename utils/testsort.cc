/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
 *
 * Test program for sorted iterators on lists.
 *
 * Correct output:
 *      Frontwards: eight five four nine one seven six ten three two 
 *      Unsorted:   one two three four five six seven eight nine ten 
 *      Backwards:  two three ten six seven one nine four five eight 
 *
 */

#include <stdio.h>
#include "wvstring.h"
#include "wvlinklist.h"

DeclareWvList( WvString )

int apples_to_oranges( const WvString ** a, const WvString ** b )
/***************************************************************/
{
    return( strcmp( **a, **b ) );
}

int oranges_to_apples( const WvString ** a, const WvString ** b )
/***************************************************************/
{
    return( -strcmp( **a, **b ) );
}

int main()
/********/
{
    WvStringList l;
    l.append( new WvString( "one" ), true );
    l.append( new WvString( "two" ), true );
    l.append( new WvString( "three" ), true );
    l.append( new WvString( "four" ), true );
    l.append( new WvString( "five" ), true );
    l.append( new WvString( "six" ), true );
    l.append( new WvString( "seven" ), true );
    l.append( new WvString( "eight" ), true );
    l.append( new WvString( "nine" ), true );
    l.append( new WvString( "ten" ), true );

    printf( "Frontwards: " );
    {
        WvStringList::Sorter s( l, apples_to_oranges );
        for( s.rewind(); s.next(); )
            printf( "%s ", (const char *) s() );
    }

    printf( "\nUnsorted:   " );
    {
        WvStringList::Iter i( l );
        for( i.rewind(); i.next(); )
            printf( "%s ", (const char *) i() );
    }

    printf( "\nBackwards:  " );
    {
        WvStringList::Sorter s( l, oranges_to_apples );
        for( s.rewind(); s.next(); )
            printf( "%s ", (const char *) s() );
    }

    return( 0 );
}
