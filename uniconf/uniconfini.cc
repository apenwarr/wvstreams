/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * UniConfIniFile is a UniConfGen for ".ini"-style files like the ones used
 * by Windows and the original WvConf.
 * 
 * See uniconfini.h.
 */
#include "uniconfini.h"
#include "uniconfiter.h"
#include "wvtclstring.h"
#include "strutils.h"
#include "wvfile.h"


UniConfIniFile::UniConfIniFile(UniConf *_top, WvStringParm _filename, bool automount)
    : filename(_filename), log(filename)
{
    top = _top;
    save_test = false;
    log(WvLog::Notice, "Using IniFile '%s' at location '%s'.\n", 
	filename, top->full_key());
    if (automount)
        top->mount(this);
}


static WvFastString inicode(WvStringParm s)
{
    static const char white[] = " \t\r\n";
    
    // we need braces if the string starts/ends with whitespace
    if (!!s && (strchr(white, *s) || strchr(white, s[strlen(s)-1])))
	return wvtcl_escape(s, " \t\r\n=[]");
    else
	return wvtcl_escape(s, "\r\n=[]");
}


static WvString inidecode(WvStringParm _s)
{
    WvString s(_s);
    return wvtcl_unescape(trim_string(s.edit()));
}


static void inisplit(WvStringParm s, WvString &key, WvString &value)
{
    WvStringList l;
    wvtcl_decode(l, s, "=", false);
    
    if (l.count() < 2) // couldn't split
    {
	key = value = WvString();
	return;
    }
    
    WvStringList::Iter i(l);
    i.rewind(); i.next();
    key = wvtcl_unescape(trim_string(i->edit()));
    
    WvString tval("");
    while (i.next())
    {
	// handle embedded equal signs that got split.
	// FIXME: in lines like 'foo = blah == weasels', we'll read as if
	//   it were 'foo = blah = weasels', which is kinda bad...
	if (!!tval)
	    tval.append("=");
	tval.append(*i);
    }
    
    value = wvtcl_unescape(trim_string(tval.edit()));
    //printf(" split '%s' to '%s'='%s'\n", (const char *)s, 
    //    key.cstr(), value.cstr());
}


void UniConfIniFile::load()
{
    char *cptr, *line;
    UniConf *h;
    WvFile f(filename, O_RDONLY);
    WvDynamicBuffer b;
    WvString section = "";
    size_t len;
    
    if (!f.isok())
    {
	log("Can't open config file: %s\n", f.errstr());
	return;
    }
    
    while (f.isok())
    {
	cptr = (char *)b.alloc(1024);
	len = f.read(cptr, 1024);
	b.unalloc(1024-len);
    }
    
    WvStringList l;
    wvtcl_decode(l, b.getstr(), "\r\n", "\r\n");
    
    WvStringList::Iter i(l);
    for (i.rewind(); i.next(); )
    {
	line = trim_string(i->edit());
	//log("-*- '%s'\n", line);
	
	// beginning of a new section?
	if (line[0] == '[' && line[strlen(line)-1] == ']')
	{
	    line[strlen(line)-1] = 0;
	    WvString ss(line+1);
	    section = inidecode(ss);
	    continue;
	}
	
	// name = value setting?
	WvString key, value;
	inisplit(line, key, value);
	if (!!key && !!value)
	{
	    h = make_tree(top, WvString("%s/%s", section, key));
	    
	    bool d1 = h->dirty, d2 = h->child_dirty;
	    h->set_without_notify(value);
	    
	    // loaded _from_ the config file, so that didn't make it dirty!
	    h->dirty = d1;
	    h->child_dirty = d2;
	}
    }
}


void UniConfIniFile::save()
{
    WvString newfile(filename);
    if (save_test)
	newfile = WvString("%s.new", filename);
    
    if (!top->dirty && !top->child_dirty)
	return; // no need to rewrite!
    
    log("Saving %s...\n", newfile);
    
    WvFile out(WvString("%s", newfile), O_WRONLY|O_CREAT|O_TRUNC);
    if (out.isok())
    {
        save_subtree(out, top, "/");
        out("\n");
    }
    else
        log("Error writing to config file: %s\n", out.errstr());
}


// find the number of non-empty-valued nodes under 'h', not including 'h'
// itself.  Store a pointer to the first one in 'ret'.
static int count_children(UniConf *h, UniConf *&ret)
{
    UniConf *tmp = NULL;
    int n, nchildren = 0;
    
    ret = NULL;
    
    if (!h->has_children())
	return 0;
    
    UniConf::Iter i(*h);
    for (i.rewind(); i.next(); )
    {
	n = count_children(i.ptr(), tmp);
	if (!!*i)
	    nchildren++;
	nchildren += n;
	
	if (!ret)
	{
	    if (!!*i)
		ret = i.ptr();
	    else
		ret = tmp;
	}
    }
    
    return nchildren;
}


// true if 'h' has any non-empty-valued child nodes beneath it, with a
// maximum of one non-empty-valued node per direct child.  The idea here
// is to let us collapse inifile branches to minimize the number of
// single-entry sections.
static bool any_interesting_children(UniConf *h)
{
    UniConf *junk;
    
    if (!h->has_children())
	return false;
    
    UniConf::Iter i(*h);
    for (i.rewind(); i.next(); )
    {
	if (!!*i)
	    return true; // a direct value exists: we have to write it.
	else if (h->parent && count_children(i.ptr(), junk) == 1)
	    return true; // exactly one indirect value exists for this child.
    }
    
    return false;
}


void UniConfIniFile::save_subtree(WvStream &out, UniConf *h, UniConfKey key)
{
    UniConf *interesting;
    
    // special case: the root node of this generator shouldn't get its own
    // section unless there are _really_ nodes directly in that section.
    // (ie. single-entry subsections should still get their own subsections
    // in the file, for compatibility with the old WvConf).
    bool top_special = (h->generator==this);
    
    // dump the "root level" of this tree into one section
    if (any_interesting_children(h))
    {
	// we could use inicode here, but then section names containing
	// '=' signs get quoted unnecessarily.
	out("\n[%s]\n", wvtcl_escape(key, " \t\r\n[]"));
    
	UniConf::Iter i(*h);
	for (i.rewind(); i.next(); )
	{
	    if (i->generator && i->generator != this) continue;
	    
	    if (!!*i)
		out("%s = %s\n", inicode(i->name), inicode(*i));
	    
	    if (!top_special && count_children(i.ptr(), interesting) == 1)
	    {
		// exactly one interesting child: don't bother with a
		// subsection.
		UniConfKey deepkey(interesting->full_key(h));
		out("%s = %s\n", inicode(deepkey), inicode(*interesting));
	    }
	}
    }
    
    // dump subtrees into their own sections
    if (h->has_children())
    {
	UniConf::Iter i(*h);
	for (i.rewind(); i.next(); )
	{
	    if (i->generator && i->generator != this)
		i->save();
	    else if (i->has_children()
		     && (top_special
			 || count_children(i.ptr(), interesting) > 1))
	    {
		UniConfKey key2(key);
		key2.append(&i->name, false);
		save_subtree(out, i.ptr(), key2);
	    }
	}
    }
}


