/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Provides basic streaming I/O support.
 */ 
#ifndef __WVSTREAM_H
#define __WVSTREAM_H

#include <xplc/utils.h>
#include "wverror.h"
#include "wvbuf.h"
#include "wvcallback.h"
#include "wvtimeutils.h"
#include <errno.h>
#include <limits.h>

#ifdef _WIN32
#include <time.h>
#include <Winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h> // not strictly necessary, but EVERYBODY uses this...
#include <sys/time.h>
#endif

class WvAddr;
class WvStream;

// parameters are: owning-stream, userdata
typedef WvCallback<void, WvStream&, void*> WvStreamCallback;

class IWvStream : public WvErrorBase, public IObject
{
public:
    /**
     * A SelectRequest is a convenient way to remember what we want to do
     * to a particular stream: read from it, write to it, or check for
     * exceptions.
     */
    struct SelectRequest {
	bool readable, writable, isexception;
	
	SelectRequest() { }
	SelectRequest(bool r, bool w, bool x = false)
		{ readable = r; writable = w; isexception = x; }
	
	SelectRequest &operator |= (const SelectRequest &r)
		{ readable |= r.readable; writable |= r.writable;
		    isexception |= r.isexception; return *this; }
    };
    
    /**
     * the data structure used by pre_select()/post_select() and internally
     * by select().
     */
    struct SelectInfo {
	fd_set read, write, except; // set by pre_select, read by post_select
	SelectRequest wants;        // what is the user looking for?
	int max_fd;                 // largest fd in read, write, or except
	time_t msec_timeout;        // max time to wait, or -1 for forever
	bool inherit_request;       // 'wants' values passed to child streams
	bool global_sure;           // should we run the globalstream callback
    };
    
    IWvStream();
    virtual ~IWvStream();
    virtual void close() = 0;
    virtual bool isok() const = 0;
    virtual void callback() = 0;
    
    // FIXME: these really have no place in the interface...
    virtual int getrfd() const = 0;
    virtual int getwfd() const = 0;

    // FIXME: evil, should go away (or be changed to localaddr/remoteaddr)
    virtual const WvAddr *src() const = 0;
    
    // needed for select().
    // Some say that pre_select() should go away.
    virtual bool pre_select(SelectInfo &si) = 0;
    virtual bool post_select(SelectInfo &si) = 0;
    
    // these are now the official way to get/put data to your stream.
    // The old uread() and uwrite() are just implementation details!
    virtual size_t read(void *buf, size_t count) = 0;
    virtual size_t write(const void *buf, size_t count) = 0;

    // FIXME: these are the new fancy versions, but WvBuf needs to have
    // a safely abstracted interface class (IWvBuf) before IWvStream will
    // really be safe, if we include these.
    virtual size_t read(WvBuf &outbuf, size_t count) = 0;
    virtual size_t write(WvBuf &inbuf, size_t count = INT_MAX) = 0;

    /**
     * Shuts down the reading side of the stream.  This is the opposite
     * of nowrite(), but the name is actually slightly misleading; subsequent
     * calls to read() *might not* fail; rather, if the other end of the
     * connection tries to write to us, they should fail.
     *
     * After noread(), if the read buffer (if any) is empty once, we promise
     * that it will never refill.
     * 
     * If you call both noread() and nowrite(), then the stream does close()
     * automatically once all buffers are empty.
     */
    virtual void noread() = 0;

    /**
     * Shuts down the writing side of the stream.
     * Subsequent calls to write() will fail.  But if there's data in the
     * output buffer, it will still be flushed.
     * 
     * If you call both noread() and nowrite(), then the stream does close()
     * automatically once all buffers are empty.
     */
    virtual void nowrite() = 0;
    
    /**
     * Auto-close the stream if the time is right.  If noread() and nowrite()
     * and all buffers are empty, then we can probably close.
     */
    virtual void maybe_autoclose() = 0;
    
    /** Returns true if the stream is readable. */
    virtual bool isreadable() = 0;
    
    /** Returns true if the stream is writable (without using the outbuf). */
    virtual bool iswritable() = 0;
    
