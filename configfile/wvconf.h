/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 *
 * Definition of the WvConfigFile, WvConfigSection, and WvConfigEntry classes, 
 * which are used to read and write entries from a Windows-INI-style file.
 *
 * Created:	Sept 12 1997		D. Coombs
 *
 */

#ifndef __WVCONF_H
#define __WVCONF_H

#include "strutils.h"
#include "wvlinklist.h"
#include "wvlog.h"
#include "wvstring.h"

DeclareWvList( WvString );

class WvConfigEntry
/*****************/
{
public:
    WvConfigEntry();
    WvConfigEntry( char * n, char * val );
    ~WvConfigEntry();

    inline char *	get_name() const
        { return name; }
    inline char *	get_value() const
        { return value; }
    void		set_value( char * val );

private:
    char *	name;
    char *	value;
};

DeclareWvList( WvConfigEntry );

class WvConfigSection
/*******************/
{
public:
    WvConfigSection( char * n );
    ~WvConfigSection();

    WvConfigEntry *	get_entry( char * e );
    void		set_entry( char * e, char * v );
    void		set_entry( WvConfigEntry * e, char * v );

    void		append_entry( WvConfigEntry * e );
    void		delete_entry( char * name );
    inline char *	get_name() const
        { return name; }
    inline bool		is_this( char * n ) const
	{ return strcasecmp( n, name ) == 0; }

    void		dump( FILE * fp );

    WvConfigEntryList	entries;
private:
    char *		name;
};

DeclareWvList( WvConfigSection );

class WvConf
/**********/
{
public:
    WvConf( char * file_name );
    ~WvConf();

    WvConfigSection *	get_section( char * section );
    inline int		get( char * section, char * name, int def_val );
    char *		get( char * a_section, 
    			     char * a_name, char * def_val = NULL );

    inline int		fuzzy_get( WvStringList& sect, char * ent,
    			     int def_val = 0 );
    char *		fuzzy_get( WvStringList& sect, char * ent,
    			     char * def_val = NULL );

    inline int		fuzzy_get( WvStringList& sect, WvStringList& ent,
    			     int def_val = 0 );
    char *		fuzzy_get( WvStringList& sect, WvStringList& ent,
    			     char * def_val = NULL );

    void		set( char * section, char * name, char * value );
    inline void		set( char * section, char * name, int value );

    void		delete_section( char * section );

    void		flush();

    bool isok() const
        { return !error; }
    bool isclean() const
        { return isok() && !dirty; }

private:
    bool		dirty;		// true if changed since last flush()
    bool		error;		// true if something has gone wrong
    WvString		config_file;
    WvLog		log;

    WvConfigSection	globalsection;
    WvConfigSection *	last_found_section;
    WvConfigSectionList	sections;

    void		load_file();
    char *		parse_section( char * s );
    char *		parse_value( char * s );
};

static int check_for_bool_string( char * s )
/******************************************/
{
    if( strcasecmp( s, "off" ) == 0 ||
    	strcasecmp( s, "false" ) == 0 ||
    	strncasecmp( s, "no", 2 ) == 0 )	// also handles "none"
    		return( 0 );

    if( strcasecmp( s, "on" ) == 0 ||
    	strcasecmp( s, "true" ) == 0 ||
    	strcasecmp( s, "yes" ) == 0 )
    		return( 1 );

    // It's not a special bool case, so just return the number
    return( atoi( s ) );
}

inline int  WvConf::get( char * section, char * name, int def_val )
/*****************************************************************/
// This "int" version of get is smart enough to interpret words like on/off,
// true/false, and yes/no.
{
    char	num[ 25 ];
    char *	response;

    sprintf( num, "%d", def_val );
    response = get( section, name, num );
    return( check_for_bool_string( response ) );
}

inline int  WvConf::fuzzy_get( WvStringList& 	sect, 
			       WvStringList& 	ent,
			       int 		def_val )
/*******************************************************/
// This "int" version of fuzzy_get is smart enough to interpret words like 
// on/off, true/false, and yes/no.
{
    char	num[ 25 ];
    char *	response;

    sprintf( num, "%d", def_val );
    response = fuzzy_get( sect, ent, num );
    return( check_for_bool_string( response ) );
}

inline int  WvConf::fuzzy_get( WvStringList&	sect,
			       char *		ent,
			       int		def_val )
/*******************************************************/
// This "int" version of fuzzy_get is smart enough to interpret words like 
// on/off, true/false, and yes/no.
{
    char	num[ 25 ];
    char *	response;

    sprintf( num, "%d", def_val );
    response = fuzzy_get( sect, ent, num );
    return( check_for_bool_string( response ) );
}

inline void WvConf::set( char * section, char * name, int value )
/***************************************************************/
{
    char	num[ 25 ];

    sprintf( num, "%d", value );
    set( section, name, num );
}

#endif
