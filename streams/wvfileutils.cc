/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Various useful file utilities.
 *
 */
#include "fileutils.h"
#include "wvfile.h"
#include "wvdiriter.h"
#include <string.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <fnmatch.h>
#endif
#ifndef _MSC_VER
#include <unistd.h>
#include <utime.h>
#endif

int wvmkdir(WvStringParm _dir, int create_mode)
{
#ifdef _WIN32
    return mkdir(_dir);
#else
    return mkdir(_dir, create_mode);
#endif
}

int mkdirp(WvStringParm _dir, int create_mode)
{
    if (!access(_dir, X_OK))
        return 0;

    // You're trying to make a nothing directory eh?
    assert(!!_dir);

    WvString dir(_dir);
    char *p = dir.edit();

    while ((p = strchr(++p, '/')))
    {
        *p = '\0';
        if (access(dir, X_OK) && wvmkdir(dir, create_mode))
            return -1;
        *p = '/';
    }

    // You're probably creating the directory to write to it? Maybe this should
    // look for R_OK&X_OK instead of X_OK&W_OK...
    return (access(dir, X_OK&W_OK) && wvmkdir(dir, create_mode)) ? -1 : 0;
}


void rm_rf(WvStringParm dir)
{
    WvDirIter i(dir, false, false); // non-recursive, don't skip_mounts
    for (i.rewind(); i.next(); )
    {
	if (i.isdir())
	    rm_rf(i->fullname);
	else
	    ::unlink(i->fullname);
    }
    ::rmdir(dir);
    ::unlink(dir);
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


bool ftouch(WvStringParm file, time_t mtime)
{
    if (!WvFile(file, O_WRONLY|O_CREAT).isok())
        return false;

    struct utimbuf *buf = NULL;
    if (mtime != 0)
    {
        buf = (struct utimbuf *)malloc(sizeof(struct utimbuf));
        buf->actime = time(NULL);
        buf->modtime = mtime;
    }

    if (utime(file, buf) == 0)
    {
        free(buf);
        return true;
    }

    free(buf);
    return false;
}


// Reads the contents of a symlink.  Returns WvString::null on error.
WvString wvreadlink(WvStringParm path)
{
#ifdef _WIN32
    return WvString::null; // no such thing as a symlink on Windows
#else
    WvString result;
    int size = 64;
    for (;;)
    {
        result.setsize(size);
        int readlink_result = readlink(path, result.edit(), size);
        if (readlink_result == -1)
            return WvString::null;
        if (readlink_result < size)
        {
            result.edit()[readlink_result] = '\0';
            break;
        }
        size = 2*size; // increase buffer size
    }
    return result;
#endif
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


#ifndef _WIN32
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
#endif

#ifndef _WIN32 // file permissions are too screwy in win32
int wvchmod(const char *path, mode_t mode)
{
    struct stat st;
    if (lstat(path, &st) == -1) {
        return -1;
    }

    int filedes = open(path, O_RDONLY);
    if (filedes == -1) {
	// if we're not running as root, this file/dir may have 0
	// perms and open() fails, so let's try again
	//
	// NOTE: This is not as secure as the proper way, since
	// it's conceivable that someone swaps out the dir/file
	// for a symlink between our check and the chmod() call
	//
        struct stat sst;
	if (getuid() != 0)
	    if (stat(path, &sst) != -1)
		if (st.st_ino == sst.st_ino)
		    return chmod(path, mode);
	
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

#ifndef _WIN32
    // we're definitely chmod'ing the open file here, which is good,
    // because the filename itself might have been moved around between
    // our stat'ing and chmod'ing it.
    int retval = fchmod(filedes, mode);
#else
    // this is guaranteed to be the same file as filedes, because in
    // Windows, open files can't be changed on the filesystem (unlike in
    // Unix).
    int retval = chmod(path, mode);
#endif
    close(filedes);

    return retval;
}
#endif // !_WIN32


FILE *wvtmpfile()
{
#ifndef _WIN32 // tmpfile() is really the best choice, when it works
    return tmpfile();
#else
    // in win32, tmpfile() creates files in c:\...
    // and that directory isn't always writable!  Idiots.
    char *name = _tempnam("c:\\temp", "wvtmp");
    FILE *f = fopen(name, "wb+");
    free(name);
    return f;
#endif
}


WvString wvtmpfilename(WvStringParm prefix)
{
#ifndef _WIN32 // tmpfile() is really the best choice, when it works
    WvString tmpname("/tmp/%sXXXXXX", prefix);
    int fd;
    if ((fd = mkstemp(tmpname.edit())) == (-1))
        return WvString();    
    close(fd);
#else
    WvString tmpname(_tempnam("c:\\temp", prefix.cstr()));
    int fd;
    fd = open(tmpname, O_WRONLY|O_CREAT|O_EXCL, 0777);
    if (fd < 0)
	return WvString::null; // weird
    _close(fd);
#endif

    return tmpname;
}


WvString wvfullpath(WvStringParm fname)
{
#ifdef _WIN32
    long len = GetFullPathName(fname, 0, NULL, NULL);
    assert(len >= 0);
    WvString buf;
    buf.setsize(len);
    long ret = GetFullPathName(fname, len, buf.edit(), NULL);
    assert(ret < len);
    return buf;
#else
    if (!fname || fname[0] == '/' || fname[0] == '\\')
	return fname; // already a full path
    else
	return WvString("%s/%s", wvgetcwd(), fname);
#endif
}


mode_t get_umask()
{
    mode_t rv = umask(0);
    umask(rv);

    return rv;
}

