/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A more convenient wrapper around WvSubProc.  See wvsystem.h.
 */
#include "wvsystem.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

WvSystem::~WvSystem()
{
    go();
}


void WvSystem::init(const char * const *argv)
{
    started = false;
    WvSubProc::preparev(argv[0], argv);
}


// open a given filename or device, making sure it has the given fd.  If
// there is an open file on that fd already, it gets closed.
static void fd_open(int fd, WvStringParm file, int mode)
{
    ::close(fd);
    int nfd = ::open(file, mode, 0666);
    if (nfd < 0)
	return;
    if (nfd != fd)
    {
	::dup2(nfd, fd);
	::close(nfd);
    }
}


// overrides WvSubProc::fork().
int WvSystem::fork(int *waitfd)
{
    int pid = WvSubProc::fork(waitfd);
    if (!pid) // child
    {
	if (!fdfiles[0].isnull())
	    fd_open(0, fdfiles[0], O_RDONLY);
	if (!fdfiles[1].isnull())
	    fd_open(1, fdfiles[1], O_WRONLY|O_CREAT);
	if (!fdfiles[2].isnull())
	    fd_open(2, fdfiles[2], O_WRONLY|O_CREAT);
    }
    
    return pid;
}


int WvSystem::go()
{
    if (!started)
    {
	WvSubProc::start_again();
	started = true;
    }
    WvSubProc::wait(-1, false);
    return WvSubProc::estatus;
}


WvSystem &WvSystem::infile(WvStringParm filename)
{
    fdfiles[0] = filename;
    return *this;
}


WvSystem &WvSystem::outfile(WvStringParm filename)
{
    fdfiles[1] = filename;
    return *this;
}


WvSystem &WvSystem::errfile(WvStringParm filename)
{
    fdfiles[2] = filename;
    return *this;
}


