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
#include "wvlinkerhack.h"

WV_LINK(UniIniGen);


static IUniConfGen *creator(WvStringParm s, IObject *, void *)
{
    return new UniIniGen(s);
}

WvMoniker<IUniConfGen> UniIniGenMoniker("ini", creator);


// forward declarations
static void printkey(WvStream &file, const UniConfKey &_key,
		     WvStringParm _value);


/***** UniIniGen *****/

UniIniGen::UniIniGen(WvStringParm _filename, int _create_mode)
    : filename(_filename), create_mode(_create_mode), log(_filename)
{
    //log(WvLog::Debug1, "Using IniFile \"%s\"\n", filename);
    // consider the generator dirty until it is first refreshed
    dirty = true;
}


UniIniGen::~UniIniGen()
{
}


bool UniIniGen::refresh()
{
    WvFile file(filename, O_RDONLY);

    #ifndef _WIN32
    struct stat statbuf;
    if (file.isok() && fstat(file.getrfd(), &statbuf) == -1)
    {
	log(WvLog::Warning, "Can't stat '%s': %s\n",
	    filename, strerror(errno));
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
        log(WvLog::Warning, 
	    "Can't open '%s' for reading: %s\n"
	    "...starting with blank configuration.\n",
	    filename, file.errstr());
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
            char *line = file.blocking_getline(-1);
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
                //log(WvLog::Debug5, "Comment: \"%s\"\n", str + 1);
                continue;
            }
	    
            if (str[0] == '[' && str[len - 1] == ']')
            {
                // a section name
                str[len - 1] = '\0';
                WvString name(wvtcl_unescape(trim_string(str + 1)));
                section = UniConfKey(name);
                //log(WvLog::Debug5, "Refresh section: \"%s\"\n", section);
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
	    
            log(WvLog::Warning,
		"Ignoring malformed input line: \"%s\"\n", word);
        }
    }

    // close the file
    file.close();
    if (file.geterr())
    {
        log(WvLog::Warning, 
	    "Error reading from config file: \"%s\"\n", file.errstr());
        WVRELEASE(newgen);
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
        log(WvLog::Warning,
	    "XXX Ignoring malformed input line: \"%s\"\n", buf.getstr());
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
    }
    else
    {
        delta(UniConfKey::EMPTY, WvString::null); // REMOVED
    }
    delete oldtree;
    unhold_delta();

    WVRELEASE(newgen);

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


#ifndef _WIN32
bool UniIniGen::commit_atomic(WvString real_filename)
{
    WvString tmp_filename("%s.tmp%s", real_filename, getpid());
    WvFile file(tmp_filename, O_WRONLY|O_TRUNC|O_CREAT, create_mode);
    struct stat statbuf;
    
    if (file.geterr()
	|| lstat(real_filename, &statbuf) == -1
	|| !S_ISREG(statbuf.st_mode))
    {
	unlink(tmp_filename);
        file.close();
        return false;
    }
    
    save(file, *root); // write the changes out to our temp file
    
    file.close();
    if (rename(tmp_filename, real_filename) == -1
            || file.geterr())
    {
        unlink(tmp_filename);
	return false;
    }

    return true;
}
#endif


void UniIniGen::commit()
{
    if (!dirty) return;

#ifdef _WIN32
    // Windows doesn't support all that fancy stuff, just open the file
    //   and be done with it
    WvFile file(filename, O_WRONLY|O_TRUNC|O_CREAT, create_mode);
    save(file, *root); // write the changes out to our file
    file.close();
    if (file.geterr())
    {
        log(WvLog::Warning, "Can't write '%s': %s\n",
	    filename, file.errstr());
	return;
    }
#else
    char resolved_path[PATH_MAX];
    WvString real_filename(filename);

    if (realpath(filename, resolved_path) != NULL)
	real_filename = resolved_path;

    WvString alt_filename("%s.tmp%s", real_filename, getpid());
    WvFile file(alt_filename, O_WRONLY|O_TRUNC|O_CREAT, 0000);
    struct stat statbuf;

    // first try to overwrite the file atomically
    if (!commit_atomic(real_filename))
    {
        // if not, overwrite it in place
        WvFile file(real_filename, O_WRONLY|O_TRUNC|O_CREAT, create_mode);
        struct stat statbuf;

        if (fstat(file.getwfd(), &statbuf) == -1)
        {
            log(WvLog::Warning, "Can't write '%s' ('%s'): %s\n",
                filename, real_filename, strerror(errno));
            return;
        }

        fchmod(file.getwfd(), (statbuf.st_mode & 07777) | S_ISVTX);
        save(file, *root); // write the changes out to our file
    
        if (!file.geterr())
        {
	    /* We only reset the sticky bit if all went well, but before
	     * we close it, because we need the file descriptor. */
	    statbuf.st_mode = statbuf.st_mode & ~S_ISVTX;
	    fchmod(file.getwfd(), statbuf.st_mode & 07777);
	}
	else
	    log(WvLog::Warning, "Error writing '%s' ('%s'): %s\n",
		filename, real_filename, file.errstr());
    }
#endif

    file.close();

    if (file.geterr())
    {
        log(WvLog::Warning, "Can't write '%s': %s\n",
	    filename, file.errstr());
	return;
    }

#ifndef _WIN32
    if (!alt_filename.isnull())
    {
	chmod(alt_filename, create_mode);
	if (rename(alt_filename, real_filename) == -1)
	{
	    log(WvLog::Warning, "Can't write '%s': %s\n",
		filename, strerror(errno));
	    unlink(alt_filename);
	    return;
	}
    }
#endif

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
    
    if (numbraces != 0)
        return true; // uneven number of braces, can't be good

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


static void save_sect(WvStream &file, UniConfValueTree &toplevel,
		      UniConfValueTree &sect, bool &printedsection,
		      bool recursive)
{
    UniConfValueTree::Iter it(sect);
    for (it.rewind(); it.next(); )
    {
        UniConfValueTree &node = *it;
	
	// FIXME: we never print empty-string ("") keys, for compatibility
	// with WvConf.  Example: set x/y = 1; delete x/y; now x = "", because
	// it couldn't be NULL while x/y existed, and nobody auto-deleted it
	// when x/y went away.  Therefore we would try to write x = "" to the
	// config file, but that's not what WvConf would do.
	// 
	// The correct fix would be to auto-delete x if the only reason it
	// exists is for x/y.  But since that's hard, we'll just *never*
	// write lines for "" entries.  Icky, but it works.
        if (!!node.value())// || !node.haschildren())
        {
            if (!printedsection)
            {
                printsection(file, toplevel.fullkey());
                printedsection = true;
            }
            printkey(file, node.fullkey(&toplevel), node.value());
        }

	// print all children, if requested
	if (recursive && node.haschildren())
	    save_sect(file, toplevel, node, printedsection, recursive);
    }
}


void UniIniGen::save(WvStream &file, UniConfValueTree &parent)
{
    if (parent.fullkey() == root->fullkey())
    {
	// the root itself is a special case, since it's not in a section,
	// and it's never NULL (so we don't need to write it if it's just
	// blank)
	if (!!parent.value())
	    printkey(file, parent.key(), parent.value());
    }

    bool printedsection = false;
    
    save_sect(file, parent, parent, printedsection, false);
    
    UniConfValueTree::Iter it(parent);
    for (it.rewind(); it.next(); )
    {
        UniConfValueTree &node = *it;
	
	printedsection = false;
	save_sect(file, node, node, printedsection, true);
    }
}
