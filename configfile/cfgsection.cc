/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 *
 * Implementation of the WvConfigSection class. 
 *
 * Created:	Sept 28 1997		D. Coombs
 *
 */
#include "wvconf.h"

WvConfigSection::WvConfigSection( char * n )
/******************************************/
{
    name = new char[ strlen( n ) + 1 ];
    strcpy( name, n );
}

WvConfigSection::~WvConfigSection()
/*********************************/
{
    // the WvConfigEntryList destructor automatically deletes all its
    // entries, so we don't need to worry about doing that.

    delete[] name;
}

WvConfigEntry *	WvConfigSection::get_entry( char * e )
/****************************************************/
{
    WvConfigEntryList::Iter	iter( entries );

    for( iter.rewind(); iter.next(); ) {
    	if( strcasecmp( iter.data()->get_name(), e ) == 0 ) {
            return( iter.data() );
        }
    }

    return( NULL );
}

void WvConfigSection::set_entry( char * e, char * v )
/***************************************************/
{
    WvConfigEntryList::Iter	iter( entries );
    WvConfigEntry *		ent;

    // First check to see if the entry is already in this section.
    iter.rewind();
    while( iter.next() ) {
    	ent = iter.data();
    	if( strcasecmp( ent->get_name(), e ) == 0 ) {
    	    ent->set_value( v );
	    return;
        }
    }

    // The entry is not in the list, so now we must append it.
    ent = new WvConfigEntry( e, v );
    entries.append( ent, true );

    return;
}

void WvConfigSection::append_entry( WvConfigEntry * ent )
/*******************************************************/
{
    entries.append( ent, true );
}

void WvConfigSection::delete_entry( char * entry )
/************************************************/
{
    WvConfigEntryList::Iter i ( entries );
    
    for( i.rewind(); i.cur() && i.next(); ) {
    	if( !strcasecmp( i.data()->get_name(), entry ) )
            i.unlink(); // i.cur() may become NULL!
    }
}

void WvConfigSection::dump( FILE * fp )
/*************************************/
{
    WvConfigEntryList::Iter i( entries );

    for( i.rewind(); i.next(); ) {
	WvConfigEntry &	ent = *i.data();
	char *		val = ent.get_value();
	if( val[0] )
	    fprintf( fp, "%s = %s\n", ent.get_name(), val );
	else
	    fprintf( fp, "%s\n", ent.get_name() );
    }
}
