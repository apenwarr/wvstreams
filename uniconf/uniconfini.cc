/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
 
/** \file
 * A generator for .ini files.
 */
#include "uniconfini.h"
#include "uniconfiter.h"
#include "wvtclstring.h"
#include "strutils.h"
#include "wvfile.h"

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


/***** UniConfIniFileGen *****/

UniConfIniFileGen::UniConfIniFileGen(
    UniConf *_top, WvStringParm _filename) :
    UniConfGen(WvString("ini://%s", _filename)),
    filename(_filename), log(filename)
{
    top = _top;
    log(WvLog::Debug1, "Using IniFile '%s' at location '%s'.\n", filename, top->full_key());
}


UniConfIniFileGen::~UniConfIniFileGen()
{
}


void UniConfIniFileGen::load()
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
	    h = make_tree(top, UniConfKey(section, key));
	    
	    bool d1 = h->dirty, d2 = h->child_dirty;
	    h->setvalue(value);
	    
	    // loaded _from_ the config file, so that didn't make it dirty!
	    h->dirty = d1;
	    h->child_dirty = d2;
	}
    }
}


void UniConfIniFileGen::save()
{
    WvString newfile(filename);
    if (!top->dirty && !top->child_dirty)
	return; // no need to rewrite!
    
    log("Saving %s...\n", newfile);
    
    WvFile out(WvString("%s", newfile), O_WRONLY|O_CREAT|O_TRUNC);
    if (out.isok())
    {
        save_subtree(out, top, UniConfKey::EMPTY);
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
    ret = NULL;
    
    int nchildren = 0;
    UniConf::Iter it(*h);
    for (it.rewind(); it.next(); )
    {
	nchildren += count_children(it.ptr(), tmp);
	if (! it->value().isnull())
	    nchildren++;
	
	if (!ret)
	{
	    if (! it->value().isnull())
		ret = it.ptr();
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
    
    UniConf::Iter it(*h);
    for (it.rewind(); it.next(); )
    {
	if (! it->value().isnull())
	    return true; // a direct value exists: we have to write it.
	else if (h->parent() && count_children(it.ptr(), junk) == 1)
	    return true; // exactly one indirect value exists for this child.
    }
    
    return false;
}


void UniConfIniFileGen::save_subtree(WvStream &out, UniConf *h, UniConfKey key)
{
    UniConf *interesting;
    
    // special case: the root node of this generator shouldn't get its own
    // section unless there are _really_ nodes directly in that section.
    // (ie. single-entry subsections should still get their own subsections
    // in the file, for compatibility with the old WvConf).
    bool top_special = h->comparegen(this); //(h->generator==this);
    
    // dump the "root level" of this tree into one section
    if (any_interesting_children(h))
    {
	// we could use inicode here, but then section names containing
	// '=' signs get quoted unnecessarily.
        /** note: we strip off leading slashes here **/
	out("\n[%s]\n", wvtcl_escape(key.strip(), " \t\r\n[]"));
    
	UniConf::Iter it(*h);
	for (it.rewind(); it.next(); )
	{
	    if (it->hasgen() && ! it->comparegen(this))
	        continue;
                
	    if (! it->value().isnull())
            {
		out("%s = %s\n", inicode(it->key().strip()),
                    inicode(it->value()));
            }
	    
	    if (!top_special && count_children(it.ptr(), interesting) == 1)
	    {
		// exactly one interesting child: don't bother with a
		// subsection.
                out("%s = %s\n",
                    inicode(interesting->full_key(h).strip()),
                    inicode(interesting->value()));
	    }
	}
    }
    
    // dump subtrees into their own sections
    if (h->haschildren())
    {
	UniConf::Iter it(*h);
	for (it.rewind(); it.next(); )
	{
	    if (it->hasgen() && ! it->comparegen(this))
		it->save();
	    else if (it->haschildren() && (top_special ||
		count_children(it.ptr(), interesting) > 1))
	    {
		save_subtree(out, it.ptr(), UniConfKey(key, it->key()));
	    }
	}
    }
}



/***** UniConfIniFileGenFactory *****/

UniConfGen *UniConfIniFileGenFactory::newgen(
    const UniConfLocation &location, UniConf *top)
{
    return new UniConfIniFileGen(top, location.payload());
}
