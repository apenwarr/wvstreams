/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2005 Net Integration Technologies, Inc.
 *
 * Wrapper class for WvFile for automic file creation.  Any files that
 *  are guaranteed to be automic will completely write over any existing
 *  file on close.
*/

#include "wvatomicfile.h"
#include "wvfileutils.h"
#include "wvstrutils.h"

WvAtomicFile::WvAtomicFile(int rwfd)
    : WvFile(rwfd), atomic(false)
{
}

WvAtomicFile::WvAtomicFile(WvStringParm filename, int mode, int create_mode)
{
    open(filename, mode, create_mode);
}

WvAtomicFile::~WvAtomicFile()
{
    close();
}


/* Mimics behaviour of wvfile except that it uses a tmp file and stores the
   real name */
bool WvAtomicFile::open(WvStringParm filename, int mode, int create_mode)
{
    atomic_file = filename;
    atomic = true;

    if (mode & O_RDWR || mode & O_WRONLY)
        writable = true;
    
    if (writable)
    { 
        struct stat old_file;
        int fexists = lstat(atomic_file, &old_file);
        if (!fexists && !S_ISREG(old_file.st_mode))
        {
            close();
            unlink(tmp_file);
            return false;
        }

        tmp_file = WvString("%s/nitixXXXXXX", getdirname(filename));
        tmpfd = mkstemp(tmp_file.edit());
        fcntl(tmpfd, F_SETFL, mode);

        if (!WvFile::open(tmpfd))
            return false;

        if (!fexists && (mode & O_APPEND))
        {
            // copy the contents from one file to another
            int fd = open(atomic_file, O_RDONLY | O_NONBLOCK);
            char buffer[256];
            int count;
            while ((count = ::read(fd, buffer, 256)) > 0)
                ::write(tmpfd, buffer, count);
            ::close(fd);
        }
        wvchmod(tmp_file, create_mode);
    }
    else
    {
        atomic = false;
        return WvFile::open(filename, mode, create_mode);
    }

    // retain rights
    //chown(tmp_file, old_file.st_uid, old_file.st_gid);
    return true;
}

void WvAtomicFile::close()
{
    WvFdStream::close();

    // this value is only filled if we OPENED the file otherwise
    //  we mimic the behaviour of a WvFile
    if (atomic)
        if (rename(tmp_file, atomic_file) < 0)
            ::unlink(tmp_file);

}
