/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2000 Net Integration Technologies, Inc.
 * 
 * Unified support for streams, that is, sequences of bytes that may or
 * may not be ready for read/write at any given time.
 * 
 * We provide typical read and write routines, as well as a select() function
 * for each stream.
 */
#ifndef __WVSTREAM_H
#define __WVSTREAM_H

#include "wvstring.h"
#include "wvbuffer.h"
#include <unistd.h> // not strictly necessary, but EVERYBODY uses this...
#include <sys/time.h>
#include <errno.h>

class WvAddr;
class WvTask;
class WvTaskMan;

class WvStream
{
public:
    typedef void Callback(WvStream &s, void *userdata);

    // constructor to create a WvStream from an existing file descriptor.
    // The file descriptor is closed automatically by the destructor.  If
    // this is undesirable, duplicate it first using dup().
    //
    WvStream(int _fd);
    virtual ~WvStream();
    
    // close the stream if it is open; isok() becomes false from now on.
    // Note!!  If you override this function in a derived class, you must
    //   call it yourself from your destructor.  WvStream::~WvStream()
    //   can only call WvStream::close() because of the way virtual
    //   functions work in C++.
    // 
    virtual void close();
    
    // return the Unix file descriptor associated with this stream
    virtual int getfd() const;
    
    // return true if the stream is actually usable right now
    virtual bool isok() const;
    
    // if isok() is false, return the system error number corresponding to
    // the error, -1 for a special error string (which you can obtain with
    // errstr()) or 0 on end of file.  If isok() is true, returns an
    // undefined number.
    // 
    virtual int geterr() const;
    virtual const char *errstr() const;
    
    // read or write a data block on the stream.  Returns the actual amount
    // read/written.
    size_t read(void *buf, size_t count);
    size_t write(const void *buf, size_t count);
    
    // unbuffered I/O functions; these ignore the buffer, which is
    // handled by read() and write().  Don't call these functions unless
    // you have a _really_ good reason.
    // 
    virtual size_t uread(void *buf, size_t count);
    virtual size_t uwrite(const void *buf, size_t count);
    
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
    // 
    void queuemin(size_t count)
        { queue_min = count; }
    
    // drain the input buffer (read and discard data until select(0)
    // returns false)
    void drain();
    
    // force write() to always buffer output.  This can be more efficient
    // if you write a lot of small segments and want to "coagulate" them
    // automatically.  To flush the output buffer, use flush() or select().
    // 
    void delay_output(bool is_delayed)
        { outbuf_delayed_flush = is_delayed; }
    
    // flush the output buffer, if we can do it without delaying more than
    // msec_timeout milliseconds at a time.  (-1 means wait forever)
    void flush(time_t msec_timeout);
    
    // flush the output buffer automatically as select() is called.  If
    // the buffer empties, close the stream.  If msec_timeout seconds pass,
    // close the stream.  After the stream closes, it will become !isok()
    // (and a WvStreamList can delete it automatically)
    // 
    void flush_then_close(int msec_timeout);
    
    // add appropriate fd to rfd, wfd, and efd sets if this stream can be
    // group-select()ed; returns true if the stream is known to _already_
    // be ready for one of the requested operations, in which case the
    // caller should not do an actual select().  This function is only
    // called for a stream where isok() returns true.
    // 
    struct SelectInfo {
	fd_set read, write, except;
	bool readable, writable, isexception;
	int max_fd;
	time_t msec_timeout;
    };
    virtual bool select_setup(SelectInfo &si);
    
    // return 'true' if this object is in the sets r, w, or x.  Called
    // from within select() to see if the object matches.
    // 
    virtual bool test_set(SelectInfo &si);

    // return true if any of the requested features are true on the stream.
    // If msec_timeout < 0, waits forever (bad idea!).  ==0, does not wait.
    // Otherwise, waits for up to msec_timeout milliseconds.
    // To change the select() behaviour of a stream, change its select_setup
    // and/or test_set functions.
    //
    bool select(time_t msec_timeout,
		bool readable = true, bool writable = false,
		bool isexception = false);
    
