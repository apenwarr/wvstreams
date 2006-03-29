/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A generator for .ini files.
 */
#include "uniinigen.h"
#include "strutils.h"
#include "unitempgen.h"
#include "wvfile.h"
#include "wvmoniker.h"
#include "wvstringmask.h"
#include "wvtclstring.h"
#include <ctype.h>
#include "wvlinkerhack.h"

WV_LINK(UniIniGen);


static IUniConfGen *creator(WvStringParm s, IObject *, void *)
{
    return new UniIniGen(s);
}

WvMoniker<IUniConfGen> UniIniGenMoniker("ini", creator);


/***** UniIniGen *****/

UniIniGen::UniIniGen(WvStringParm _filename, int _create_mode, UniIniGen::SaveCallback _save_cb)
    : filename(_filename), create_mode(_create_mode), log(_filename), save_cb(_save_cb)
{
    // Create the root, since this generator can't handle it not existing.
    UniTempGen::set(UniConfKey::EMPTY, WvString::empty);
    memset(&old_st, 0, sizeof(old_st));
}


void UniIniGen::set(const UniConfKey &key, WvStringParm value)
{
    UniTempGen::set(key, value);

    // Re-create the root, since this generator can't handle it not existing.
    if (value.isnull() && key.isempty())
        UniTempGen::set(UniConfKey::EMPTY, WvString::empty);

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
    
    if (file.isok() // guarantes statbuf is valid from above
	&& statbuf.st_ctime == old_st.st_ctime
	&& statbuf.st_dev == old_st.st_dev
	&& statbuf.st_ino == old_st.st_ino
	&& statbuf.st_blocks == old_st.st_blocks
	&& statbuf.st_size == old_st.st_size)
    {
	log(WvLog::Debug3, "refresh: file hasn't changed; do nothing.\n");
	return true;
    }
    memcpy(&old_st, &statbuf, sizeof(statbuf));
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
    newgen->set(UniConfKey::EMPTY, WvString::empty);
    UniConfKey section;
    WvDynBuf buf;
    while (buf.used() || file.isok())
    {
        if (file.isok())
        {
            // read entire lines to ensure that we get whole values
            char *line = file.blocking_getline(-1);
            if (line)
	    {
                buf.putstr(line);
		buf.put('\n'); // this was auto-stripped by getline()
	    }
        }

        WvString word;
	while (!(word = wvtcl_getword(buf,
				      WVTCL_NASTY_NEWLINES,
				      false)).isnull())
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
	    static const WvStringMask nasty_equals("=");
            WvString name = wvtcl_getword(line, nasty_equals, false);
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
	    
	    // if we get here, the line was tcl-decoded but not useful.
            log(WvLog::Warning,
		"Ignoring malformed input line: \"%s\"\n", word);
        }
	
	if (buf.used() && !file.isok())
	{
	    // EOF and some of the data still hasn't been used.  Weird.
	    // Let's remove a line of data and try again.
	    size_t offset = buf.strchr('\n');
	    assert(offset); // the last thing we put() is *always* a newline!
	    WvString line1(trim_string(buf.getstr(offset).edit()));
	    if (!!line1) // not just whitespace
		log(WvLog::Warning,
		    "XXX Ignoring malformed input line: \"%s\"\n", line1);
	}
    }

    if (file.geterr())
    {
        log(WvLog::Warning, 
	    "Error reading from config file: %s\n", file.errstr());
        WVRELEASE(newgen);
        return false;
    }

    // switch the trees and send notifications
    hold_delta();
    UniConfValueTree *oldtree = root;
    UniConfValueTree *newtree = newgen->root;
    root = newtree;
    newgen->root = NULL;
    dirty = false;
    oldtree->compare(newtree, UniConfValueTree::Comparator
            (this, &UniIniGen::refreshcomparator), NULL);
    
    delete oldtree;
    unhold_delta();

    WVRELEASE(newgen);

    UniTempGen::refresh();
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
	    // Issue notifications for every that is missing.
            a->visit(UniConfValueTree::Visitor(this,
                &UniIniGen::notify_deleted), NULL, false, true);
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
bool UniIniGen::commit_atomic(WvStringParm real_filename)
{
    struct stat statbuf;

    if (lstat(real_filename, &statbuf) == -1)
    {
	if (errno != ENOENT)
	    return false;
    }
    else
	if (!S_ISREG(statbuf.st_mode))
	    return false;

    WvString tmp_filename("%s.tmp%s", real_filename, getpid());
    WvFile file(tmp_filename, O_WRONLY|O_TRUNC|O_CREAT, 0000);

    if (file.geterr())
    {
	log(WvLog::Warning, "Can't write '%s': %s\n",
	    tmp_filename, strerror(errno));
	unlink(tmp_filename);
	file.close();
	return false;
    }

    save(file, *root); // write the changes out to our temp file

    mode_t theumask = umask(0);
    umask(theumask);
    fchmod(file.getwfd(), create_mode & ~theumask);

    file.close();

    if (file.geterr() || rename(tmp_filename, real_filename) == -1)
    {
        log(WvLog::Warning, "Can't write '%s': %s\n",
	    filename, strerror(errno));
        unlink(tmp_filename);
	return false;
    }

    return true;
}
#endif