    /**
     * flush the output buffer, if we can do it without delaying more than
     * msec_timeout milliseconds at a time.  (-1 means wait forever)
     * 
     * Returns true if it finished flushing (ie. the outbuf is empty).
     * 
     * FIXME: Something like this probably belongs in IWvStream, but
     * probably not exactly this.
     */
    virtual bool flush(time_t msec_timeout) = 0;

    /**
     * Returns true if we want to flush the output buffer right now.  This
     * allows us to implement delayed_flush(), flush_then_close(), etc, but
     * it's still super-ugly and probably needs to go away.  (In fact, all
     * our buffer flushing is super-ugly right now.)
     */
    virtual bool should_flush() = 0;

    /** Sets a callback to be invoked on close().  */
    virtual void setclosecallback(WvStreamCallback _callfunc,
				  void *_userdata) = 0;
};

#ifndef SWIG
DEFINE_IID(IWvStream, {0x7ca76e98, 0xb653, 0x43d7,
    {0xb0, 0x56, 0x8b, 0x9d, 0xde, 0x9a, 0xbe, 0x9d}});
#endif

/**
 * Unified support for streams, that is, sequences of bytes that may or
 * may not be ready for read/write at any given time.
 * 
 * We provide typical read and write routines, as well as a select() function
 * for each stream.
 */
class WvStream: public IWvStream
{
    IMPLEMENT_IOBJECT(WvStream);
public:
    /**
     * 'force' is the list of default SelectRequest values when you use the
     * variant of select() that doesn't override them.
     */
    SelectRequest force;
    
    /**
     * If this is set, select() doesn't return true for read unless the
     * given stream also returns true for write.
     */
    WvStream *read_requires_writable;

    /**
     * If this is set, select() doesn't return true for write unless the
     * given stream also returns true for read.
     */
    WvStream *write_requires_readable;
    
    /** If this is set, enables the use of continue_select(). */
    bool uses_continue_select;

    /** Specifies the stack size to reserve for continue_select(). */
    size_t personal_stack_size;

    /**
     * This will be true during callback execution if the
     * callback was triggered by the alarm going off.
     */
    bool alarm_was_ticking;
    
    /** True if noread()/nowrite()/close() have been called, respectively. */
    bool stop_read, stop_write, closed;
    
    /** Basic constructor for just a do-nothing WvStream */
    WvStream();
    virtual ~WvStream();

    /**
     * Close the stream if it is open; isok() becomes false from now on.
     * Note!!  If you override this function in a derived class, you must
     *   call it yourself from your destructor.  WvStream::~WvStream()
     *   can only call WvStream::close() because of the way virtual
     *   functions work in C++.
     */ 
    virtual void close();

    /** Override seterr() from WvError so that it auto-closes the stream. */
    virtual void seterr(int _errnum);
    void seterr(WvStringParm specialerr)
        { WvErrorBase::seterr(specialerr); }
    void seterr(WVSTRING_FORMAT_DECL)
        { seterr(WvString(WVSTRING_FORMAT_CALL)); }
    
    /** return true if the stream is actually usable right now */
    virtual bool isok() const;
    
    /** read a data block on the stream.  Returns the actual amount read. */
    virtual size_t read(void *buf, size_t count);

    /**
     * Read exactly count bytes from the stream.
     *
     * Notes:
     *      must be using continue_select to use this function.
     *      if timeout strikes or !isok() before count bytes could be read,
     *          nothing is read and 0 is returned.
     *      resets queuemin to 0.
     * 
     * FIXME: yes, that means if the stream closes, continue_read might not
     * read the last bit of data.  You can use read() for that if you want.
     */
    virtual size_t continue_read(time_t wait_msec, void *buf, size_t count);

    /** Read exactly count bytes from the stream, using continue_select(). */
    virtual size_t continue_read(time_t wait_msec, WvBuf &outbuf,
				 size_t count);

    /**
     * Reads up to 'count' bytes of data from the stream into the buffer.
     * Returns the actual amount read.
     *
     * If 'count' is greater than the amount of free space available
     * in the buffer, only reads at most that amount.  You should
     * specify a reasonable upper bound on how much data should
     * be read at once.
     */
    virtual size_t read(WvBuf &outbuf, size_t count);

    /** 
     * Puts data back into the stream's internal buffer.  We cheat so that
     * there's no restriction on how much (or what) data can be unread().
     * This is different from WvBuf::unget() (which is rather restrictive).
     */
    virtual void unread(WvBuf &outbuf, size_t count);

