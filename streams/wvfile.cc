/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A simple class to access filesystem files using WvStreams.
 */
#include "wvfile.h"

bool WvFile::open(WvStringParm filename, int mode, int create_mode)
{
    if (rwfd >= 0)
	close();
    rwfd = ::open(filename, mode | O_NONBLOCK, create_mode);
    if (rwfd < 0)
    {
	seterr(errno);
	return false;
    }

    fcntl(rwfd, F_SETFD, 1);
    return true;
}
