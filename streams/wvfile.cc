/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 * 
 * A simple class to access filesystem files using WvStreams.
 */
#include "wvfile.h"
#include "wvmoniker.h"

WvFile::WvFile()
{
    readable = writable = false;
}

#ifndef _WIN32 // meaningless to do this on win32
/*
 * The Win32 runtime library doesn't provide fcntl so we can't
 * set readable and writable reliably. Use the other constructor.
 */
WvFile::WvFile(int rwfd) : WvFDStream(rwfd)
{
    if (rwfd > -1)
    {
	/* We have to do it this way since O_RDONLY is defined as 0
	    in linux. */
	mode_t xmode = fcntl(rwfd, F_GETFL);
	xmode = xmode & (O_RDONLY | O_WRONLY | O_RDWR);
	readable = (xmode == O_RDONLY) || (xmode == O_RDWR);
	writable = (xmode == O_WRONLY) || (xmode == O_RDWR);
    }
    else
	readable = writable = false;
}
#endif


WvFile::WvFile(WvStringParm filename, int mode, int create_mode)
{
    open(filename, mode, create_mode);
}


static IWvStream *increator(WvStringParm s)
{
    return new WvFile(s, O_RDONLY, 0666);
}

static IWvStream *outcreator(WvStringParm s)
{
    return new WvFile(s, O_WRONLY|O_CREAT|O_TRUNC, 0666);
}

static IWvStream *creator(WvStringParm s)
{
    return new WvFile(s, O_RDWR|O_CREAT, 0666);
}

static WvMoniker<IWvStream> reg0("infile", increator);
static WvMoniker<IWvStream> reg1("outfile", outcreator);
static WvMoniker<IWvStream> reg3("file", creator);

bool WvFile::open(WvStringParm filename, int mode, int create_mode)
{
    noerr();
    
    /* We have to do it this way since O_RDONLY is defined as 0
       in linux. */
    int xmode = (mode & (O_RDONLY | O_WRONLY | O_RDWR));
    readable = (xmode == O_RDONLY) || (xmode == O_RDWR);
    writable = (xmode == O_WRONLY) || (xmode == O_RDWR);

    // don't do the default force_select of read if we're not readable!
    if (!readable)
	undo_force_select(true, false, false);
    
    close();
    #ifndef _WIN32
    int rwfd = ::open(filename, mode | O_NONBLOCK, create_mode);
    #else
    int rwfd = ::_open(filename, mode | O_NONBLOCK, create_mode);
    #endif
    if (rwfd < 0)
    {
	seterr(errno);
	return false;
    }
    setfd(rwfd);
    fcntl(rwfd, F_SETFD, 1);

    closed = stop_read = stop_write = false;
    return true;
}

#ifndef _WIN32  // since win32 doesn't support fcntl

bool WvFile::open(int _rwfd)
{
    noerr();
    if (_rwfd < 0)
        return false;

    noerr();
    close();
    
    int mode = fcntl(_rwfd, F_GETFL);
    int xmode = (mode & (O_RDONLY | O_WRONLY | O_RDWR));
    readable = (xmode == O_RDONLY) || (xmode == O_RDWR);
    writable = (xmode == O_WRONLY) || (xmode == O_RDWR);

    if (!readable)
	undo_force_select(true, false, false);

    setfd(_rwfd);
    fcntl(_rwfd, F_SETFL, mode | O_NONBLOCK);
    fcntl(_rwfd, F_SETFD, 1);

    closed = stop_read = stop_write = false;
    return true;
}

#endif 

// files not open for read are never readable; files not open for write
// are never writable.
void WvFile::pre_select(SelectInfo &si)
{
    SelectRequest oldwant = si.wants;    
    if (!readable) si.wants.readable = false;
    if (!writable) si.wants.writable = false;

    WvFDStream::pre_select(si);    
    si.wants = oldwant;
}
