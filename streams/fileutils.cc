/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Various useful file utilities.
 *
 */
#include "fileutils.h"
#include "wvfile.h"
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <utime.h>
#include <fnmatch.h>

bool mkdirp(WvStringParm _dir, int create_mode)
{
    if (!access(_dir, X_OK))
        return true;

    // You're trying to make a nothing directory eh?
    assert(!!_dir);

    WvString dir(_dir);
    char *p = dir.edit();

    while ((p = strchr(++p, '/')))
    {
        *p = '\0';
        if (access(dir.cstr(), X_OK) && mkdir(dir.cstr(), create_mode))
            return false;
        *p = '/';
    }

    // You're probably creating the directory to write to it? Maybe this should
    // look for R_OK&X_OK instead of X_OK&W_OK...
    return  !(access(dir.cstr(), X_OK&W_OK) && mkdir(dir.cstr(), create_mode));
}


bool fcopy(WvStringParm src, WvStringParm dst)
{
    struct stat buf;
    if (stat(src, &buf))
        return false;

    WvFile in(src, O_RDONLY);
    unlink(dst);

    int oldmode = umask(0);
    WvFile out(dst, O_CREAT|O_WRONLY, buf.st_mode & 007777);
    umask(oldmode);

    in.autoforward(out);
    while (in.isok() && out.isok())
    {
	/* This used to be a select(0), but really, if select() returns
	 * false, it'll keep doing it until the end of time. If you're
	 * going into an infinite loop, better save the CPU a bit, since
	 * you can still find out about it with strace... */
        if (in.select(-1, true, false))
            in.callback();
    }
    if (!out.isok())
        return false;

    struct utimbuf utim;
    utim.actime = utim.modtime = buf.st_mtime;
    if (utime(dst, &utim))
        return false;

    return true;
}


bool fcopy(WvStringParm srcdir, WvStringParm dstdir, WvStringParm relname)
{
    return fcopy(WvString("%s/%s", srcdir, relname),
        WvString("%s/%s", dstdir, relname));
}


bool samedate(WvStringParm file1, WvStringParm file2)
{
    struct stat buf;
    struct stat buf2;

    if (stat(file1, &buf) || stat(file2, &buf2))
        return false;

    if (buf.st_mtime == buf2.st_mtime || buf.st_ctime == buf2.st_ctime)
        return true;

    return false;
}


bool samedate(WvStringParm dir1, WvStringParm dir2, WvStringParm relname)
{
    return samedate(WvString("%s/%s", dir1, relname),
        WvString("%s/%s", dir2, relname));
}


// runs fnmatch against everything in patterns.  We also interpret 
// CVS-style '!' patterns, which makes us very fancy.
bool wvfnmatch(WvStringList& patterns, WvStringParm name, int flags)
{
    WvStringList::Iter i(patterns);
    bool match = false;

    for (i.rewind(); i.next(); )
    {
        // if we hit JUST a '!', reset any matches found so far.
        if (*i == "!") {
            match = false;
            continue;
        }

        // if we hit something that starts with '!', we unmatch anything
        // found so far.
        if (i->cstr()[0] == '!')
        {
            if (!match)
                continue;   // nothing to unmatch, so why try?
            if (fnmatch(*i+1, name, flags) == 0)    // matches
                match = false;                      // unmatch it.
        }
        else
        {
            // just a straightforward matching case.
            if (fnmatch(*i, name, flags) == 0)  // matches
                match = true;
        }
    }

    return match;
}

// Only chmod a given file or dir, do not follow symlinks
int wvchmod(const char *path, mode_t mode)
{
    struct stat st;
    if (lstat(path, &st) == -1) {
        return -1;
    }

    int filedes = open(path, O_RDONLY);
    if (filedes == -1) {
	return -1;
    }

    struct stat fst;
    if (fstat(filedes, &fst) == -1) {
	close(filedes);
	return -1;
    }

    if (st.st_ino != fst.st_ino) {
	close(filedes);
	return -1;
    }

    int retval = fchmod(filedes, mode);
    close(filedes);

    return retval;
}
