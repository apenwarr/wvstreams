/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Various useful file utilities.
 *
 */
#include "fileutils.h"
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <utime.h>
#include <wvfile.h>

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
        if (in.select(0))
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
