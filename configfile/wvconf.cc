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
		     WvStringParm , WvStringParm ,
		     WvStringParm , WvStringParm )
{
    *(bool *)userdata = true;
}
		     


WvConf::WvConf(WvStringParm _filename, int _create_mode)
	: filename(_filename), log(filename), globalsection("")
{
    create_mode = _create_mode;
    filename.unique();
    dirty = error = loaded_once = false;
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
    WvFile file;
    char *p;
    char *from_file;
    WvConfigSection *sect = &globalsection;
    bool quick_mode = false;

    file.open(filename, O_RDONLY);
    if (!file.isok())
    {
	// Could not open for read.
        log(loaded_once ? WvLog::Debug1 : WvLog::Warning,
	    "Can't read config file %s: %s\n", filename, file.errstr());
	if (file.geterr() != ENOENT && !loaded_once)
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
    }
    
    s->set(entry, value);
    dirty = true;
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

    if( section )
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


static WvString follow_links(WvString fname)
{
    struct stat st;
    WvString cwd;
    WvString tmp, tmpdir;
    char *cptr;
    int linksize;
    
    cwd.setsize(10240);
    getcwd(cwd.edit(), 10240-5);
    
    if (fname[0] != '/')
	fname = WvString("%s/%s", cwd, fname);
    
    if (lstat(fname, &st))
	return fname; // nonexistent file or something... stop here.
	
    for (;;)
    {
	// fprintf(stderr, "follow_links: trying '%s'\n", (const char*)fname);
	
	// not a symlink - done
	if (!S_ISLNK(st.st_mode))
	    return fname;
	
	// read the link data into tmp
	tmp.setsize(st.st_size + 2);
	cptr = tmp.edit();
	linksize = readlink(fname, cptr, st.st_size + 2);
	if (linksize < 1)
	    return fname; // ugly, but not sure what else to do...
	cptr[st.st_size] = 0;
	cptr[linksize] = 0;
	
	// not an absolute link - need to merge current path and new one
	if (cptr[0] != '/')
	{
	    // need to copy the current directory name from fname
	    tmpdir = fname;
	    cptr = tmpdir.edit();
	    cptr = strrchr(cptr, '/');
	    if (cptr)
		*cptr++ = 0;
	    
	    WvString x(tmp);
	    tmp = WvString("%s/%s", tmpdir, x);
	}
	
	if (lstat(tmp, &st))
	    return fname; // can't read target file... don't use it!
	
	fname = tmp;
    }
    
    return tmp;
}

void WvConf::save(WvStringParm _filename)
{
    if (error || !_filename)
	return;
    
    WvString xfilename(follow_links(_filename));
    
    // temporary filename has the last char changed to '!' (or '#' if it's
    // already '#').  We can't just append a character, because that might
    // confuse a dos filesystem.
    WvString tmpfilename(xfilename);
    char *cptr = strchr(tmpfilename.edit(), 0);
    cptr--;
    if (*cptr != '!')
	*cptr = '!';
    else
	*cptr = '#';
    
    ::unlink(tmpfilename);
    WvFile fp(tmpfilename, O_WRONLY|O_CREAT|O_TRUNC, create_mode);

    if (!fp.isok())
    {
	log(xfilename==filename ? WvLog::Error : WvLog::Debug1,
	    "Can't write to config file %s: %s\n",
	    tmpfilename, strerror(errno));
	if (xfilename == filename)
	    error = true;
	return;
    }
    
    globalsection.dump(fp);
    
    Iter i(*this);
    for (i.rewind(); i.next();)
    {
	WvConfigSection & sect = *i;
	fp.print("\n[%s]\n", sect.name);
	sect.dump(fp);
    }
    
    ::unlink(xfilename);
    ::rename(tmpfilename, xfilename);
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
			  WvStringParm section, WvStringParm entry)
{
    callbacks.append(new WvConfCallbackInfo(callback, userdata,
					    section, entry), true);
}


void WvConf::del_callback(WvConfCallback callback, void *userdata,
			  WvStringParm section, WvStringParm entry)
{
    WvConfCallbackInfoList::Iter i(callbacks);
    
    for (i.rewind(); i.next(); )
    {
	WvConfCallbackInfo &c(*i);
	
	if (c.callback == callback && c.userdata == userdata
	    && c.section == section && c.entry == entry)
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
