/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998, 1999 Worldvisions Computer Technology, Inc.
 *
 * Implementation of the WvConfigFile class.
 *
 * Created:     Sept 12 1997            D. Coombs
 *
 */
#include "wvconf.h"
#include "wvstream.h"
#include <string.h>
#include <unistd.h>


void WvConf::setbool(WvConf &, void *userdata,
		     const WvString &, const WvString &,
		     const WvString &, const WvString &)
{
    *(bool *)userdata = true;
}
		     


WvConf::WvConf(const WvString &_filename, int _create_mode)
	: filename(_filename), log(filename), globalsection("")
{
    create_mode = _create_mode;
    filename.unique();
    dirty = error = false;
    load_file();
}


static int check_for_bool_string(const char *s)
{
    if (strcasecmp(s, "off") == 0
     || strcasecmp(s, "false") == 0
     || strncasecmp(s, "no", 2) == 0)	// also handles "none"
	return (0);

    if (strcasecmp(s, "on") == 0
     || strcasecmp(s, "true") == 0
     || strcasecmp(s, "yes") == 0)
	return (1);

    // not a special bool case, so just return the number
    return (atoi(s));
}


// This "int" version of get is smart enough to interpret words like on/off,
// true/false, and yes/no.
int WvConf::get(const WvString &section, const WvString &entry, int def_val)
{
    WvString def_str(def_val);
    return check_for_bool_string(get(section, entry, def_str));
}


// This "int" version of fuzzy_get is smart enough to interpret words like 
// on/off, true/false, and yes/no.
int WvConf::fuzzy_get(WvStringList &section, WvStringList &entry,
		      int def_val)
{
    WvString def_str(def_val);
    return check_for_bool_string(fuzzy_get(section, entry, def_str));
}


// This "int" version of fuzzy_get is smart enough to interpret words like 
// on/off, true/false, and yes/no.
int WvConf::fuzzy_get(WvStringList &section, const WvString &entry,
		      int def_val)
{
    WvString def_str(def_val);
    return check_for_bool_string(fuzzy_get(section, entry, def_str));
}


void WvConf::set(const WvString &section, const WvString &entry, int value)
{
    WvString def_str(value);
    set(section, entry, def_str);
}

 
void WvConf::load_file()
{
    WvFile file;
    char *p;
    char *from_file;
    WvConfigSection *sect = &globalsection;
    bool quick_mode = false;

    file.open(filename, O_RDONLY);
    if (!file.isok())
    {
	// Could not open for read.
        log(WvLog::Warning, "Can't read config file: %s\n", file.errstr());
	if (file.geterr() != ENOENT)
	    error = true;
	return;
    }

    while ((from_file = trim_string(file.getline(0))) != NULL)
    {
	if ((p = parse_section(from_file)) != NULL)
	{
	    quick_mode = false;
	    
	    // a new section?
	    if (!p[0])		// blank name: global section
		sect = &globalsection;
	    else
	    {
		sect = (*this)[p];
		if (!sect)
		{
		    sect = new WvConfigSection(p);
		    append(sect, true);
		    quick_mode = true;
		}
	    }
	}
	else
	{
	    // it must be an element for the current section *sect.
	    p = parse_value(from_file);
	    if (!p)
		p = "";		// allow empty entries

	    from_file = trim_string(from_file);
	    if (from_file[0])	// nonblank option name
	    {
		if (quick_mode)
		    sect->quick_set(from_file, p);
		else
		    sect->set(from_file, p);
	    }
	}
    }
}


WvConf::~WvConf()
{
    // We don't really have to do anything here.  sections's destructor
    // will go through and delete all its entries, so we should be fine.

    flush();
}


const char *WvConf::get(const WvString &section, const WvString &entry,
			const char *def_val)
{
    WvConfigSection *s = (*this)[section];
    if (!s) return def_val;
    
    WvConfigEntry *e = (*s)[entry];
    if (!e)
	return globalsection.get(entry, def_val);
    else
	return e->value;
}