    /**
     * Write data to the stream.  Returns the actual amount written.
     * Since WvStream has an output buffer, it *always* successfully "writes"
     * the full amount (but you might have to flush the buffers later so it
     * actually gets sent).
     */
    virtual size_t write(const void *buf, size_t count);

    /**
     * Writes data to the stream from the given buffer.
     * Returns the actual amount written.
     *
     * If count is greater than the amount of data available in
     * the buffer, only writes at most that amount.
     */
    virtual size_t write(WvBuf &inbuf, size_t count = INT_MAX);

    /**
     * set the maximum size of outbuf, beyond which a call to write() will
     * return 0.  I need to do this for tape backups, since all I can do
     * is write to the loopback as fast as I can, which causes us to run 
     * out of memory and get SIGABRT'd.  (dcoombs: 12/15/2000)
     * 
     * FIXME: there must be a better way.  This confuses the semantics of
     * write(); can you trust it to always write all the bytes, or not?
     */
    void outbuf_limit(size_t size)
        { max_outbuf_size = size; }

    virtual void noread();
    virtual void nowrite();
    virtual void maybe_autoclose();
    
    virtual bool isreadable();
    virtual bool iswritable();
    
    /**
     * unbuffered I/O functions; these ignore the buffer, which is
     * handled by read().  Don't call these functions explicitly unless
     * you have a _really_ good reason.
     * 
     * This is what you would override in a derived class.
     */ 
    virtual size_t uread(void *buf, size_t count)
        { return 0; /* basic WvStream doesn't actually do anything! */ }

    /**
     * unbuffered I/O functions; these ignore the buffer, which is
     * handled by write().  Don't call these functions explicitly unless
     * you have a _really_ good reason.
     * 
     * This is what you would override in a derived class.
     */ 
    virtual size_t uwrite(const void *buf, size_t count)
        { return count; /* basic WvStream doesn't actually do anything! */ }
    
    /**
     * read up to one line of data from the stream and return a pointer
     * to the internal buffer containing this line.  If the end-of-line
     * 'separator' is encountered, it is removed from the string.  If
     * wait_msec times out before the end of line is found, returns NULL and
     * the line may be returned next time, or you can read what we have so
     * far by calling read().
     *
     * If wait_msec < 0, waits forever for a newline (often a bad idea!)
     * If wait_msec=0, never waits.  Otherwise, waits up to wait_msec
     * milliseconds until a newline appears.
     *
     * Readahead specifies the maximum amount of data that the stream is
     * allowed to read in one shot.
     *
     * It is expected that there will be no NULL characters on the line.
     * 
     * If uses_continue_select is true, getline() will use continue_select()
     * rather than select() to wait for its timeout.
     */
    char *getline(time_t wait_msec, char separator = '\n',
		  int readahead = 1024);
    
    /**
     * read up to count characters into buf, up to and including the first
     * instance of separator.
     * 
     * if separator is not found on input before timeout (usual symantics)
     * or stream close or error, or if count is 0, nothing is placed in buf
     * and 0 is returned.
     * 
     * if your buffer is not large enough for line, call multiple times
     * until seperator is found at end of buffer to retrieve the entire
     * line.
     * 
     * Returns the number of characters that were put in buf.
     * 
     * If uses_continue_select is true, getline() will use
     * continue_select() rather than select() to wait for its timeout.
     */
    size_t read_until(void *buf, size_t count, time_t wait_msec,
                      char separator);

    /**
     * force read() to not return any bytes unless 'count' bytes can be
     * read at once.  (Useful for processing Content-Length headers, etc.)
     * Use count==0 to disable this feature.
     * 
     * WARNING: getline() sets queuemin to 0 automatically!
     */ 
    void queuemin(size_t count)
        { queue_min = count; }

    /**
     * drain the input buffer (read and discard data until select(0)
     * returns false)
     */
    void drain();
    
    /**
     * force write() to always buffer output.  This can be more efficient
     * if you write a lot of small segments and want to "coagulate" them
     * automatically.  To flush the output buffer, use flush() or select().
     */ 
    void delay_output(bool is_delayed)
    {
        outbuf_delayed_flush = is_delayed;
        want_to_flush = !is_delayed;
    }

