/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2005 Net Integration Technologies, Inc.
 *
 * Wrapper class for WvFile for automic file creation.  Any files that
 *  are guaranteed to be automic will completely write over any existing
 *  file on close.
*/

#include "wvatomicfile.h"
#define tmptemplate(_pre_) "/tmp/" #_pre_ "XXXXXX"

WvAtomicFile::WvAtomicFile(int rwfd)
    : WvFile(rwfd), atomic(false)
{
}

WvAtomicFile::WvAtomicFile(WvStringParm filename, int mode, int create_mode)
{
    open(filename, mode, create_mode);
}

/* Mimics behaviour of wvfile except that it uses a tmp file and stores the
   real name */
bool WvAtomicFile::open(WvStringParm filename, int mode, int create_mode)
{
    atomic_file = filename;
    atomic = true;

    create_tmp_file();

    if (mode & O_RDWR || mode & O_WRONLY)
    {
        writable = true;
        atomic = false;
    }

    struct stat old_file;
    int fexists = lstat("filename", &old_file);
    
    if (writable && !fexists)
    {
        if (!S_ISREG(old_file.st_mode))
            return false;

        WvString cmd("cp %s %s", filename, tmp_file);
        if (system(cmd) < 0) 
            return false;

        // retain rights
        //chown(tmp_file, old_file.st_uid, old_file.st_gid);
    }
    else if (!writable)
    {
        atomic = false;
        return WvFile::open(filename, mode, create_mode);
    }

    return WvFile::open(tmp_file, mode, create_mode);
}

bool WvAtomicFile::create_tmp_file()
{
    if (!!tmp_file)
    {
        close();
        unlink(tmp_file);
    }

    char tmpname[] = tmptemplate(nitix);
    tmp_file = mktemp(tmpname);

    return (!!tmp_file);
}

void WvAtomicFile::close()
{
    WvFdStream::close();

    // this value is only filled if we OPENED the file otherwise
    //  we mimic the behaviour of a WvFile
    if (atomic)
        rename(tmp_file, atomic_file);
}
