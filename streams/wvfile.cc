/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A simple class to access filesystem files using WvStreams.
 */
#include "wvfile.h"

bool WvFile::open(const WvString &filename, int mode, int create_mode)
{
    if (fd >= 0)
	close();
    fd = ::open(filename, mode | O_NONBLOCK, create_mode);
    if (fd < 0)
    {
	seterr(errno);
	return false;
    }

    fcntl(fd, F_SETFD, 1);
    return true;
}