void UniIniGen::commit()
{
    if (!dirty)
	return;

    UniTempGen::commit();

#ifdef _WIN32
    // Windows doesn't support all that fancy stuff, just open the
    // file and be done with it
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
    WvString real_filename(filename);
    char resolved_path[PATH_MAX];

    if (realpath(filename, resolved_path) != NULL)
	real_filename = resolved_path;

    if (!commit_atomic(real_filename))
    {
        WvFile file(real_filename, O_WRONLY|O_TRUNC|O_CREAT, create_mode);
        struct stat statbuf;

        if (fstat(file.getwfd(), &statbuf) == -1)
        {
            log(WvLog::Warning, "Can't write '%s' ('%s'): %s\n",
                filename, real_filename, strerror(errno));
            return;
        }

        fchmod(file.getwfd(), (statbuf.st_mode & 07777) | S_ISVTX);

        save(file, *root);
    
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


static void printsection(WvStream &file, const UniConfKey &key, UniIniGen::SaveCallback save_cb)
{
    WvString s;
    static const WvStringMask nasties("\r\n[]");

    if (absolutely_needs_escape(key, "\r\n[]"))
	s = wvtcl_escape(key, nasties);
    else
	s = key;
    // broken up for optimization, no temp wvstring created
    //file.print("\n[%s]\n", s);
    file.print("\n[");
    file.print(s);
    file.print("]\n");

    if (!!save_cb)
        save_cb();
}


static void printkey(WvStream &file, const UniConfKey &_key,
		     WvStringParm _value, UniIniGen::SaveCallback save_cb)
{
    WvString key, value;
    static const WvStringMask nasties("\r\n\t []=#");

    if (absolutely_needs_escape(_key, "\r\n[]=#\""))
	key = wvtcl_escape(_key, nasties);
    else if (_key == "")
	key = "/";
    else
	key = _key;
    
    // value is more relaxed, since we don't use wvtcl_getword after we grab
    // the "key=" part of each line
    if (absolutely_needs_escape(_value, "\r\n"))
	value = wvtcl_escape(_value, WVTCL_NASTY_SPACES);
    else
	value = _value;
    
    // need to escape []#= in key only to distinguish a key/value
    // pair from a section name or comment and to delimit the value
    // broken up for optimization, no temp wvstring created
    //file.print("%s = %s\n", key, value);
    file.print(key);
    file.print(" = ");
    file.print(value);
    file.print("\n");

    if (!!save_cb)
        save_cb();
}


static void save_sect(WvStream &file, UniConfValueTree &toplevel,
		      UniConfValueTree &sect, bool &printedsection,
		      bool recursive, UniIniGen::SaveCallback save_cb)
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
                printsection(file, toplevel.fullkey(), save_cb);
                printedsection = true;
            }
            printkey(file, node.fullkey(&toplevel), node.value(), save_cb);
        }

	// print all children, if requested
	if (recursive && node.haschildren())
	    save_sect(file, toplevel, node, printedsection, recursive, save_cb);
    }
}


void UniIniGen::save(WvStream &file, UniConfValueTree &parent)
{
    // parent might be NULL, so it really should be a pointer, not
    // a reference.  Oh well...
    if (!&parent) return;
    
    if (parent.fullkey() == root->fullkey())
    {
	// the root itself is a special case, since it's not in a section,
	// and it's never NULL (so we don't need to write it if it's just
	// blank)
	if (!!parent.value())
	    printkey(file, parent.key(), parent.value(), save_cb);
    }

    bool printedsection = false;
    
    save_sect(file, parent, parent, printedsection, false, save_cb);
    
    UniConfValueTree::Iter it(parent);
    for (it.rewind(); it.next(); )
    {
        UniConfValueTree &node = *it;
	
	printedsection = false;
	save_sect(file, node, node, printedsection, true, save_cb);
    }
}