const char *WvConf::fuzzy_get(WvStringList &sections, WvStringList &entries,
			      const char *def_val)
{
    WvStringList::Iter i(sections), i2(entries);

    for (i.rewind(); i.next(); )
    {
	WvConfigSection *s = (*this)[i];
	if (!s) continue; // no such section

	for (i2.rewind(); i2.next();)
	{
	    WvConfigEntry *e = (*s)[i];
	    if (e) return e->value;
	}
    }
    
    return def_val;
}


const char *WvConf::fuzzy_get(WvStringList &sections, const WvString &entry,
			      const char *def_val)
{
    WvStringList::Iter i(sections);

    for (i.rewind(); i.next(); )
    {
	WvConfigSection *s = (*this)[i];
	if (!s) continue;
	
	WvConfigEntry *e = (*s)[entry];
	if (e) return e->value;
    }

    return def_val;
}


void WvConf::set(const WvString &section, const WvString &entry,
		 const char *value)
{
    WvConfigSection *s = (*this)[section];
    
    // section does not exist yet
    if (!s)
    {
	if (!value || !value[0])
	    return; // no section, no entry, no problem!
	
	s = new WvConfigSection(section);
	append(s, true);
    }
    
    const char *oldval = s->get(entry, "");
    if (!value) value = "";
    if (strcmp(oldval, value)) // case sensitive
	run_callbacks(section, entry, oldval, value);
    s->set(entry, value);
    dirty = true;
}


WvConfigSection *WvConf::operator[] (const WvString &section)
{
    Iter i(*this);

    // otherwise, search the whole list.
    for (i.rewind(); i.next(); )
    {
	if (strcasecmp(i().name, section) == 0)
	    return &i();
    }

    return NULL;
}


void WvConf::delete_section(const WvString &section)
{
    WvConfigSection *s = (*this)[section];
    if (s)
	unlink(s);
    dirty = true;
}


char *WvConf::parse_section(char *s)
{
    char *q;

    if (s[0] != '[')
	return (NULL);

    q = strchr(s, ']');
    if (!q || q[1])
	return (NULL);

    *q = 0;
    return trim_string(s + 1);
}


char *WvConf::parse_value(char *s)
{
    char *q;

    q = strchr(s, '=');
    if (q == NULL)
	return (NULL);

    *q++ = 0;			// 's' points to option name, 'q' points to value

    return (trim_string(q));
}


void WvConf::flush()
{

    if (!dirty || error)
	return;
    
    WvFile fp(filename, O_WRONLY|O_CREAT|O_TRUNC, create_mode);

    if (!fp.isok())
    {
	log(WvLog::Error, "Can't write to config file: %s\n",
	    strerror(errno));
	error = true;
	return;
    }
    
    globalsection.dump(fp);
    
    Iter i(*this);
    for (i.rewind(); i.next();)
    {
	WvConfigSection & sect = i;
	fp.print("\n[%s]\n", sect.name);
	sect.dump(fp);
    }
    
    dirty = false;
}


void WvConf::add_callback(WvConfCallback *callback, void *userdata,
			  const WvString &section, const WvString &entry)
{
    callbacks.append(new WvConfCallbackInfo(callback, userdata,
					    section, entry), true);
}


void WvConf::del_callback(WvConfCallback *callback, void *userdata,
			  const WvString &section, const WvString &entry)
{
    WvConfCallbackInfoList::Iter i(callbacks);
    
    for (i.rewind(); i.next(); )
    {
	WvConfCallbackInfo &c(i);
	
	if (c.callback == callback && c.userdata == userdata
	    && c.section == section && c.entry == entry)
	{
	    i.unlink();
	    return;
	}
    }
}


void WvConf::run_callbacks(const WvString &section, const WvString &entry,
			   const WvString &oldvalue, const WvString &newvalue)
{
    WvConfCallbackInfoList::Iter i(callbacks);
    
    for (i.rewind(); i.next(); )
    {
	if (!i().section || !strcasecmp(i().section, section))
	{
	    if (!i().entry || !strcasecmp(i().entry, entry))
		i().callback(*this, i().userdata, section, entry,
			     oldvalue, newvalue);
	}
    }
}
