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
#include "wvstringtable.h"


WvConfigEntry *WvConfigSectionEmu::operator[] (WvStringParm s)
{
    assert(false);
    return NULL;

    WvConfigEntryEmu* entry = entries[s];

    if (!entry && uniconf[s].exists())
    {
	entry = new WvConfigEntryEmu(s, uniconf[s].get());
	entries.add(entry, true);
    }

    return entry;
}


const char *WvConfigSectionEmu::get(WvStringParm entry, const char *def_val)
{
    return uniconf[entry].get(def_val);
}


#if 1
void WvConfigSectionEmu::set(WvStringParm entry, WvStringParm value)
{
    assert(false);
}
#endif


#if 1
void WvConfigSectionEmu::quick_set(WvStringParm entry, WvStringParm value)
{
    assert(false);
}
#endif


#if 1
bool WvConfigSectionEmu::isempty() const
{
    assert(false);
    return false;
}
#endif


#if 1
size_t WvConfigSectionEmu::count() const
{
    assert(false);
    return 0;
}
#endif


#if 1
void WvConfigSectionEmu::Iter::unlink()
{
    assert(false);
}
#endif


#if 1
void WvConfigSectionEmu::Iter::xunlink()
{
    assert(false);
}
#endif


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


#if 1
bool WvConfEmu::isclean() const
{
    assert(false);
    return false;
}
#endif


#if 1
bool WvConfEmu::isok() const
{
    assert(false);
    return false;
}
#endif


void WvConfEmu::load_file(WvStringParm filename)
{
    UniConfRoot new_uniconf(WvString("ini:%s", filename));

    hold = true;
    new_uniconf.copy(uniconf, true);
    hold = false;
}


#if 1
void WvConfEmu::save(WvStringParm filename)
{
    assert(false);
}
#endif


#if 1
void WvConfEmu::save()
{
    assert(false);
}
#endif


#if 1
void WvConfEmu::flush()
{
    assert(false);
}
#endif


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


#if 1
void WvConfEmu::del_callback(WvStringParm section, WvStringParm entry, void *cookie)
{
    assert(false);
}
#endif


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


#if 1
void WvConfEmu::add_addname(WvStringList *list, WvStringParm sect, WvStringParm ent)
{
    assert(false);
}
#endif


#if 1
void WvConfEmu::del_addname(WvStringList *list, WvStringParm sect, WvStringParm ent)
{
    assert(false);
}
#endif


#if 1
void WvConfEmu::add_addfile(WvString *filename, WvStringParm sect, WvStringParm ent)
{
    assert(false);
}
#endif


#if 1
WvString WvConfEmu::getraw(WvString wvconfstr, int &parse_error)
{
    assert(false);
    return "";
}
#endif


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
    WvString def_str(def_val);
    return check_for_bool_string(fuzzy_get(sect, entry, def_str));
}


const char *WvConfEmu::fuzzy_get(WvStringList &sect, WvStringParm entry,
				 const char *def_val)
{
    WvStringList::Iter i(sect);
    WvStringTable cache(5);
    WvConfigSection *s;

    for (i.rewind(); i.next(); )
    {
	for(s = (*this)[*i];
	    s && !cache[s->name];
	    s = (*s)["Inherits"] ? (*this)[(*s)["Inherits"]->value] : NULL)
	{
	    const char *ret = s->get(entry);
	    if (ret) return ret;
	    cache.add(&s->name, false);
	}
    }

    return def_val;
}


#if 1
void WvConfEmu::setraw(WvString wvconfstr, const char *&value, int &parse_error)
{
    assert(false);
}
#endif


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
    if (!get(section, entry, NULL))
	setint(section, entry, value);
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


#if 0
size_t WvConfEmu::count() const
{
    assert(false);
    return 0;
}
#endif


int WvConfEmu::check_for_bool_string(const char *s)
{
    if (strcasecmp(s, "off") == 0
	|| strcasecmp(s, "false") == 0
	|| strncasecmp(s, "no", 2) == 0)   // also handles "none"
	return (0);

    if (strcasecmp(s, "on") == 0
	|| strcasecmp(s, "true") == 0
	|| strcasecmp(s, "yes") == 0)
	return (1);

    // not a special bool case, so just return the number
    return (atoi(s));
}

