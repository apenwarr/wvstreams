/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * Unified support for streams, that is, sequences of bytes that may or
 * may not be ready for read/write at any given time.
 * 
 * We provide typical read and write routines, as well as a select() function
 * for each stream and a WvStreamList::select() for multiple simultaneous
 * streams.
 */
#ifndef __WVSTREAM_H
#define __WVSTREAM_H

#include "wvstring.h"
#include "wvbuffer.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

class WvStreamList;
class WvAddr;

class WvStream
{
public:
    typedef int Callback(WvStream &s, void *userdata);

    // constructor to create a WvStream from an existing file descriptor.
    // The file descriptor is closed automatically by the destructor.  If
    // this is undesirable, duplicate it first using dup().
    WvStream(int _fd)
        { init(); fd = _fd; }
    virtual ~WvStream();
    
    // close the stream if it is open; isok() becomes false from now on.
    // Note!!  If you override this function in a derived class, you must
    //   call it yourself from your destructor.  WvStream::~WvStream()
    //   can only call WvStream::close() because of the way virtual
    //   functions work in C++.
    virtual void close();
    
    // return the Unix file descriptor associated with this stream
    virtual int getfd() const;
    
    // return true if the stream is actually usable right now
    virtual bool isok() const;
    
    // if isok() is false, return the system error number corresponding to
    // the error, -1 for a special error string (which you can obtain with
    // errstr()) or 0 on end of file.  If isok() is true, returns an
    // undefined number.
    virtual int geterr() const;
    virtual const char *errstr() const;
    
    // read or write a data block on the stream.  Returns the actual amount
    // read/written.
    size_t read(void *buf, size_t count);
    size_t write(const void *buf, size_t count)
        { return uwrite(buf, count); }  // no write buffer
    
    // read up to one line of data from the stream and return a pointer
    // to the internal buffer containing this line.  If the end-of-line \n
    // is encountered, it is removed from the string.  If wait_msec times
    // out before the end of line is found, returns NULL and the line may
    // be returned later.
    //
    // If wait_msec < 0, waits forever for a newline (bad idea!)
    // If wait_msec=0, never waits.  Otherwise, waits up to wait_msec
    // milliseconds until a newline appears.
    //
    // This now uses the dynamic-sized WvBuffer.  It is expected that there
    // will be no NULL characters on the line.
    //
    char *getline(time_t wait_msec, char separator = '\n');
    
    // force read() to not return any bytes unless 'count' bytes can be
    // read at once.  (Useful for processing Content-Length headers,
    // etc.)  Use count==0 to disable this feature.  getline() sets it to 0
    // automatically.
    void queuemin(size_t count)
        { queue_min = count; }
    
    // drain the input buffer (read and discard data until select(0)
    // returns false)
    void drain();
    
    // print a preformatted WvString to the stream
    size_t write(const WvString &s)
        { return write(s.str, strlen(s.str)); }

    // add appropriate fd to rfd, wfd, and efd sets if this stream can be
    // group-select()ed; returns true if the stream is known to _already_
    // be ready for one of the requested operations, in which case the
    // caller should not do an actual select().  This function is only
    // called for a stream where isok() returns true.
    virtual bool select_setup(fd_set &r, fd_set &w, fd_set &x, int &max_fd,
			      bool readable, bool writable, bool isexception);
    
    // return 'true' if this object is in the sets r, w, or x.  Called
    // from within select() to see if the object matches.
    virtual bool test_set(fd_set &r, fd_set &w, fd_set &x);

    // return true if any of the requested features are true on the stream.
    // If msec_timeout < 0, waits forever (bad idea!).  ==0, does not wait.
    // Otherwise, waits for up to msec_timeout milliseconds.
    //
    // Certain stream types may want to redefine this function.  The default
    // calls standard Unix select() on the 'fd' member.
    //
    virtual bool select(time_t msec_timeout,
			bool readable = true, bool writable = false,
			bool isexception = false);

    // get the remote address from which the last data block was received.
    // May be NULL.  The pointer becomes invalid upon the next call to read().
    virtual const WvAddr *src() const;
    
    // define the callback function for this stream, called whenever
    // the callback() member is run, and passed the 'userdata' pointer.
    void setcallback(Callback *_callfunc, void *_userdata)
        { callfunc = _callfunc; userdata = _userdata; }
    
    // set the callback function for this stream to an internal routine
    // that auto-forwards all incoming stream data to the given output
    // stream.
    void autoforward(WvStream &s)
        { callfunc = autoforward_callback; userdata = &s; }
    static int autoforward_callback(WvStream &s, void *userdata);
    
    // if the stream has a callback function defined, call it now.
    int callback()
        { return callfunc ? callfunc(*this, userdata) : 0; }
    
    // preformat and print a string.
    size_t print(const WvString &s)
        { return write(s); }
    inline size_t print(WVSTRING_FORMAT_DECL)
	{ return write(WvString(WVSTRING_FORMAT_CALL)); }
    size_t operator() (const WvString &s)
        { return write(s); }
    inline size_t operator() (WVSTRING_FORMAT_DECL)
        { return write(WvString(WVSTRING_FORMAT_CALL)); }

private:
    void init();
    
protected:
    Callback *callfunc;
    void *userdata;
    int fd, errnum;
    WvString errstring;
    WvBuffer inbuf;
    bool select_ignores_buffer;
    size_t queue_min;

    // plain internal constructor to just set up internal variables.
    WvStream()
        { init(); fd = -1; }
    
    // unbuffered I/O functions; these can ignore the buffer, which is
    // handled better by read() and write().
    virtual size_t uread(void *buf, size_t count);
    virtual size_t uwrite(const void *buf, size_t count);
    
    // set the errnum variable and close the stream -- we have an error.
    void seterr(int _errnum);
    void seterr(const WvString &specialerr);
};


// WvFile implements a stream connected to a file or Unix device.  We
// include no support for operations like seek().  Since files are not
// really streams, you probably do not need WvStream support for seekable
// files; just use standard C I/O functions in that case.
//
// WvFile is primarily useful for Unix device files, which have defined
// select() behaviour for example.
class WvFile : public WvStream
{
public:
    WvFile()
        { }
    WvFile(const WvString &filename, int mode, int create_mode = 0666)
        { open(filename, mode, create_mode); }
    bool open(const WvString &filename, int mode, int create_mode = 0666);
};


// Create the WvStreamListBase class - a simple linked list of WvStreams
DeclareWvList3(WvStream, WvStreamListBase, );

class WvStreamList : public WvStream, public WvStreamListBase
{
public:
    WvStreamList();
    virtual bool isok() const;
    virtual bool select_setup(fd_set &r, fd_set &w, fd_set &x, int &max_fd,
			      bool readable, bool writable, bool isexception);
    virtual bool test_set(fd_set &r, fd_set &w, fd_set &x);
    WvStream *select_one(int msec_timeout, bool readable = true,
			 bool writable = false, bool isexception = false);
    
protected:
    bool sets_valid;
    fd_set sel_r, sel_w, sel_x;
    WvStream *sure_thing;
    static Callback dist_callback;
};


// console stream, typically a WvSplitStream made from fd's 0 and 1.  This
// can be reassigned while the program is running, if desired, but MUST NOT
// be NULL.
extern WvStream *wvcon;


#endif // __WVSTREAM_H
