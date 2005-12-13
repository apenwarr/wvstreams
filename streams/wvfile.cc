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
    skip_select = false;
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

    skip_select = false;
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

static const UUID uuid0 = {0x7a959fe1, 0xb845, 0x4520,
			  {0xbf, 0x4b, 0xc0, 0xda, 0xe4, 0xf4, 0xf4, 0x4e}};
static WvMoniker<IWvStream> reg0("infile", uuid0, increator);

static const UUID uuid1 = {0x0043cedc, 0xd748, 0x48c4,
			   {0xa1, 0x9c, 0x63, 0xfc, 0xb8, 0x8d, 0xa5, 0x24}};
static WvMoniker<IWvStream> reg1("outfile", uuid1, outcreator);

static const UUID uuid3 = {0x0c5db556, 0xe399, 0x4e80,
			   {0x80, 0x89, 0x8c, 0xc6, 0x06, 0x3a, 0x2b, 0x38}};
static WvMoniker<IWvStream> reg3("file", uuid3, creator);

bool WvFile::open(WvStringParm filename, int mode, int create_mode)
{
    noerr();
    
    /* We have to do it this way since O_RDONLY is defined as 0
       in linux. */
    int xmode = (mode & (O_RDONLY | O_WRONLY | O_RDWR));
    readable = (xmode == O_RDONLY) || (xmode == O_RDWR);
    writable = (xmode == O_WRONLY) || (xmode == O_RDWR);

    skip_select = false;
    
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

    skip_select = false;
    
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
bool WvFile::pre_select(SelectInfo &si)
{
    bool ret;
    
    SelectRequest oldwant = si.wants;
    
    if (!readable) si.wants.readable = false;
    if (!writable) si.wants.writable = false;
    ret = WvFDStream::pre_select(si);
    
    si.wants = oldwant;

    // Force select() to always return true by causing it to not wait and
    // setting our pre_select() return value to true.
    if (skip_select)
    {
	si.msec_timeout = 0;
	ret = true;
    }
    return ret;
}
