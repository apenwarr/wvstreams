/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 *
 * Implementation of the WvConfigFile class.
 *
 * Created:	Sept 12 1997		D. Coombs
 *
 */
#include "wvconf.h"
#include "wvstream.h"
#include <string.h>
#include <unistd.h>

WvConf::WvConf( char * file_name )
/********************************/
: config_file( file_name ), log( config_file ),
    globalsection( "" )
{
    last_found_section = NULL;
    dirty = error = false;
    load_file();
}

void WvConf::load_file()
/**********************/
{
    WvFile		file;
    char *		p;
    char *		from_file;
    WvConfigSection *	sect	= &globalsection;

    file.open( config_file.str, O_RDONLY );
    if( !file.isok() ) {
    	// Could not open for read.
	log( WvLog::Warning, "Can't read config file: %s\n", file.errstr() );
	if( file.geterr() != ENOENT )
	    error = true;
	return;
    }

    while( ( from_file = trim_string( file.getline(0) ) ) != NULL ) {
	if( ( p = parse_section( from_file ) ) != NULL ) { 
	    // a new section?
	    if( !p[0] ) // blank name: global section
		sect = &globalsection;
	    else if( get_section( p ) )
		sect = get_section( p );
	    else
	    {
		sect = new WvConfigSection( p );
		sections.append( sect, true );
	    }
	} else { 
	    // it must be an element for the current section *sect.
	    p = parse_value( from_file) ;
	    if( !p ) p = ""; // allow empty entries

	    from_file = trim_string( from_file );
	    if( from_file[0] ) // nonblank option name
		sect->set_entry( from_file, p );
	}
    }
}

WvConf::~WvConf()
/***************/
{
    // We don't really have to do anything here.  sections's destructor
    // will go through and delete all its entries, so we should be fine.

    flush();
}

char *	WvConf::get( char * a_section, char * a_name, char * def_val )
/********************************************************************/
{
    WvConfigSection *	sect;
    WvConfigEntry *	ent;

    char *	section = trim_string( a_section );
    char *	name 	= trim_string( a_name );

    sect = get_section( section );
    
    ent = NULL;
    if( sect )
	ent = sect->get_entry( name );
    if( !ent )
	ent = globalsection.get_entry( name );
    
    return( ent ? ent->get_value() : def_val );
}

char *	WvConf::fuzzy_get( WvStringList&	sect,
			   WvStringList&	ent,
			   char *		def_val )
/*******************************************************/
{
    WvStringList::Iter	iter( sect );
    WvStringList::Iter	iter2( ent );
    char *		ret;

    for( iter.rewind(); iter.next(); ) {
	char *	p         = strdup( iter.data()->str );
	char *	this_sect = trim_string( p );

	for( iter2.rewind(); iter2.next(); ) {
	    char * q	  	= strdup( iter.data()->str );
	    char * this_ent	= trim_string( q );

	    ret = get( this_sect, this_ent );
	    if( ret != NULL ) {
	    	free( p );
	    	free( q );
	    	return( ret );
	    }
	    free( q );
	}
	free( p );
    }
    return( def_val );
}

char *	WvConf::fuzzy_get( WvStringList &	sect,
			   char *		ent,
			   char *		def_val )
/*******************************************************/
{
    WvStringList::Iter	iter( sect );
    char *		ret;

    for( iter.rewind(); iter.next(); ) {
    	char *	p	  = strdup( iter.data()->str );
    	char *	this_sect = trim_string( p );

	ret = get( this_sect, ent );
	if( ret != NULL ) {
	    free( p );
	    return( ret );
	}
	free( p );
    }
    return( def_val );
}

void WvConf::set( char * section, char * name, char * value )
/***********************************************************/
{
    WvConfigSection *	sect;
    WvConfigEntry *	ent;
    char *		p;

    // First find a matching section....
    sect = get_section( section );
    if( !sect ) {
    	// Make one...
    	sect = new WvConfigSection( section );
    	sections.add_after( sections.tail, sect, true );
    }
    
    if( !value ) {
	// delete the entry
	sect->delete_entry( name );
	dirty = true;
    } else {
	// add/modify the entry 
	WvString tmp_string( value );
	ent = sect->get_entry( name );
	p = trim_string( tmp_string.str );
	if( !ent || strcasecmp( ent->get_value(), p ) ) {
	    if( !ent )
		sect->set_entry( name, p ); // appends the entry
	    else
		ent->set_value( p );	// more efficient otherwise
	    dirty = true;    // !!! NOTE:  Don't forget to flush() !!!
	}
    }
}

WvConfigSection * WvConf::get_section( char * section )
/*****************************************************/
{
    WvConfigSectionList::Iter	iter( sections );
    WvConfigSection *		sect;
    
    // short circuit the usual case, when we access the same section
    // several times in a row.
    if( last_found_section && last_found_section->is_this( section ) )
	return( last_found_section );

    // otherwise, search the whole list.
    for( iter.rewind(); iter.next(); ) {
    	sect = iter.data();
    	if( sect->is_this( section ) == true ) {
	    last_found_section = sect;
    	    return( sect );
    	}
    }
    return( NULL );
}

void WvConf::delete_section( char * section )
/*******************************************/
{
    WvConfigSectionList::Iter i( sections );
    
    for( i.rewind(); i.cur() && i.next(); ) {
	if( i.data()->is_this( section ) ) {
	    if( last_found_section == i.data() )
		last_found_section = NULL;
	    i.unlink(); // may make i.cur()==0, so i.next() would segfault
	    dirty = true;
	}
    }
}

char *	WvConf::parse_section( char * s )
/***************************************/
{
    char *	q;

    if( s[0] != '[' )
    	return( NULL );

    q = strchr( s, ']' );
    if( !q || q[1] )
	return( NULL );

    *q = 0;
    return trim_string( s + 1 );
}

char *	WvConf::parse_value( char * s )
/*************************************/
{
    char *	q;

    q = strchr( s, '=' );
    if( q == NULL )
    	return( NULL );

    *q++ = 0; // 's' points to option name, 'q' points to value
    return( trim_string( q ) );
}

void WvConf::flush()
/******************/
{
    FILE *	fp;

    if( dirty && !error ) {
    	fp = fopen( config_file.str, "w" );
    	if( !fp ) {
	    log( WvLog::Error, "Can't write to config file: %s\n",
		strerror( errno ) );
	    error = true;
    	    return;
	}
	
	globalsection.dump( fp );

	WvConfigSectionList::Iter i( sections );
	for( i.rewind(); i.next(); ) {
	    WvConfigSection &sect = *i.data();
	    fprintf( fp, "\n[%s]\n", sect.get_name() );
	    sect.dump( fp );
	}
	
	fclose( fp );
	dirty = false;
    }
}
