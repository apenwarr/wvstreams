/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2001 Net Integration Technologies, Inc.
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
#include "wvstringlist.h"

class WvConf;


class WvConfigEntry
{
public:
    WvConfigEntry();
    WvConfigEntry(const WvString &_name, const WvString &_value);
    ~WvConfigEntry();
    
    void set(const WvString &_value)
        { value = _value; value.unique(); }
    
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
    
    // add an entry to the end of the section, _assuming_ no duplicates exist
    void quick_set(const WvString &entry, const WvString &value);

    void dump(WvStream &fp);

    WvString name;
};


typedef void WvConfCallback(WvConf &cfg, void *userdata,
			    const WvString &section, const WvString &entry,
			    const WvString &oldval, const WvString &newval);

class WvConfCallbackInfo
{
public:
    WvConfCallback *callback;
    void *userdata;
    const WvString section, entry;
    
    WvConfCallbackInfo(WvConfCallback *_callback, void *_userdata,
		       const WvString &_section, const WvString &_entry)
	: section(_section), entry(_entry)
        { callback = _callback; userdata = _userdata; }
};


DeclareWvList(WvConfCallbackInfo);
DeclareWvList(WvConfigSection);


class WvConf : public WvConfigSectionList
{
public:
    WvConf(const WvString &_filename, int _create_mode = 0666);
    ~WvConf();
    
    bool isok() const
    	{ return !error; }
    bool isclean() const
    	{ return isok() && !dirty; }
    void save(const WvString &filename);
    void save();
    void flush();

    WvConfigSection *operator[] (const WvString &s);

    int getint(const WvString &section, const WvString &entry, int def_val);
    
    const char *get(const WvString &section, const WvString &entry,
		    const char *def_val = NULL);

    int fuzzy_getint(WvStringList &sect, const WvString &entry,
		  int def_val);
    const char *fuzzy_get(WvStringList &sect, const WvString &entry,
			  const char *def_val = NULL);

    int fuzzy_getint(WvStringList &sect, WvStringList &entry,
		  int def_val);
    const char *fuzzy_get(WvStringList & sect, WvStringList & ent,
			  const char *def_val = NULL);

    void setint(const WvString &section, const WvString &entry, int value);
    void set(const WvString &section, const WvString &entry,
	     const char *value);
    
    void maybesetint(const WvString &section, const WvString &entry,
		     int value);
    void maybeset(const WvString &section, const WvString &entry,
		  const char *value);

    void delete_section(const WvString &section);

    // section and entry may be blank -- that means _all_ sections/entries!
    void add_callback(WvConfCallback *callback, void *userdata,
		      const WvString &section, const WvString &entry);
    void del_callback(WvConfCallback *callback, void *userdata,
		      const WvString &section, const WvString &entry);
    void run_callbacks(const WvString &section, const WvString &entry,
		       const WvString &oldvalue, const WvString &newvalue);
    void run_all_callbacks();
    
    // generic callback function for setting a bool to "true" when changed
    static WvConfCallback setbool;
    void add_setbool(bool *b, const WvString &section, const WvString &entry)
        { add_callback(setbool, b, section, entry); }
    void del_setbool(bool *b, const WvString &section, const WvString &entry)
        { del_callback(setbool, b, section, entry); }
		    
    void load_file() // append the contents of the real config file
        { load_file(filename); }
    void load_file(const WvString &filename); // append any config file

private:
    bool dirty;			// true if changed since last flush()
    bool error;			// true if something has gone wrong
    bool loaded_once;		// true if load_file succeeded at least once
    int create_mode;		// if we must create config file

    WvString filename;
    WvLog log;

    WvConfigSection globalsection;
    WvConfCallbackInfoList callbacks;

    char *parse_section(char *s);
    char *parse_value(char *s);
};


#endif // __WVCONF_H
