/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 *
 * Implementation of the WvConfigEntry class. 
 *
 * Created:	Sept 28 1997		D. Coombs
 *
 */
#include "wvconf.h"

WvConfigEntry::WvConfigEntry()
/****************************/
{
    value = name = NULL;
}

WvConfigEntry::WvConfigEntry( char * n, char * val )
/**************************************************/
{
    value = new char[ strlen( val ) + 1 ];
    name = new char[ strlen( n ) + 1 ];

    strcpy( value, val );
    strcpy( name, n );
}

WvConfigEntry::~WvConfigEntry()
/*****************************/
{
    if( value ) delete[] value;
    if( name )  delete[] name;
}

void WvConfigEntry::set_value( char * val )
/*****************************************/
{
    if( val != NULL ) {
	delete[] value;
	value = new char[ strlen( val ) + 1 ];
	strcpy( value, val );
    }

    return;
}