    /**
     * if true, force write() to call flush() each time, the default behavour.
     * otherwise, flush() is granted special meaning when explicitly invoked
     * by the client and write() may empty the output buffer, but will not
     * explicitly flush().
     */
    void auto_flush(bool is_automatic)
        { is_auto_flush = is_automatic; }

    /**
     * flush the output buffer, if we can do it without delaying more than
     * msec_timeout milliseconds at a time.  (-1 means wait forever)
     * 
     * Returns true if the flushing finished (the output buffer is empty).
     */
    virtual bool flush(time_t msec_timeout);

    virtual bool should_flush();

    /**
     * flush the output buffer automatically as select() is called.  If
     * the buffer empties, close the stream.  If msec_timeout seconds pass,
     * close the stream.  After the stream closes, it will become !isok()
     * (and a WvStreamList can delete it automatically)
     */ 
    void flush_then_close(int msec_timeout);
    
    /**
     * pre_select() sets up for eventually calling ::select().
     * It adds the right fds to the read, write, and except lists in the
     * SelectInfo struct.
     * 
     * Returns true if we already know this stream is ready, and there's no
     * need to actually do a real ::select().  Some streams, such as timers,
     * can be implemented by _only_ either returning true or false here after
     * doing a calculation, and never actually adding anything to the
     * SelectInfo.
     * 
     * You can add your stream to any of the lists even if readable,
     * writable, or isexception isn't set.  This is what force_select()
     * does.  You can also choose not to add yourself to the list if you know
     * it would be useless right now.
     * 
     * pre_select() is only called if isok() is true.
     * 
     * pre_select() is allowed to reduce msec_timeout (or change it if it's
     * -1).  However, it's not allowed to _increase_ msec_timeout.
     */ 
    virtual bool pre_select(SelectInfo &si);
    
    /**
     * A more convenient version of pre_select() usable for overriding the
     * 'want' value temporarily.
     */
    bool pre_select(SelectInfo &si, const SelectRequest &r)
    {
	SelectRequest oldwant = si.wants;
	si.wants = r;
	bool val = pre_select(si);
	si.wants = oldwant;
	return val;
    }
    
    /**
     * Like pre_select(), but still exists even if you override the other
     * pre_select() in a subclass.  Sigh.
     */
    bool xpre_select(SelectInfo &si, const SelectRequest &r)
        { return pre_select(si, r); }
    
    /**
     * post_select() is called after ::select(), and returns true if this
     * object is now ready.  Usually this is done by checking for this object
     * in the read, write, and except lists in the SelectInfo structure.  If
     * you want to do it in some other way, you should usually do it in
     * pre_select() instead.
     * 
     * You may also want to do extra maintenance functions here; for example,
     * the standard WvStream::post_select tries to flush outbuf if it's
     * nonempty.  WvTCPConn might retry connect() if it's waiting for a
     * connection to be established.
     */
    virtual bool post_select(SelectInfo &si);

    /**
     * Like post_select(), but still exists even if you override the other
     * post_select() in a subclass.  Sigh.
     */
    bool xpost_select(SelectInfo &si, const SelectRequest &r)
        { return post_select(si, r); }
    
    /**
     * A more convenient version of post_select() usable for overriding the
     * 'want' value temporarily.
     */
    bool post_select(SelectInfo &si, const SelectRequest &r)
    {
	SelectRequest oldwant = si.wants;
	si.wants = r;
	bool val = post_select(si);
	si.wants = oldwant;
	return val;
    }
    
    /**
     * Return true if any of the requested features are true on the stream.
     * If msec_timeout < 0, waits forever (bad idea!).  ==0, does not wait.
     * Otherwise, waits for up to msec_timeout milliseconds.
     * 
     * **NOTE**
     *   select() is _not_ virtual!  To change the select() behaviour
     *   of a stream, override the pre_select() and/or post_select()
     *   functions.
     * 
     * This version of select() sets forceable==true, so force_select
     * options are taken into account.
     * 
     * You almost always use this version of select() with callbacks, like
     * this:  if (stream.select(1000)) stream.callback();
     * 
     * If you want to read/write the stream in question, try using the other
     * variant of select().
     * 
     * DEPRECATED.  Call runonce() instead.
     */
    bool select(time_t msec_timeout)
        { return _select(msec_timeout, false, false, false, true); }
    
