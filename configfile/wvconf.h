/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
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
#include "wvcallback.h"

class WvConf;


class WvConfigEntry
{
public:
    WvConfigEntry();
    WvConfigEntry(WvStringParm _name, WvStringParm _value);
    ~WvConfigEntry();
    
    void set(WvStringParm _value)
        { value = _value; }
    
    WvString name, value;
};


DeclareWvList(WvConfigEntry);


class WvConfigSection : public WvConfigEntryList
{
public:
    WvConfigSection(WvStringParm name);
    ~WvConfigSection();
    
    WvConfigEntry *operator[] (WvStringParm s);

    const char *get(WvStringParm entry, const char *def_val = NULL);
    void set(WvStringParm entry, WvStringParm value);
    void set(WvConfigEntry *e, WvStringParm value);
    
    // add an entry to the end of the section, _assuming_ no duplicates exist
    void quick_set(WvStringParm entry, WvStringParm value);

    void dump(WvStream &fp);

    WvString name;
};


// parameters are: userdata, section, entry, oldval, newval
DeclareWvCallback(5, void, WvConfCallback,
		  void *,
		  WvStringParm, WvStringParm,
		  WvStringParm, WvStringParm);

class WvConfCallbackInfo
{
public:
    WvConfCallback callback;
    void *userdata;
    const WvString section, entry;
    
    WvConfCallbackInfo(WvConfCallback _callback, void *_userdata,
		       WvStringParm _section, WvStringParm _entry)
	: callback(_callback), section(_section), entry(_entry)
        { userdata = _userdata; }
};


DeclareWvList(WvConfCallbackInfo);
DeclareWvList(WvConfigSection);

/**
 * WvConf configuration file management class: used to read/write config
 * files that are formatted in the style of Windows .ini files.
 */
class WvConf : public WvConfigSectionList
{
public:
    WvConf(WvStringParm _filename, int _create_mode = 0666);
    ~WvConf();
    
    bool isok() const
    	{ return !error; }
    bool isclean() const
    	{ return isok() && !dirty; }
    void save(WvStringParm filename);
    void save();
    void flush();

    WvConfigSection *operator[] (WvStringParm s);

    int parse_wvconf_request(char *request, char *&section, char *&entry,
			     char *&value);

    int getint(WvStringParm section, WvStringParm entry, int def_val);
    
    const char *get(WvStringParm section, WvStringParm entry,
		    const char *def_val = NULL);
    WvString getraw(WvString wvconfstr, int &parse_error);

    int fuzzy_getint(WvStringList &sect, WvStringParm entry,
		  int def_val);
    const char *fuzzy_get(WvStringList &sect, WvStringParm entry,
			  const char *def_val = NULL);

    int fuzzy_getint(WvStringList &sect, WvStringList &entry,
		  int def_val);
    const char *fuzzy_get(WvStringList & sect, WvStringList & ent,
			  const char *def_val = NULL);

    void setint(WvStringParm section, WvStringParm entry, int value);
    void set(WvStringParm section, WvStringParm entry,
	     const char *value);
    void setraw(WvString wvconfstr, const char *&value, int &parse_error);
    
    void maybesetint(WvStringParm section, WvStringParm entry,
		     int value);
    void maybeset(WvStringParm section, WvStringParm entry,
		  const char *value);

    void delete_section(WvStringParm section);

    // section and entry may be blank -- that means _all_ sections/entries!
    void add_callback(WvConfCallback callback, void *userdata,
		      WvStringParm section, WvStringParm entry);
    void del_callback(WvConfCallback callback, void *userdata,
		      WvStringParm section, WvStringParm entry);
    void run_callbacks(WvStringParm section, WvStringParm entry,
		       WvStringParm oldvalue, WvStringParm newvalue);
    void run_all_callbacks();
    
    // generic callback function for setting a bool to "true" when changed
    void setbool(void *userdata,
		 WvStringParm section, WvStringParm entry,
		 WvStringParm oldval, WvStringParm newval);
    
    void add_setbool(bool *b, WvStringParm section, WvStringParm entry)
        { add_callback(wvcallback(WvConfCallback, *this, WvConf::setbool),
		       b, section, entry); }
    void del_setbool(bool *b, WvStringParm section, WvStringParm entry)
        { del_callback(wvcallback(WvConfCallback, *this, WvConf::setbool),
		       b, section, entry); }
		    
    void load_file() // append the contents of the real config file
        { load_file(filename); }
    void load_file(WvStringParm filename); // append any config file

    // Gets a user's password and decrypts it.  This isn't defined in wvconf.cc.
    WvString get_passwd(WvStringParm sect, WvStringParm user);
    WvString get_passwd(WvStringParm user)
        { return get_passwd("Users", user); }

    // Encrypts and sets a user's password.  This isn't defined in wvconf.cc.
    void set_passwd(WvStringParm sect, WvStringParm user, WvStringParm passwd);
    void set_passwd(WvStringParm user, WvStringParm passwd)
        { set_passwd("Users", user, passwd); }

    // Converts all passwords to unencrypted format.  Not defined in wvconf.cc.
    void convert_to_old_pw();

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
