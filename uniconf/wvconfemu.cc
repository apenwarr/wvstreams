/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Basic WvConf emulation layer for UniConf.
 */
#include "wvconfemu.h"
#include "uniinigen.h"
#include "wvstringtable.h"
#include "wvfile.h"
#include "strutils.h"

//#define DEBUG_DEL_CALLBACK 1
#ifdef DEBUG_DEL_CALLBACK
#include <execinfo.h>
#endif

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


static void do_setbool(void *userdata,
		       WvStringParm section, WvStringParm key,
		       WvStringParm oldval, WvStringParm newval)
{
    bool* b = static_cast<bool*>(userdata);

    *b = true;
}


static void do_addname(void *userdata,
		       WvStringParm section, WvStringParm key,
		       WvStringParm oldval, WvStringParm newval)
{
    if (!!key)
        (*(WvStringList *)userdata).append(new WvString(key), true);
}


WvConfigEntryEmu *WvConfigSectionEmu::operator[] (WvStringParm s)
{
    WvConfigEntryEmu* entry = entries[s];

    if (uniconf[s].exists())
    {
	if (!entry)
	{
	    entry = new WvConfigEntryEmu(s, uniconf[s].getme());
	    entries.add(entry, true);
	}
	else
	    entry->value = uniconf[s].getme();
    }
    else
	entry = NULL;

    return entry;
}


const char *WvConfigSectionEmu::get(WvStringParm entry, const char *def_val)
{
    if (!entry)
	return def_val;

    WvString s(uniconf[entry].getme(def_val));
    
    // look it up in the cache
    WvString *sp = values[s];
    if (!sp) values.add(sp = new WvString(s), true);
    return sp->cstr();
}


void WvConfigSectionEmu::set(WvStringParm entry, WvStringParm value)
{
    if (!!entry)
    {
        if (!!value)
            uniconf[entry].setme(value);
        else
            uniconf[entry].setme(WvString::null);
    }
}


void WvConfigSectionEmu::quick_set(WvStringParm entry, WvStringParm value)
{
    uniconf[entry].setme(value);
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
    while (iter.next())
	if (!!iter->getme())
	{
	    /*
	     * FIXME: if the WvConfEmu is not at the root of the
	     * UniConf tree, this will give an incorrect result.
	     */
	    entry = sect[iter->fullkey(sect.uniconf)];
	    link.data = static_cast<void*>(entry);
	    assert(entry);
	    return &link;
	}

    return NULL;
}


WvLink *WvConfigSectionEmu::Iter::cur()
{
    return &link;
}


WvConfigEntryEmu *WvConfigSectionEmu::Iter::ptr() const
{
    return entry;
}


void *WvConfigSectionEmu::Iter::vptr() const
{
    return link.data;
}


void WvConfEmu::notify(const UniConf &_uni, const UniConfKey &_key)
{
    WvString section(_key.first());
    WvString key(_key.removefirst());

    if (hold)
	return;

    WvString value = uniconf[section][key].getme("");
    
    WvList<CallbackInfo>::Iter i(callbacks);
    for (i.rewind(); i.next(); )
    {
	if ((!i->section || !strcasecmp(i->section, section))
	    && (!i->key || !strcasecmp(i->key, key)))
	{
	    i->callback(i->userdata, section, key, WvString(), value);
	}
    }
}


WvConfEmu::WvConfEmu(const UniConf &_uniconf)
    : sections(42), hold(false), values(420), uniconf(_uniconf)
{
    wvauthd = NULL;
    uniconf.add_callback(this,
			 UniConfCallback(this, &WvConfEmu::notify),
			 true);
    dirty = false;
}


WvConfEmu::~WvConfEmu()
{
    // things will "work" if you don't empty the callback list before
    // deleting the WvConfEmu, but they probably won't work the way you
    // think they will. (ie. someone might be using a temporary WvConfEmu
    // and think his callbacks will stick around; they won't!)
#ifndef DEBUG_DEL_CALLBACK
    assert(callbacks.isempty());
#else
    if (!callbacks.isempty())
    {
	WvList<CallbackInfo>::Iter i(callbacks);

	fprintf(stderr, " *** leftover callbacks in WvConfEmu ***\n");
	for (i.rewind(); i.next(); )
	{
	    fprintf(stderr, "     - [%s]%s (%p)\n", i->section.cstr(), 
                    i->key.cstr(), i->cookie);
	}
    }
#endif

    uniconf.del_callback(this);
}


void WvConfEmu::zap()
{
    uniconf.remove();
}