    /**
     * Exactly the same as:
     *     if (select(timeout)) callback();
     * 
     * ...except that the above is deprecated, because it assumes callbacks
     * aren't called automatically and that the return value of one-parameter
     * select() is actually meaningful.
     * 
     * Update your main loop to call runonce() instead of the above.
     * 
     * Almost all modern programs should use msec_timeout = -1.
     */
    void runonce(time_t msec_timeout = -1)
        { if (select(msec_timeout)) callback(); }
    
     /**
      * This version of select() sets forceable==false, so we use the exact
      * readable/writable/isexception options provided.
      * 
      * You normally use this variant of select() when deciding whether you
      * should read/write a particular stream.  For example:
      * 
      *     if (stream.select(1000, true, false))
      *             len = stream.read(buf, sizeof(buf));
      * 
      * This variant of select() is probably not what you want with
      * most WvStreamLists, unless you know exactly what you're doing.
      * 
      * WARNING: the difference between the one-parameter and multi-parameter
      * versions of select() is *incredibly* confusing.  Make sure you use the
      * right one!
      * 
      * DEPRECATED.  Call isreadable() or iswritable() instead, if
      * msec_timeout was going to be zero.  Other values of msec_timeout are
      * not really recommended anyway.
      */
    bool select(time_t msec_timeout,
		bool readable, bool writable, bool isex = false)
        { return _select(msec_timeout, readable, writable, isex, false); }
    
    /**
     * Use force_select() to force one or more particular modes (readable,
     * writable, or isexception) to true when selecting on this stream.
     * 
     * If an option is set 'true', we will select on that option when someone
     * does a select().  If it's set 'false', we don't change its force
     * status.  (To de-force something, use undo_force_select().)
     */
    void force_select(bool readable, bool writable, bool isexception = false);
    
    /**
     * Undo a previous force_select() - ie. un-forces the options which
     * are 'true', and leaves the false ones alone.
     */
    void undo_force_select(bool readable, bool writable,
			   bool isexception = false);
    
    /**
     * return to the caller from execute(), but don't really return exactly;
     * this uses WvCont::yield() to return to the caller of callback()
     * without losing our place in execute() itself.  So, next time someone
     * calls callback(), it will be as if continue_select() returned.
     * 
     * NOTE: execute() will won't be called recursively this way, but any
     * other member function might get called, or member variables changed,
     * or the state of the world updated while continue_select() runs.  Don't
     * assume that nothing has changed after a call to continue_select().
     * 
     * NOTE 2: if you're going to call continue_select(), you should set
     * uses_continue_select=true before the first call to callback().
     * Otherwise your WvCont won't get created.
     * 
     * NOTE 3: if msec_timeout >= 0, this uses WvStream::alarm().
     */
    bool continue_select(time_t msec_timeout);
    
    /**
     * you MUST run this from your destructor if you use continue_select(), or
     * very weird things will happen if someone deletes your object while in
     * continue_select().
     */
    void terminate_continue_select();

    /**
     * get the remote address from which the last data block was received.
     * May be NULL.  The pointer becomes invalid upon the next call to read().
     */
    virtual const WvAddr *src() const;
    
    /**
     * define the callback function for this stream, called whenever
     * the callback() member is run, and passed the 'userdata' pointer.
     */
    void setcallback(WvStreamCallback _callfunc, void *_userdata);
        
    /** Sets a callback to be invoked on close().  */
    void setclosecallback(WvStreamCallback _callfunc, void *_userdata);

    /**
     * set the callback function for this stream to an internal routine
     * that auto-forwards all incoming stream data to the given output
     * stream.
     */
    void autoforward(WvStream &s);

    /** Stops autoforwarding. */
    void noautoforward();
    static void autoforward_callback(WvStream &s, void *userdata);
    
    /**
     * A wrapper that's compatible with WvCont, but calls the "real" callback.
     */
    void *_callwrap(void *);
    
    /**
     * Actually call the registered callfunc and execute().
     */
    void _callback();
    
    /**
     * if the stream has a callback function defined, call it now.
     * otherwise call execute().
     */
    virtual void callback();
    
