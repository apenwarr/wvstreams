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

WvAtomicFile::WvAtomicFile(WvStringParm filename, mode_t create_mode)
    : tmp_file(WvString::null)
{
    open(filename, create_mode);
}

WvAtomicFile::~WvAtomicFile()
{
    close();
}


/* Mimics behaviour of wvfile except that it uses a tmp file and stores the
   real name */
bool WvAtomicFile::open(WvStringParm filename, mode_t create_mode)
{
    close();

    atomic_file = filename;

    // Ensure that if the file exists it is a regular file
    struct stat st;
    if (lstat(atomic_file, &st) == 0 && !S_ISREG(st.st_mode))
        return false;
 
    WvString new_tmp_file("%s/WvAtomicFile-XXXXXX", getdirname(filename));
    
    // Get the current umask and guarantee that mkstemp() creates
    // a file with maximal restrictions
    mode_t old_umask = ::umask(077);
    int tmp_fd = ::mkstemp(new_tmp_file.edit());
    ::umask(old_umask);
    if (tmp_fd == -1)
         return false;
 
    // Set the permissions as specified using the original umask
    // We will only possibly be adding permissions here...
    ::fchmod(tmp_fd, create_mode & ~old_umask);

    if (!WvFile::open(tmp_fd))
    {
        ::close(tmp_fd);
        return false;
    }
    
    tmp_file = new_tmp_file;

    return true;
}

void WvAtomicFile::close()
{
    if (!tmp_file) return;

    WvFdStream::close();

    if (::rename(tmp_file, atomic_file) == -1)
        ::unlink(tmp_file);
    
    tmp_file = WvString::null;
}

bool WvAtomicFile::chmod(mode_t mode)
{
    if (getfd() == -1) return false;
    
    if (fchmod(getfd(), mode) == -1)
    {
    	seterr(errno);
    	return false;
    }
    
    return true;
}

bool WvAtomicFile::chown(uid_t owner, gid_t group)
{
    if (getfd() == -1) return false;
    
    if (fchown(getfd(), owner, group) == -1)
    {
    	seterr(errno);
    	return false;
    }
    
    return true;
}
