/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A generator for .ini files.
 */
#include "uniinigen.h"
#include "unitempgen.h"
#include "wvtclstring.h"
#include "strutils.h"
#include "wvfile.h"
#include "wvmoniker.h"
#include <ctype.h>

static IUniConfGen *creator(WvStringParm s, IObject *, void *)
{
    return new UniIniGen(s);
}

static WvMoniker<IUniConfGen> reg("ini", creator);


// forward declarations
static void printkey(WvStream &file, const UniConfKey &_key,
		     WvStringParm _value);


/***** UniIniGen *****/

UniIniGen::UniIniGen(WvStringParm _filename, int _create_mode)
    : filename(_filename), create_mode(_create_mode), log(filename)
{
    log(WvLog::Debug1, "Using IniFile \"%s\"\n", filename);
    // consider the generator dirty until it is first refreshed
    dirty = true;
}


UniIniGen::~UniIniGen()
{
}


bool UniIniGen::refresh()
{
    WvFile file(filename, O_RDONLY);
    if (!file.isok())
    {
        log("Can't open '%s' for reading: %s\n", filename, file.errstr());
        return false;
    }
    
    // loop over all Tcl words in the file
    UniTempGen *newgen = new UniTempGen();
    UniConfKey section;
    WvDynBuf buf;
    for (bool eof = false; !eof; )
    {
        if (file.isok())
        {
            // read entire lines to ensure that we get whole values
            char *line = file.getline(-1);
            if (line)
                buf.putstr(line);
            else
                eof = true;
        }
        else
            eof = true;

        if (eof)
        {
            // detect missing newline at end of file
            size_t avail = buf.used();
            if (avail == 0)
                break;
            if (buf.peek(avail - 1) == '\n')
                break;
            // run the loop one more time to compensate
        }
        buf.put('\n');

        WvString word;
	while (!(word = wvtcl_getword(buf, "\r\n", false)).isnull())
        {
	    //log(WvLog::Info, "LINE: '%s'\n", word);
	    
            char *str = trim_string(word.edit());
            int len = strlen(str);
            if (len == 0) continue; // blank line
	    
            if (str[0] == '#')
            {
                // a comment line.  FIXME: we drop it completely!
                log(WvLog::Debug5, "Comment: \"%s\"\n", str + 1);
                continue;
            }
	    
            if (str[0] == '[' && str[len - 1] == ']')
            {
                // a section name
                str[len - 1] = '\0';
                WvString name(wvtcl_unescape(trim_string(str + 1)));
                section = UniConfKey(name);
                log(WvLog::Debug5, "Refresh section: \"%s\"\n", section);
                continue;
            }
	    
            // we possibly have a key = value line
            WvConstStringBuffer line(word);
            WvString name = wvtcl_getword(line, "=", false);
            if (!name.isnull() && line.used())
            {
                name = wvtcl_unescape(trim_string(name.edit()));
		
                if (!!name)
                {
		    UniConfKey key(name);
                    key.prepend(section);
		    
                    WvString value = line.getstr();
		    assert(*value == '=');
                    value = wvtcl_unescape(trim_string(value.edit() + 1));
                    newgen->set(key, value.unique());

                    //log(WvLog::Debug5, "Refresh: (\"%s\", \"%s\")\n",
                    //    key, value);
                    continue;
                }
            }
	    
            log("Ignoring malformed input line: \"%s\"\n", word);
        }
    }

    // close the file
    file.close();
    if (file.geterr())
    {
        log("Error reading from config file: \"%s\"\n", file.errstr());
        delete newgen;
        return false;
    }

    // handle unparsed input
    size_t avail = buf.used();
    while (avail > 0 && buf.peek(avail - 1) == '\n')
    {
        buf.unalloc(1); // strip off uninteresting trailing newlines
        avail -= 1;
    }
    if (avail > 0)
    {
        // last line must have contained junk
        log("XXX Ignoring malformed input line: \"%s\"\n", buf.getstr());
    }

    // switch the trees and send notifications
    hold_delta();
    UniConfValueTree *oldtree = root;
    UniConfValueTree *newtree = newgen->root;
    root = newtree;
    newgen->root = NULL;
    dirty = false;
    if (oldtree && newtree)
    {
        oldtree->compare(newtree, UniConfValueTree::Comparator
            (this, &UniIniGen::refreshcomparator), NULL);
        delete oldtree;
    }
    else
    {
        delta(UniConfKey::EMPTY, WvString::null); // REMOVED
    }
    unhold_delta();

    delete newgen;

    return true;
}


