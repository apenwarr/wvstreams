/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Basic WvConf emulation layer for UniConf.
 */
#include "wvconfemu.h"
#include "wvstringtable.h"
#include "wvfile.h"
#include "strutils.h"


/*
 * Parse the WvConf string "request"; pointers to the found section,
 * entry, and value fields are stored in *section, *entry, and *value
 * respectively, and request[] is modified.
 * 
 * For example, the string:
 *     [silly]billy=willy
 * is parsed into:
 *     section="silly"; entry="billy"; value="willy";
 * 
 * Returns 0 on success, -1 if the command is missing the '[', -2 if
 * the string is missing a ']', or -3 if the section or entry is
 * blank.  If a "value" is not found (ie. there is no equal sign
 * outside the [] brackets) this does not qualify as an error, but
 * *value is set to NULL.
*/
static int parse_wvconf_request(char *request, char *&section,
				char *&entry, char *&value)
{
    entry = value = NULL;
    
    section = strchr(request, '[');
    if (!section)
	return -1;

    section++;
    
    entry = strchr(section, ']');
    if (!entry)
	return -2;

    *entry++ = 0;
    
    value = strchr(entry, '=');
    if (value)
    {
	*value++ = 0;
	value = trim_string(value);
    }
    
    section = trim_string(section);
    entry = trim_string(entry);
    
    if (!*section)
	return -3;
    
    return 0;
}


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


WvConfigEntryEmu *WvConfigSectionEmu::operator[] (WvStringParm s)
{
    WvConfigEntryEmu* entry = entries[s];

    if (uniconf[s].exists())
    {
	if (!entry)
	{
	    entry = new WvConfigEntryEmu(s, uniconf[s].get());
	    entries.add(entry, true);
	}
	else
	    entry->value = uniconf[s].get();
    }
    else
	entry = NULL;

    return entry;
}


const char *WvConfigSectionEmu::get(WvStringParm entry, const char *def_val)
{
    WvString *value = new WvString(uniconf[entry].get(def_val));
    values.add(value, true);
    return value->cstr();
}


void WvConfigSectionEmu::set(WvStringParm entry, WvStringParm value)
{
    if (!!value)
	uniconf[entry].set(value);
    else
	uniconf[entry].set(WvString::null);
}


void WvConfigSectionEmu::quick_set(WvStringParm entry, WvStringParm value)
{
    uniconf[entry].set(value);
}


bool WvConfigSectionEmu::isempty() const
{
    return !uniconf.haschildren();
}


WvConfigSectionEmu::Iter::~Iter()
{
}


void WvConfigSectionEmu::Iter::rewind()
{
    iter.rewind();
    link.data = entry = NULL;
}


WvLink *WvConfigSectionEmu::Iter::next()
{
    if (iter.next())
    {
	entry = sect[iter->key()];
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


void* WvConfigSectionEmu::Iter::vptr() const
{
    return link.data;
}


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
	    WvString value = uniconf[section][key].get("");
	    i->callback(i->userdata, section, key, i->last, value);
	    i->last = value;
	}
    }
}


WvConfEmu::WvConfEmu(const UniConf& _uniconf):
    uniconf(_uniconf), sections(42), hold(false)
{
    wvauthd = NULL;
    uniconf.add_callback(this,
			 UniConfCallback(this, &WvConfEmu::notify),
			 true);
}


void WvConfEmu::zap()
{
    uniconf.remove();
}


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


void WvConfEmu::save(WvStringParm filename)
{
    UniConfRoot tmp_uniconf(WvString("ini:%s", filename), false);

    uniconf.copy(tmp_uniconf, true);
}


void WvConfEmu::save()
{
    uniconf.commit();
}


void WvConfEmu::flush()
{
    uniconf.commit();
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
			     WvStringParm section, WvStringParm key,
			     void *cookie)
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
				      cookie, get(section, key, "")),
		     true);
}


void WvConfEmu::del_callback(WvStringParm section, WvStringParm key, void *cookie)
{
    WvList<CallbackInfo>::Iter i(callbacks);

    assert(cookie);

    for (i.rewind(); i.next(); )
    {
	if (i->cookie == cookie
	    && i->section == section
	    && i->key == key)
	    i.xunlink();
    }
}


void WvConfEmu::add_setbool(bool *b, WvStringParm _section, WvStringParm _key)
{
    add_callback(do_setbool, b, _section, _key, b);
}


void WvConfEmu::add_addname(WvStringList *list, WvStringParm sect, WvStringParm ent)
{
    add_callback(do_addname, list, sect, ent, list);
}


void WvConfEmu::del_addname(WvStringList *list,
			    WvStringParm sect, WvStringParm ent)
{
    del_callback(sect, ent, list);
}


void WvConfEmu::add_addfile(WvString *filename,
			    WvStringParm sect, WvStringParm ent)
{
    add_callback(do_addfile, filename, sect, ent, NULL);
}


WvString WvConfEmu::getraw(WvString wvconfstr, int &parse_error)
{
    char *section, *entry, *value;
    parse_error = parse_wvconf_request(wvconfstr.edit(),
				       section, entry, value);

    if (parse_error)
	return WvString();

    return get(section, entry, value);
}


int WvConfEmu::getint(WvStringParm section, WvStringParm entry, int def_val)
{
    return uniconf[section][entry].getint(def_val);
}


const char *WvConfEmu::get(WvStringParm section, WvStringParm entry,
			   const char *def_val)
{
    WvString *value = new WvString(uniconf[section][entry].get(def_val));
    values.add(value, true);
    return value->cstr();
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

void WvConfEmu::setraw(WvString wvconfstr, const char *&_value,
		       int &parse_error)
{
    char *section, *entry, *value;
    parse_error = parse_wvconf_request(wvconfstr.edit(),
				       section, entry, value);
    if (!parse_error)
    {
	set(section, entry, value);
	_value = get(section, entry, value);
    }
    else
	_value = NULL;
}


void WvConfEmu::setint(WvStringParm section, WvStringParm entry, int value)
{
    uniconf[section][entry].setint(value);
}


void WvConfEmu::set(WvStringParm section, WvStringParm entry,
		    const char *value)
{
    if (value && value[0] != 0)
	uniconf[section][entry].set(value);
    else
	uniconf[section][entry].set(WvString::null);
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
    uniconf[section].set(WvString::null);
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


void WvConfEmu::Iter::rewind()
{
    iter.rewind();
    link.data = NULL;
}


WvLink *WvConfEmu::Iter::next()
{
    if (iter.next())
    {
	link.data = static_cast<void*>(conf[iter->key()]);
	return &link;
    }

    return NULL;
}


WvConfigSectionEmu* WvConfEmu::Iter::ptr() const
{
    return conf[iter->key()];
}

