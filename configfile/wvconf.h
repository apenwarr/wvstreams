/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 *
 * Definition of the WvConfigFile, WvConfigSection, and WvConfigEntry classes, 
 * which are used to read and write entries from a Windows-INI-style file.
 *
 * Created:     Sept 12 1997            D. Coombs
 *
 */

#ifndef __WVCONF_H
#define __WVCONF_H

#include "strutils.h"
#include "wvlinklist.h"
#include "wvlog.h"
#include "wvstring.h"

DeclareWvList(WvString);


class WvConfigEntry
{
public:
    WvConfigEntry();
    WvConfigEntry(const WvString &_name, const WvString &_value);
    ~WvConfigEntry();
    
    void set(const WvString &_value)
        { value = _value; }
    
    WvString name, value;
};


DeclareWvList(WvConfigEntry);


class WvConfigSection : public WvConfigEntryList
{
public:
    WvConfigSection(const WvString &name);
    ~WvConfigSection();
    
    WvConfigEntry *operator[] (const WvString &s);

    const char *get(const WvString &entry, const char *def_val = NULL);
    void set(const WvString &entry, const WvString &value);
    void set(WvConfigEntry *e, const WvString &value);

    void dump(FILE * fp);

    WvString name;
};


DeclareWvList(WvConfigSection);


class WvConf : public WvConfigSectionList
{
public:
    WvConf(const WvString &_filename);
    ~WvConf();
    
    WvConfigSection *operator[] (const WvString &s);

    int get(const WvString &section, const WvString &entry, int def_val);
    
    const char *get(const WvString &section, const WvString &entry,
		    const char *def_val = NULL);

    int fuzzy_get(WvStringList &sect, const WvString &entry,
		  int def_val);
    const char *fuzzy_get(WvStringList &sect, const WvString &entry,
			  const char *def_val = NULL);

    int fuzzy_get(WvStringList &sect, WvStringList &entry,
		  int def_val);
    const char *fuzzy_get(WvStringList & sect, WvStringList & ent,
			  const char *def_val = NULL);

    void set(const WvString &section, const WvString &entry,
	     const char *value);
    void set(const WvString &section, const WvString &entry, int value);

    void delete_section(const WvString &section);

    void flush();

    bool isok() const
    	{ return !error; }
    bool isclean() const
    	{ return isok() && !dirty; }

private:
    bool dirty;			// true if changed since last flush()
    bool error;			// true if something has gone wrong

    WvString filename;
    WvLog log;

    WvConfigSection globalsection;

    void load_file();
    char *parse_section(char *s);
    char *parse_value(char *s);
};


#endif // __WVCONF_H