bool WvConfEmu::isclean() const
{
    return isok() && !dirty;
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


void WvConfEmu::save(WvStringParm filename, int _create_mode)
{
    UniConfRoot tmp_uniconf(new UniIniGen(filename, _create_mode), false);

    uniconf.copy(tmp_uniconf, true);

    tmp_uniconf.commit();
}


void WvConfEmu::save()
{
    uniconf.commit();
}


void WvConfEmu::flush()
{
    uniconf.commit();
    dirty = false;
}


WvConfigSectionEmu *WvConfEmu::operator[] (WvStringParm sect)
{
    if (UniConfKey(sect).numsegments() != 1)
	return NULL;

    WvConfigSectionEmu* section = sections[sect];

    if (!section && uniconf[sect].exists())
    {
	section = new WvConfigSectionEmu(uniconf[sect], sect, &values);
	sections.add(section, true);
    }

    return section;
}


void WvConfEmu::add_callback(WvConfCallback callback, void *userdata,
			     WvStringParm section, WvStringParm key,
			     void *cookie)
{
    if (!callback)
	return;

    WvList<CallbackInfo>::Iter i(callbacks);
    for (i.rewind(); i.next(); )
    {
	if (i->cookie == cookie
	    && i->section == section
	    && i->key == key)
	    return;
    }

#ifdef DEBUG_DEL_CALLBACK
    void* trace[10];
    int count = backtrace(trace, sizeof(trace)/sizeof(trace[0]));
    char** tracedump = backtrace_symbols(trace, count);
    fprintf(stderr, "TRACE:add:%s:%s:%p", section.cstr(), key.cstr(), cookie);
    for (int i = 0; i < count; ++i)
	fprintf(stderr, ":%s", tracedump[i]);
    fprintf(stderr, "\n");
    free(tracedump);
#endif

    callbacks.append(new CallbackInfo(callback, userdata, section, key,
				      cookie),
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
	{
#ifdef DEBUG_DEL_CALLBACK
	    fprintf(stderr, "TRACE:del:%s:%s:%p\n", section.cstr(), key.cstr(), cookie);
#endif
	    i.xunlink();
	}
    }
}


void WvConfEmu::add_setbool(bool *b, WvStringParm _section, WvStringParm _key)
{
    add_callback(do_setbool, b, _section, _key, b);
}


void WvConfEmu::del_setbool(bool *b, WvStringParm _section, WvStringParm _key)
{
    del_callback(_section, _key, b);
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
    if (!section || !entry)
	return def_val;

    return uniconf[section][entry].getmeint(def_val);
}


const char *WvConfEmu::get(WvStringParm section, WvStringParm entry,
			   const char *def_val)
{
    if (!section || !entry)
	return def_val;

    WvString s(uniconf[section][entry].getme(def_val));
    
    // look it up in the cache
    WvString *sp = values[s];
    if (!sp) values.add(sp = new WvString(s), true);
    return sp->cstr();
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
    WvConfigSectionEmu *s;

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
    if (!!entry)
        uniconf[section][entry].setmeint(value);
}


void WvConfEmu::set(WvStringParm section, WvStringParm entry,
		    const char *value)
{
    if (!!entry)
    {
        if (value && value[0] != 0)
            uniconf[section][entry].setme(value);
        else
            uniconf[section][entry].setme(WvString::null);
        dirty = true;
    }
}


void WvConfEmu::maybesetint(WvStringParm section, WvStringParm entry,
			    int value)
{
    if (!!entry && !get(section, entry, NULL))
	setint(section, entry, value);
}


void WvConfEmu::maybeset(WvStringParm section, WvStringParm entry,
			 const char *value)
{
    if (!!entry && get(section, entry, 0) == 0)
	set(section, entry, value);
}


void WvConfEmu::delete_section(WvStringParm section)
{
    uniconf[section].remove();
    dirty = true;
}


int WvConfEmu::check_for_bool_string(const char *s)
{
    if (strcasecmp(s, "off") == 0
	|| strcasecmp(s, "false") == 0
	|| strncasecmp(s, "no", 2) == 0)   // also handles "none"
	return 0;

    if (strcasecmp(s, "on") == 0
	|| strcasecmp(s, "true") == 0
	|| strcasecmp(s, "yes") == 0)
	return 1;

    // not a special bool case, so just return the number
    return atoi(s);
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


WvConfigSectionEmu *WvConfEmu::Iter::ptr() const
{
    return conf[iter->key()];
}