// returns: true if a==b
bool UniIniGen::refreshcomparator(const UniConfValueTree *a,
    const UniConfValueTree *b, void *userdata)
{
    if (a)
    {
        if (b)
        {
            if (a->value() != b->value())
            {
                // key changed
                delta(b->fullkey(), b->value()); // CHANGED
		return false;
            }
            return true;
        }
        else
        {
            // key removed
            delta(a->fullkey(), WvString::null); // REMOVED
            return false;
        }
    }
    else // a didn't exist
    {
        assert(b);
        // key added
        delta(b->fullkey(), b->value()); // ADDED
        return false;
    }
}


void UniIniGen::commit()
{
    if (!dirty) return;

    WvFile file(filename, O_WRONLY|O_TRUNC|O_CREAT, create_mode);
    
    if (root) // the tree may be empty, so NULL root is okay
    {
	// the root itself is a special case, since it's not in a section,
	// and it's never NULL (so we don't need to write it if it's just
	// blank)
	if (!!root->value())
	    printkey(file, root->key(), root->value());
	
	// do all subkeys
	save(file, *root);
    }

    file.close();
    if (file.geterr())
        log("Can't write '%s': %s\n", filename, file.errstr());
    else
	dirty = false;
}


// may return false for strings that wvtcl_escape would escape anyway; this
// may not escape tcl-invalid strings, but that's on purpose so we can keep
// old-style .ini file compatibility (and wvtcl_getword() and friends can
// still parse them anyway).
static bool absolutely_needs_escape(WvStringParm s, const char *sepchars)
{
    const char *cptr;
    int numbraces = 0;
    bool inescape = false, inspace = false;
    
    if (isspace((unsigned char)*s))
	return true; // leading whitespace needs escaping
    
    for (cptr = s; *cptr; cptr++)
    {
	if (inescape)
	    inescape = false; // fine
	else if (!numbraces && strchr(sepchars, *cptr))
	    return true; // one of the magic characters, and not escaped
	else if (*cptr == '\\')
	    inescape = true;
	else if (*cptr == '{')
	    numbraces++;
	else if (*cptr == '}')
	    numbraces--;
	
	inspace = isspace((unsigned char)*cptr);
	
	if (numbraces < 0) // yikes!  mismatched braces will need some help.
	    return false; 
    }
    
    if (inescape || inspace)
	return true; // terminating backslash or whitespace... evil.
    
    // otherwise, I guess we're safe.
    return false;
}


static void printsection(WvStream &file, const UniConfKey &key)
{
    WvString s;
    
    if (absolutely_needs_escape(key, "\r\n[]"))
	s = wvtcl_escape(key, "\r\n[]");
    else
	s = key;
    file.print("\n[%s]\n", s);
}


static void printkey(WvStream &file, const UniConfKey &_key,
		     WvStringParm _value)
{
    WvString key, value;
    
    if (absolutely_needs_escape(_key, "\r\n[]=#\""))
	key = wvtcl_escape(_key, "\r\n\t []=#");
    else if (_key == "")
	key = "/";
    else
	key = _key;
    
    // value is more relaxed, since we don't use wvtcl_getword after we grab
    // the "key=" part of each line
    if (absolutely_needs_escape(_value, "\r\n"))
	value = wvtcl_escape(_value, "\r\n\t ");
    else
	value = _value;
    
    // need to escape []#= in key only to distinguish a key/value
    // pair from a section name or comment and to delimit the value
    file.print("%s = %s\n", key, value);
}


void UniIniGen::save(WvStream &file, UniConfValueTree &parent)
{
    UniConfValueTree::Iter it(parent);
    
    // we want to ensure that a key with an empty value will
    // get created either by writing its value or by ensuring
    // that some subkey will create it implicitly with empty value
    bool printedsection = false;
    for (it.rewind(); it.next() && file.isok(); )
    {
        UniConfValueTree *node = it.ptr();
        if (!!node->value() || !node->haschildren())
        {
            if (!printedsection)
            {
                printsection(file, parent.fullkey());
                printedsection = true;
            }
            printkey(file, node->key(), node->value());
        }
    }

    // do subsections
    for (it.rewind(); it.next() && file.isok(); )
        save(file, *it);
}
