/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Basic WvConf emulation layer for UniConf.
 */
#ifndef USE_WVCONFEMU
#define USE_WVCONFEMU
#endif
#include "wvconfemu.h"


WvConfigEntry *WvConfigSectionEmu::operator[] (WvStringParm s)
{
    assert(false);
    return NULL;
}


const char *WvConfigSectionEmu::get(WvStringParm entry, const char *def_val)
{
    assert(false);
    return false;
}


void WvConfigSectionEmu::set(WvStringParm entry, WvStringParm value)
{
    assert(false);
}


void WvConfigSectionEmu::quick_set(WvStringParm entry, WvStringParm value)
{
    assert(false);
}


bool WvConfigSectionEmu::isempty() const
{
    assert(false);
    return false;
}


size_t WvConfigSectionEmu::count() const
{
    assert(false);
    return 0;
}


void WvConfigSectionEmu::Iter::unlink()
{
    assert(false);
}


void WvConfigSectionEmu::Iter::xunlink()
{
    assert(false);
}


void WvConfEmu::notify(const UniConf &_uni, const UniConfKey &_key)
{
    WvList<SetBool>::Iter i(setbools);
    WvString section(_key.first());
    WvString key(_key.removefirst());

    if (hold)
	return;

    i.rewind();
    while (i.next())
	if (((i->section && !i->section) || !strcasecmp(i->section, section))
	    && ((i->key && !i->key) || !strcasecmp(i->key, key)))
	    *(i->b) = true;
}


WvConfEmu::WvConfEmu(const UniConf& _uniconf):
    uniconf(_uniconf), sections(42), hold(false)
{
    uniconf.add_callback(this,
			 UniConfCallback(this, &WvConfEmu::notify),
			 true);
}


void WvConfEmu::zap()
{
    uniconf.remove();
}


bool WvConfEmu::isclean() const
{
    assert(false);
    return false;
}

bool WvConfEmu::isok() const
{
    assert(false);
    return false;
}


void WvConfEmu::load_file(WvStringParm filename)
{
    UniConfRoot new_uniconf(WvString("ini:%s", filename));

    hold = true;
    new_uniconf.copy(uniconf, true);
    hold = false;
}


void WvConfEmu::save(WvStringParm filename)
{
    assert(false);
}


void WvConfEmu::save()
{
    assert(false);
}


void WvConfEmu::flush()
{
    assert(false);
}


WvConfigSectionEmu *WvConfEmu::operator[] (WvStringParm sect)
{
    WvConfigSectionEmu* section = sections[sect];

    if (!section && uniconf[sect].exists())
    {
	section = new WvConfigSectionEmu(uniconf[sect], sect);
	sections.add(section, true);
    }

    return section;
}


void WvConfEmu::add_callback(WvConfCallback callback, void *userdata,
		  WvStringParm section, WvStringParm entry, void *cookie)
{
    assert(false);
}


void WvConfEmu::del_callback(WvStringParm section, WvStringParm entry, void *cookie)
{
    assert(false);
}


void WvConfEmu::add_setbool(bool *b, WvStringParm _section, WvStringParm _key)
{
    WvList<SetBool>::Iter i(setbools);

    i.rewind();
    while (i.next())
    {
	if (i->b == b
	    && i->section == _section
	    && i->key == _key)
	    return;
    }

    setbools.append(new SetBool(b, _section, _key), true);
}


void WvConfEmu::add_addname(WvStringList *list, WvStringParm sect, WvStringParm ent)
{
    assert(false);
}


void WvConfEmu::del_addname(WvStringList *list, WvStringParm sect, WvStringParm ent)
{
    assert(false);
}


void WvConfEmu::add_addfile(WvString *filename, WvStringParm sect, WvStringParm ent)
{
    assert(false);
}


WvString WvConfEmu::getraw(WvString wvconfstr, int &parse_error)
{
    assert(false);
    return "";
}


int WvConfEmu::getint(WvStringParm section, WvStringParm entry, int def_val)
{
    return uniconf[section][entry].getint(def_val);
}


const char *WvConfEmu::get(WvStringParm section, WvStringParm entry,
			   const char *def_val)
{
    return uniconf[section][entry].get(def_val);
}


int WvConfEmu::fuzzy_getint(WvStringList &sect, WvStringParm entry,
			    int def_val)
{
    assert(false);
    return 0;
}


const char *WvConfEmu::fuzzy_get(WvStringList &sect, WvStringParm entry,
				 const char *def_val)
{
    assert(false);
    return NULL;
}


void WvConfEmu::setraw(WvString wvconfstr, const char *&value, int &parse_error)
{
    assert(false);
}


void WvConfEmu::setint(WvStringParm section, WvStringParm entry, int value)
{
    uniconf[section][entry].setint(value);
}


void WvConfEmu::set(WvStringParm section, WvStringParm entry,
		    const char *value)
{
    uniconf[section][entry].set(value);
}


void WvConfEmu::maybesetint(WvStringParm section, WvStringParm entry,
			    int value)
{
    assert(false);
}


void WvConfEmu::maybeset(WvStringParm section, WvStringParm entry,
			 const char *value)
{
    if (get(section, entry, 0) == 0)
	set(section, entry, value);
}


void WvConfEmu::delete_section(WvStringParm section)
{
    assert(false);
}


size_t WvConfEmu::count() const
{
    assert(false);
    return 0;
}


int WvConfEmu::check_for_bool_string(const char *s)
{
    assert(false);
    return 0;
}

