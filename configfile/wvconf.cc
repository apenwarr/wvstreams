/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Implementation of the WvConfigFile class.
 *
 * Created:     Sept 12 1997            D. Coombs
 *
 */
#include "wvconf.h"
#include "wvfile.h"
#include "wvstringtable.h"
#include <string.h>
#include <sys/stat.h>


void WvConf::setbool(void *userdata,
		     WvStringParm sect, WvStringParm ent,
		     WvStringParm oldval, WvStringParm newval)
{
    if (!*(bool *)userdata)
    {
	WvLog log("Config Event", WvLog::Debug);
	if(sect == "Tunnel Vision" && ent == "Magic Password")
  	    log("Changed:[%s]%s\n",sect, ent);
	else
	    log("Changed: [%s]%s = '%s' -> '%s'\n", sect, ent, oldval, newval);
    }
    
    *(bool *)userdata = true;
}

void WvConf::addname(void *userdata,
		     WvStringParm sect, WvStringParm ent,
		     WvStringParm oldval, WvStringParm newval)
{
    (*(WvStringList *)userdata).append(new WvString(ent), true);
}


void WvConf::addfile(void *userdata,
                     WvStringParm sect, WvStringParm ent,
                     WvStringParm oldval, WvStringParm newval)
{
    WvFile tmp(WvString("/home/%s/%s", ent, *(WvString *)userdata), 
               O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if(tmp.isok())
    {
        if(!!newval)
            tmp.print("%s\n", newval);
        else
            tmp.print("%s\n", ent);
    }
}

WvConf::WvConf(WvStringParm _filename, int _create_mode)
	: filename(_filename), log(filename), globalsection("")
{
    create_mode = _create_mode;
    dirty = error = loaded_once = false;
    wvauthd = NULL;
    load_file();
}


int WvConf::check_for_bool_string(const char *s)
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

// parse the WvConf string "request"; pointers to the found section,
// entry, and value fields are stored in *section, *entry, and *value
// respectively, and request[] is modified.
//
// For example, the string:
//         [silly]billy=willy
// is parsed into:
//         section="silly"; entry="billy"; value="willy";
//
// Returns 0 on success, -1 if the command is missing the '[', -2 if the
// string is missing a ']', or -3 if the section or entry is blank.  If a
// "value" is not found (ie. there is no equal sign outside the [] brackets)
// this does not qualify as an error, but *value is set to NULL.
//
int WvConf::parse_wvconf_request(char *request, char *&section,
				 char *&entry, char *&value)
{
    //printf("parsing %s\n", request);
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
    
    //printf("section: %s\nentry: %s\n", section, entry);
    section = trim_string(section);
    entry = trim_string(entry);
    
    if (!*section)
	return -3;
    
    return 0;
}


// This "int" version of get is smart enough to interpret words like on/off,
// true/false, and yes/no.
int WvConf::getint(WvStringParm section, WvStringParm entry, int def_val)
{
    WvString def_str(def_val);
    return check_for_bool_string(get(section, entry, def_str));
}


// This "int" version of fuzzy_get is smart enough to interpret words like 
// on/off, true/false, and yes/no.
int WvConf::fuzzy_getint(WvStringList &section, WvStringList &entry,
			 int def_val)
{
    WvString def_str(def_val);
    return check_for_bool_string(fuzzy_get(section, entry, def_str));
}


// This "int" version of fuzzy_get is smart enough to interpret words like 
// on/off, true/false, and yes/no.
int WvConf::fuzzy_getint(WvStringList &section, WvStringParm entry,
			 int def_val)
{
    WvString def_str(def_val);
    return check_for_bool_string(fuzzy_get(section, entry, def_str));
}


void WvConf::setint(WvStringParm section, WvStringParm entry, int value)
{
    WvString def_str(value);
    set(section, entry, def_str);
}


// only set the value if it isn't already in the config file
void WvConf::maybesetint(WvStringParm section, WvStringParm entry,
			 int value)
{
    if (!get(section, entry, NULL))
	setint(section, entry, value);
}

 
void WvConf::load_file(WvStringParm filename)
{
    const char *p;
    char *from_file;
    WvConfigSection *sect = &globalsection;
    bool quick_mode = false;

    WvFile file(filename, O_RDONLY);

    #ifdef _WIN32
    //FIXME: Windows doesn't have a sticky bit so we can't use that to signal other processes that
    //  the file is being written to. Just be careful :).
    #else
    // check the sticky bit and fail if set
    struct stat statbuf;
    if (file.isok() && fstat(file.getrfd(), &statbuf) == -1)
    {
	log(WvLog::Warning, "Can't stat config file %s\n", filename);
	file.close();
    }

    if (file.isok() && (statbuf.st_mode & S_ISVTX))
    {
	file.close();
	file.seterr(EAGAIN);
    }
    #endif

    if (!file.isok())
    {
	// Could not open for read.
	// ...actually, this warning is mainly just annoying.
        //log(loaded_once ? WvLog::Debug1 : WvLog::Warning,
	//    "Can't read config file %s: %s\n", filename, file.errstr());
	if (file.geterr() != ENOENT && !loaded_once)
	    error = true;
	return;
    }

    while ((from_file = trim_string(file.getline())) != NULL)
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
    
    run_all_callbacks();

    loaded_once = true;
}


WvConf::~WvConf()
{
    // We don't really have to do anything here.  sections's destructor
    // will go through and delete all its entries, so we should be fine.

    flush();
}


const char *WvConf::get(WvStringParm section, WvStringParm entry,
			const char *def_val)
{
    WvStringTable cache(5);
    WvConfigSection *s;
    
    for(s = (*this)[section];
	s && !cache[s->name];
	s = (*s)["Inherits"] ? (*this)[(*s)["Inherits"]->value] : NULL)
    {
	const char *ret = s->get(entry);
	if (ret) return ret;
	cache.add(&s->name, false);
    }

    return globalsection.get(entry, def_val);
}


// Gets an entry, given a string in the form [section]entry=value.  Returns
// the value or NULL if not found.  The parameter parse_error is set to the
// return value of parse_wvconf_request.
WvString WvConf::getraw(WvString wvconfstr, int &parse_error)
{
    char *section, *entry, *value;
    parse_error = parse_wvconf_request(wvconfstr.edit(),
				       section, entry, value);

    if (parse_error)
	return WvString();

    return get(section, entry, value);
}


const char *WvConf::fuzzy_get(WvStringList &sections, WvStringList &entries,
			      const char *def_val)
{
    WvStringList::Iter i(sections), i2(entries);
    WvStringTable cache(5);
    WvConfigSection *s;

    for (i.rewind(); i.next(); )
    {
	for (i2.rewind(); i2.next();)
	{
	    for(s = (*this)[*i];
		s && !cache[s->name];
		s = (*s)["Inherits"] ? (*this)[(*s)["Inherits"]->value] : NULL)
	    {
		const char *ret = s->get(*i2);
		if (ret) return ret;
		cache.add(&s->name, false);
	    }
	}
    }
    
    return def_val;
}


const char *WvConf::fuzzy_get(WvStringList &sections, WvStringParm entry,
			      const char *def_val)
{
    WvStringList::Iter i(sections);
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


void WvConf::set(WvStringParm section, WvStringParm entry,
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
    {
	run_callbacks(section, entry, oldval, value);

	/* fprintf(stderr, "cfg.set: set [%s]%s = %s\n",
		(const char *)section, (const char *)entry,
		(const char *)value ?: "!!!"); */
    
	s->set(entry, value);
	dirty = true;
    }
}


// Takes a string in the form [section]entry=value and sets it.  Returns an
// error code as defined in parse_wvconf_request.  The value parameter is
// also set to the value (useful in rcommand, when we display the value after
// it has been set).
void WvConf::setraw(WvString wvconfstr, const char *&xvalue, int &parse_error)
{
    char *section, *entry, *value;
    parse_error = parse_wvconf_request(wvconfstr.edit(),
				       section, entry, value);
    if (!parse_error)
    {
	set(section, entry, value);
	xvalue = get(section, entry, value);
    }
    else
	xvalue = NULL;
}


// only set the value if it isn't already in the config file
void WvConf::maybeset(WvStringParm section, WvStringParm entry,
		      const char *value)
{
    if (value && !get(section, entry, NULL))
	set(section, entry, value);
}


WvConfigSection *WvConf::operator[] (WvStringParm section)
{
    Iter i(*this);

    if (section)
	for (i.rewind(); i.next(); )
	{
	    if (strcasecmp(i().name, section) == 0)
		return &i();
	}

    return NULL;
}


void WvConf::delete_section(WvStringParm section)
{
    WvConfigSection *s = (*this)[section];
    if (s)
    {
	unlink(s);
	dirty = true;
    }
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


void WvConf::save(WvStringParm _filename)
{
    if (error || !_filename)
	return;
    
    WvFile fp(_filename, O_WRONLY|O_CREAT|O_TRUNC, create_mode);

    if (!fp.isok())
    {
	log(WvLog::Error, "Can't write to config file %s: %s\n",
	    _filename, strerror(errno));
	if (fp.geterr() != ENOENT)
	    error = true;
	return;
    }

    #ifdef _WIN32
    //FIXME: Windows doesn't have a sticky bit so we can't use that to signal other processes that
    //  the file is being written to. Just be careful :).
    #else
    struct stat statbuf;
    if (fstat(fp.getwfd(), &statbuf) == -1)
    {
	log(WvLog::Error, "Can't stat config file %s: %s\n",
	    _filename, strerror(errno));
	error = true;
	return;
    }

    fchmod(fp.getwfd(), (statbuf.st_mode & 07777) | S_ISVTX);
    #endif

    globalsection.dump(fp);
    
    Iter i(*this);
    for (i.rewind(); i.next();)
    {
	WvConfigSection & sect = *i;
	fp.print("\n[%s]\n", sect.name);
	sect.dump(fp);
    }

    #ifdef _WIN32
    //FIXME: Windows doesn't have a sticky bit so we can't use that to signal other processes that
    //  the file is being written to. Just be careful :).
    #else
    fchmod(fp.getwfd(), statbuf.st_mode & 07777);
    #endif
}


void WvConf::save()
{
    save(filename);
}


// only save the config file if it's dirty
void WvConf::flush()
{
    if (!dirty || error)
	return;
    
    // save under default filename
    save(filename);
    
    dirty = false;
}


void WvConf::add_callback(WvConfCallback callback, void *userdata,
			  WvStringParm section, WvStringParm entry,
			  void *cookie)
{
    callbacks.append(new WvConfCallbackInfo(callback, userdata,
					    section, entry, cookie), true);
}


void WvConf::del_callback(WvStringParm section, WvStringParm entry,
			  void *cookie)
{
    WvConfCallbackInfoList::Iter i(callbacks);
    
    for (i.rewind(); i.next(); )
    {
	if (i->cookie == cookie && i->section == section && i->entry == entry)
	{
	    i.unlink();
	    return;
	}
    }
}


void WvConf::run_callbacks(WvStringParm section, WvStringParm entry,
			   WvStringParm oldvalue, WvStringParm newvalue)
{
    WvConfCallbackInfoList::Iter i(callbacks);
    
    for (i.rewind(); i.next(); )
    {
	if (!i->section || !strcasecmp(i->section, section))
	{
	    if (!i->entry || !strcasecmp(i->entry, entry))
		i->callback(i->userdata, section, entry,
			    oldvalue, newvalue);
	}
    }
}


void WvConf::run_all_callbacks()
{
    WvConfCallbackInfoList::Iter i(callbacks);

    for (i.rewind(); i.next(); )
        i->callback(i->userdata, "", "", "", "");
}