    /**
     * set an alarm, ie. select() will return true after this many ms.
     * The alarm is cleared when callback() is called.
     */
    void alarm(time_t msec_timeout);

    /**
     * return the number of milliseconds remaining before the alarm will go
     * off; -1 means no alarm is set (infinity), 0 means the alarm has
     * been hit and will be cleared by the next callback().
     */
    time_t alarm_remaining();

    /**
     * print a preformatted WvString to the stream.
     * see the simple version of write() way up above.
     */
    size_t write(WvStringParm s)
        { return write(s.cstr(), s.len()); }
    size_t print(WvStringParm s)
        { return write(s); }
    size_t operator() (WvStringParm s)
        { return write(s); }

    /** preformat and write() a string. */
    size_t print(WVSTRING_FORMAT_DECL)
	{ return write(WvString(WVSTRING_FORMAT_CALL)); }
    size_t operator() (WVSTRING_FORMAT_DECL)
        { return write(WvString(WVSTRING_FORMAT_CALL)); }

protected:
    // builds the SelectInfo data structure (runs pre_select)
    // returns true if there are callbacks to be dispatched
    //
    // all of the fields are filled in with new values
    // si.msec_timeout contains the time until the next alarm expires
    bool _build_selectinfo(SelectInfo &si, time_t msec_timeout,
        bool readable, bool writable, bool isexcept,
        bool forceable);

    // runs the actual select() function over the given
    // SelectInfo data structure, returns the number of descriptors
    // in the set, and sets the error code if a problem occurs
    int _do_select(SelectInfo &si);

    // processes the SelectInfo data structure (runs post_select)
    // returns true if there are callbacks to be dispatched
    bool _process_selectinfo(SelectInfo &si, bool forceable);

    // tries to empty the output buffer if the stream is writable
    // not quite the same as flush() since it merely empties the output
    // buffer asynchronously whereas flush() might have other semantics
    // also handles autoclose (eg. after flush)
    bool flush_outbuf(time_t msec_timeout);

    // called once flush() has emptied outbuf to ensure that any other
    // internal stream buffers actually do get flushed before it returns
    virtual bool flush_internal(time_t msec_timeout);
    
    // the real implementations for these are actually in WvFDStream, which
    // is where they belong.  By IWvStream needs them to exist for now, so
    // it's a hack.  In standard WvStream they return -1.
    virtual int getrfd() const;
    virtual int getwfd() const;
    
private:
    /** The function that does the actual work of select(). */
    bool _select(time_t msec_timeout,
		 bool readable, bool writable, bool isexcept,
		 bool forceable);


protected:
    WvDynBuf inbuf, outbuf;
    WvStreamCallback callfunc, closecb_func;
    WvCallback<void*,void*> call_ctx;
    void *userdata;
    void *closecb_data;
    size_t max_outbuf_size;
    bool outbuf_delayed_flush;
    bool is_auto_flush;

    // Used to guard against excessive flushing when using delay_flush
    bool want_to_flush;

    // Used to ensure we don't flush recursively.
    bool is_flushing;

    size_t queue_min;		// minimum bytes to read()
    time_t autoclose_time;	// close eventually, even if output is queued
    WvTime alarm_time;          // select() returns true at this time
    WvTime last_alarm_check;    // last time we checked the alarm_remaining
    bool wvstream_execute_called;
    
    /** Prevent accidental copying of WvStreams. */
    WvStream(const WvStream &s) { }
    WvStream& operator= (const WvStream &s) { return *this; }

    /**
     * The callback() function calls execute(), and then calls the user-
     * specified callback if one is defined.  Do not call execute() directly;
     * call callback() instead.
     * 
     * The default execute() function does nothing.
     * 
     * Note: If you override this function in a derived class, you must
     * call the parent execute() yourself from the derived class.
     */
    virtual void execute();
    
    // every call to select() selects on the globalstream.
    static WvStream *globalstream;
};

/**
 * Console streams...
 *
 * This can be reassigned while the program is running, if desired,
 * but MUST NOT be NULL.
 */
extern WvStream *wvcon; // tied stdin and stdout stream
extern WvStream *wvin;  // stdin stream
extern WvStream *wvout; // stdout stream
extern WvStream *wverr; // stderr stream

#endif // __WVSTREAM_H
