/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Basic WvConf emulation layer for UniConf.
 */
#ifndef __WVCONFEMU_H
#define __WVCONFEMU_H


#ifndef USE_WVCONFEMU

#include "wvconf.h"

#else

#include "uniconfroot.h"

#define WvConf WvConfEmu
#define WvConfigSection WvConfigSectionEmu
#define WvConfigSectionList WvConfigSectionListEmu
#define WvConfigEntry WvConfigEntryEmu
#define WvConfigEntryList WvConfigEntryListEmu


class WvConfEmu;
class WvConfigEntryEmu;
class WvConfigSectionEmu;

typedef WvConfEmu WvConfigSectionListEmu;
typedef WvConfigSectionEmu WvConfigEntryListEmu;


class WvConfigEntryEmu
{
public:
    const WvString name;
    const WvString value;
    WvConfigEntryEmu(WvStringParm _name, WvStringParm _value):
	name(_name), value(_value)
    {}
};


DeclareWvDict(WvConfigEntryEmu, WvString, name);


class WvConfigSectionEmu
{
private:
    const UniConf uniconf;
    WvConfigEntryEmuDict entries;
public:
    const WvString name;
    WvConfigSectionEmu(const UniConf& _uniconf, WvStringParm _name):
	uniconf(_uniconf), entries(42), name(_name)
    {}

    WvConfigEntryEmu *operator[] (WvStringParm s);

    const char *get(WvStringParm entry, const char *def_val = NULL);
    void set(WvStringParm entry, WvStringParm value);
    void quick_set(WvStringParm entry, WvStringParm value);

    bool isempty() const;

    class Iter;
    friend class Iter;
};


DeclareWvDict(WvConfigSectionEmu, WvString, name);


class WvConfigSectionEmu::Iter
{
private:
    UniConf::Iter iter;
    WvLink link;
    WvConfigEntryEmu* entry;
public:
    Iter(WvConfigSectionEmu& _sect):
	iter(_sect.uniconf), link(NULL, false),
	entry(NULL)
    {}
    void rewind();
    WvLink *next();
    WvLink *cur();
    WvConfigEntryEmu* ptr() const;
    void unlink();
    void xunlink();
    WvIterStuff(WvConfigEntryEmu);
};


// parameters are: userdata, section, entry, oldval, newval
typedef WvCallback<void, void*, WvStringParm, WvStringParm, WvStringParm, WvStringParm> WvConfCallback;


class WvConfEmu
{
private:
    struct CallbackInfo
    {
	WvConfCallback callback;
	void* userdata;
	WvString section;
	WvString key;
	void* cookie;
	WvString last;
	CallbackInfo(WvConfCallback _callback, void* _userdata,
		     WvStringParm _section, WvStringParm _key,
		     void* _cookie, WvStringParm _last):
	    callback(_callback), userdata(_userdata), section(_section),
	    key(_key), cookie(_cookie), last(_last)
	{}
    };

    const UniConf uniconf;
    WvConfigSectionEmuDict sections;
    bool hold;
    WvList<CallbackInfo> callbacks;

    void notify(const UniConf &_uni, const UniConfKey &_key);
public:
    WvConfEmu(const UniConf& _uniconf);
    void zap();
    bool isok() const;
    void load_file(WvStringParm filename);
    void save(WvStringParm filename);
    void save();
    void flush();

    WvConfigSectionEmu *operator[] (WvStringParm sect);

    void add_callback(WvConfCallback callback, void *userdata,
		      WvStringParm section, WvStringParm key, void *cookie);
    void del_callback(WvStringParm section, WvStringParm key, void *cookie);

    void add_setbool(bool *b, WvStringParm _section, WvStringParm _key);

    void add_addname(WvStringList *list, WvStringParm sect, WvStringParm ent);
    void del_addname(WvStringList *list, WvStringParm sect, WvStringParm ent);

    void add_addfile(WvString *filename, WvStringParm sect, WvStringParm ent);

    WvString getraw(WvString wvconfstr, int &parse_error);
    int getint(WvStringParm section, WvStringParm entry, int def_val);
    const char *get(WvStringParm section, WvStringParm entry,
		    const char *def_val = NULL);
    int fuzzy_getint(WvStringList &sect, WvStringParm entry,
		  int def_val);
    const char *fuzzy_get(WvStringList &sect, WvStringParm entry,
			  const char *def_val = NULL);

    void setraw(WvString wvconfstr, const char *&value, int &parse_error);
    void setint(WvStringParm section, WvStringParm entry, int value);
    void set(WvStringParm section, WvStringParm entry,
	     const char *value);

    void maybesetint(WvStringParm section, WvStringParm entry,
		     int value);
    void maybeset(WvStringParm section, WvStringParm entry,
		  const char *value);

    void delete_section(WvStringParm section);

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

    static int check_for_bool_string(const char *s);

    class Iter;
    friend class Iter;
};


class WvConfEmu::Iter
{
    WvConfEmu& conf;
    UniConf::Iter iter;
    WvLink link;
public:
    Iter(WvConfEmu& _conf):
	conf(_conf), iter(conf.uniconf), link(NULL, false)
    {}
    void rewind()
    {
	iter.rewind();
    }
    WvLink *next()
    {
	if (iter.next())
	{
	    link.data = static_cast<void*>(conf[iter->key()]);
	    return &link;
	}

	return NULL;
    }
    WvConfigSectionEmu* ptr() const
    {
	return conf[iter->key()];
    }
    WvIterStuff(WvConfigSectionEmu);
};


#endif /* USE_WVCONFEMU */

#endif // __WVCONFEMU_H
