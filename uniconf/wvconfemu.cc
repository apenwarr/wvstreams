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
#include "wvfile.h"
#include "strutils.h"


static void do_setbool(void* userdata,
		       WvStringParm section, WvStringParm key,
		       WvStringParm oldval, WvStringParm newval)
{
    bool* b = static_cast<bool*>(userdata);

    *b = true;
}


static void do_addname(void* userdata,
		       WvStringParm section, WvStringParm key,
		       WvStringParm oldval, WvStringParm newval)
{
    (*(WvStringList *)userdata).append(new WvString(key), true);
}


static void do_addfile(void* userdata,
		       WvStringParm section, WvStringParm key,
		       WvStringParm oldval, WvStringParm newval)
{
    WvFile tmp(WvString("/home/%s/%s", key, *(WvString *)userdata), 
               O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if(tmp.isok())
    {
        if(!!newval)
            tmp.print("%s\n", newval);
        else
            tmp.print("%s\n", key);
    }
}


WvConfigEntry *WvConfigSectionEmu::operator[] (WvStringParm s)
{
    assert(false && "not implemented");
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


void WvConfigSectionEmu::set(WvStringParm entry, WvStringParm value)
{
    uniconf[entry].set(value);
}


#if 1
void WvConfigSectionEmu::quick_set(WvStringParm entry, WvStringParm value)
{
    assert(false && "not implemented");
}
#endif


bool WvConfigSectionEmu::isempty() const
{
    return !uniconf.haschildren();
}


void WvConfigSectionEmu::Iter::rewind()
{
    iter.rewind();
}


WvLink *WvConfigSectionEmu::Iter::next()
{
    if (iter.next())
    {
	entry = new WvConfigEntryEmu(iter->key(), iter->get());
	link.data = static_cast<void*>(entry);
	return &link;
    }

    return NULL;
}


WvLink *WvConfigSectionEmu::Iter::cur()
{
    return &link;
}


WvConfigEntryEmu* WvConfigSectionEmu::Iter::ptr() const
{
    return entry;
}


#if 1
void WvConfigSectionEmu::Iter::unlink()
{
    assert(false && "not implemented");
}
#endif


#if 1
void WvConfigSectionEmu::Iter::xunlink()
{
    assert(false && "not implemented");
}
#endif


void WvConfEmu::notify(const UniConf &_uni, const UniConfKey &_key)
{
    WvList<CallbackInfo>::Iter i(callbacks);
    WvString section(_key.first());
    WvString key(_key.removefirst());

    if (hold)
	return;

    for (i.rewind(); i.next(); )
    {
	if (((i->section && !i->section) || !strcasecmp(i->section, section))
	    && ((i->key && !i->key) || !strcasecmp(i->key, key)))
	{
	    WvString value = get(section, key, NULL);
	    i->callback(i->userdata, section, key, i->last, value);
	    i->last = value;
	}
    }
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
    assert(false && "not implemented");
    return false;
}
#endif


bool WvConfEmu::isok() const
{
    return !uniconf.isnull();
}


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
    assert(false && "not implemented");
}
#endif


#if 1
void WvConfEmu::save()
{
    assert(false && "not implemented");
}
#endif


#if 1
void WvConfEmu::flush()
{
    assert(false && "not implemented");
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
		  WvStringParm section, WvStringParm key, void *cookie)
{
    WvList<CallbackInfo>::Iter i(callbacks);

    if (!callback)
	return;

    for (i.rewind(); i.next(); )
    {
	if (i->cookie == cookie
	    && i->section == section
	    && i->key == key)
	    return;
    }

    callbacks.append(new CallbackInfo(callback, userdata, section, key,
				      cookie, get(section, key, NULL)),
		     true);
}


#if 1
void WvConfEmu::del_callback(WvStringParm section, WvStringParm entry, void *cookie)
{
    assert(false && "not implemented");
    assert(cookie);
}
#endif


void WvConfEmu::add_setbool(bool *b, WvStringParm _section, WvStringParm _key)
{
    add_callback(do_setbool, b, _section, _key, b);
}


void WvConfEmu::add_addname(WvStringList *list, WvStringParm sect, WvStringParm ent)
{
    add_callback(do_addname, list, sect, ent, list);
}


#if 1
void WvConfEmu::del_addname(WvStringList *list,
			    WvStringParm sect, WvStringParm ent)
{
    assert(false && "not implemented");
}
#endif


#if 1
void WvConfEmu::add_addfile(WvString *filename,
			    WvStringParm sect, WvStringParm ent)
{
    add_callback(do_addfile, filename, sect, ent, NULL);
}
#endif


#if 1
WvString WvConfEmu::getraw(WvString wvconfstr, int &parse_error)
{
    assert(false && "not implemented");
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
    assert(false && "not implemented");
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
    assert(false && "not implemented");
}


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