    // return to the caller from execute(), but don't really return exactly;
    // this uses WvTaskMan::yield() to return to the caller of callback()
    // without losing our place in execute() itself.  So, next time someone
    // calls callback(), it will be as if continue_select() returned.
    // 
    // NOTE: execute() will won't be called recursively this way, but any
    // other member function might get called, or member variables changed,
    // or the state of the world updated while continue_select() runs.  Don't
    // assume that nothing has changed after a call to continue_select().
    // 
    // NOTE 2: if you're going to call continue_select(), you should set
    // uses_continue_select=true before the first call to callback().
    // Otherwise your WvTask struct won't get created.
    // 
    // NOTE 3: if msec_timeout >= 0, this uses WvStream::alarm().
    // 
    bool uses_continue_select;
    size_t personal_stack_size;	// stack size to reserve for continue_select()
    bool continue_select(time_t msec_timeout);

    // get the remote address from which the last data block was received.
    // May be NULL.  The pointer becomes invalid upon the next call to read().
    // 
    virtual const WvAddr *src() const;
    
    // define the callback function for this stream, called whenever
    // the callback() member is run, and passed the 'userdata' pointer.
    // 
    void setcallback(Callback *_callfunc, void *_userdata)
        { callfunc = _callfunc; userdata = _userdata; }
    
    // set the callback function for this stream to an internal routine
    // that auto-forwards all incoming stream data to the given output
    // stream.
    void autoforward(WvStream &s)
        { callfunc = autoforward_callback; userdata = &s; }
    static void autoforward_callback(WvStream &s, void *userdata);
    
    // if the stream has a callback function defined, call it now.
    // otherwise call execute().
    // 
    void callback();
    
    // set an alarm, ie. select() will return true after this many ms.
    // The alarm is cleared when callback() is called.
    void alarm(time_t msec_timeout);
    
    // return the number of milliseconds remaining before the alarm will go
    // off; -1 means no alarm is set (infinity), 0 means the alarm has
    // been hit and will be cleared by the next callback().
    time_t alarm_remaining();
    
    // print a preformatted WvString to the stream.
    // see the simple version of write() way up above.
    size_t write(const WvString &s)
        { return write(s, strlen(s)); }

    // preformat and print a string.
    size_t print(const WvString &s)
        { return write(s); }
    size_t print(WVSTRING_FORMAT_DECL)
	{ return write(WvString(WVSTRING_FORMAT_CALL)); }
    size_t operator() (const WvString &s)
        { return write(s); }
    size_t operator() (WVSTRING_FORMAT_DECL)
        { return write(WvString(WVSTRING_FORMAT_CALL)); }

private:
    void init();
    
protected:
    Callback *callfunc;
    void *userdata;
    int fd, errnum;
    WvString errstring;
    WvBuffer inbuf, outbuf;
    bool select_ignores_buffer, outbuf_delayed_flush, alarm_was_ticking;
    size_t queue_min;		// minimum bytes to read()
    time_t autoclose_time;	// close eventually, even if output is queued
    struct timeval alarm_time;	// select() returns true at this time
    
    static WvTaskMan *taskman;
    WvTask *task;

    // plain internal constructor to just set up internal variables.
    WvStream()
        { init(); fd = -1; }
    
    // set the errnum variable and close the stream -- we have an error.
    void seterr(int _errnum);
    void seterr(const WvString &specialerr);
    
    // actually do the callback for an arbitrary stream.
    // This is a static function so we can pass it as a function pointer
    // to WvTask functions.
    static void _callback(void *stream);
    
    // if no callback function is defined, we call execute() instead.
    // the default execute() function does nothing.
    // To get this function to run, use callback().
    virtual void execute();
};


// console stream, typically a WvSplitStream made from fd's 0 and 1.  This
// can be reassigned while the program is running, if desired, but MUST NOT
// be NULL.
extern WvStream *wvcon;

#endif // __WVSTREAM_H
