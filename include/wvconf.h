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


#ifdef __WVCONFEMU_H
#warning "disabling wvconfemu transparent emulation"
#undef WvConf
#undef WvConfigSection
#undef WvConfigSectionList
#undef WvConfigEntry
#undef WvConfigEntryList
#endif


class WvConf;


class WvConfigEntry
{
public:
    WvConfigEntry();
    WvConfigEntry(WvStringParm _name, WvStringParm _value);
    ~WvConfigEntry();
    
    void set(WvStringParm _value)
        { value = _value; }
    
    WvString name;
    WvString value;
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
typedef WvCallback<void, void*, WvStringParm, WvStringParm, WvStringParm, WvStringParm> WvConfCallback;


class WvConfCallbackInfo
{
public:
    WvConfCallback callback;
    void *userdata, *cookie;
    const WvString section, entry;
    
    WvConfCallbackInfo(WvConfCallback _callback, void *_userdata,
		       WvStringParm _section, WvStringParm _entry,
		       void *_cookie)
	: callback(_callback), section(_section), entry(_entry)
        { userdata = _userdata; cookie = _cookie; }
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

    static int check_for_bool_string(const char *s);
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
    // the 'cookie' is a random value that must be unique between all
    // registered callbacks on a particular key.  (Hint: maybe you should
    // use your 'this' pointer.)
    void add_callback(WvConfCallback callback, void *userdata,
		      WvStringParm section, WvStringParm entry, void *cookie);
    void del_callback(WvStringParm section, WvStringParm entry, void *cookie);
    void run_callbacks(WvStringParm section, WvStringParm entry,
		       WvStringParm oldvalue, WvStringParm newvalue);
    void run_all_callbacks();
    
    // generic callback function for setting a bool to "true" when changed
    void setbool(void *userdata,
		 WvStringParm section, WvStringParm entry,
		 WvStringParm oldval, WvStringParm newval);

    // generic callback for adding an entry name to name list when changed
    void addname(void *userdata,
		 WvStringParm section, WvStringParm entry,
		 WvStringParm oldval, WvStringParm newval);

    // generic callback to create a file with a one-line backup string
    void addfile(void *userdata,
                 WvStringParm section, WvStringParm entry,
                 WvStringParm oldval, WvStringParm newval);

    void add_addfile(WvString *filename, WvStringParm sect, WvStringParm ent)
	{ add_callback(WvConfCallback(this, &WvConf::addfile),
		       filename, sect, ent, new int); }

    void add_addname(WvStringList *list, WvStringParm sect, WvStringParm ent)
	{ add_callback(WvConfCallback(this, &WvConf::addname),
		       list, sect, ent, list); }
    void del_addname(WvStringList *list, WvStringParm sect, WvStringParm ent)
	{ del_callback(sect, ent, list); }
    
    void add_setbool(bool *b, WvStringParm section, WvStringParm entry)
        { add_callback(WvConfCallback(this, &WvConf::setbool),
		       b, section, entry, b); }
    void del_setbool(bool *b, WvStringParm section, WvStringParm entry)
        { del_callback(section, entry, b); }
		    
    void load_file() // append the contents of the real config file
        { load_file(filename); }
    void load_file(WvStringParm filename); // append any config file

    // Gets a user's password and decrypts it.  This isn't defined in wvconf.cc.
    WvString get_passwd(WvStringParm sect, WvStringParm user);
    WvString get_passwd(WvStringParm user)
        { return get_passwd("Users", user); }
    WvString get_passwd2(WvString pwenc);

    // Encrypts and sets a user's password.  This isn't defined in wvconf.cc.
    void set_passwd(WvStringParm sect, WvStringParm user, WvStringParm passwd);
    void set_passwd(WvStringParm user, WvStringParm passwd)
        { set_passwd("Users", user, passwd); }
    WvString set_passwd2(WvStringParm passwd);

    // Converts all passwords to unencrypted format.  Not defined in wvconf.cc.
    void convert_to_old_pw();

    // needed by wvfast_user_import
    void setdirty()
    { dirty = true; }

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
