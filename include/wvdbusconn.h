/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 * Pathfinder Software:
 *   Copyright (C) 2007, Carillon Information Security Inc.
 *
 * This library is licensed under the LGPL, please read LICENSE for details.
 *
 * A WvDBusConn represents a connection to another application. Messages
 * can be sent and received via this connection. In most cases, the
 * other application is a message bus. 
 */ 
#ifndef __WVDBUSCONN_H
#define __WVDBUSCONN_H

#include "wvstreamclone.h"
#include "wvlog.h"
#include "wvdbusmsg.h"

#undef interface
#include <dbus/dbus.h>

#define WVDBUS_DEFAULT_TIMEOUT (300*1000)

class WvDBusConn;

/**
 * The data type of callbacks used by WvDBusConn::add_callback().  The
 * return value should be true if the callback processes the message, false
 * otherwise.
 */
typedef WvCallback<bool, WvDBusConn&, WvDBusMsg&> WvDBusCallback;
    

class IWvDBusAuth
{
public:
    virtual ~IWvDBusAuth() { };
    
    /**
     * Main action callback.  Called whenever d seems to have data available.
     * Return false if you're not yet authorized and need to be called again
     * when data is available; return true if you're done.
     * 
     * If authorization fails, call seterr on d with an appropriate error
     * message.
     */
    virtual bool authorize(WvDBusConn &c) = 0;
};


class WvDBusClientAuth : public IWvDBusAuth
{
    bool sent_request;
public:
    WvDBusClientAuth();
    virtual bool authorize(WvDBusConn &c);
};


class WvDBusConn : public WvStreamClone
{
    bool client, authorized;
    WvString _uniquename;
    IWvDBusAuth *auth;
    
public:
    WvLog log;
    
    WvDBusConn(DBusConnection*, WvStringParm)
	: WvStreamClone(0), log("foo"), pending(10)
    {
	assert(0);
    }
    
    /**
     * Creates a new dbus connection using the given WvStreams moniker.
     * 
     * WvDBus uses special monikers for the "standard" DBus buses:
     * dbus:system, dbus:session, and dbus:starter.  These correspond to 
     * DBUS_BUS_SYSTEM, DBUS_BUS_SESSION, and DBUS_BUS_STARTER, respectively.
     */
    WvDBusConn(WvStringParm moniker, IWvDBusAuth *_auth = NULL,
	       bool _client = true);
    
    /**
     * Creates a new dbus connection from the given stream.  Takes ownership
     * of the stream and will WVRELEASE() it when done.
     */
    WvDBusConn(IWvStream *_cloned, IWvDBusAuth *_auth = NULL,
	       bool _client = true);
    
    void init(IWvDBusAuth *_auth, bool _client);
    
    /**
     * Release this connection.  If this is the last owner of the associated
     * DBusConnection object, the connection itself closes.
     */
    virtual ~WvDBusConn();
    
    void set_uniquename(WvStringParm s);
    void try_auth();
    void send_hello();
    
    void out(WvStringParm s);
    void out(WVSTRING_FORMAT_DECL)
        { return out(WvString(WVSTRING_FORMAT_CALL)); }
    const char *in();

    /**
     * Request the given service name on DBus.  There's no guarantee the
     * server will let us have the requested name, though.
     * 
     * The name will be released when this connection object is destroyed.
     */
    void request_name(WvStringParm name, const WvDBusCallback &onreply = 0,
		      time_t msec_timeout = WVDBUS_DEFAULT_TIMEOUT);
    
    /**
     * Return this connection's unique name on the bus, assigned by the server
     * at connect time.
     */
    WvString uniquename() const;
    
    /**
     * Close the underlying stream.  The object becomes unusable.  This is
     * also called whenever an error is set.
     */
    virtual void close();
    
    /**
     * Send a message on the bus, calling onreply() when the reply comes in
     * or the messages times out.
     */
    void send(WvDBusMsg msg, const WvDBusCallback &onreply,
	      time_t msec_timeout = WVDBUS_DEFAULT_TIMEOUT)
    {
	send(msg);
	if (onreply)
	    add_pending(msg, onreply, msec_timeout);
    }

    /**
     * Send a message on the bus, not expecting any reply.  Returns the
     * assigned serial number in case you want to track it some other way.
     */
    uint32_t send(WvDBusMsg msg);
    
    /**
     * The priority level of a callback registration.  This defines the order
     * in which callbacks are processed, from lowest to highest integer.
     */
    enum CallbackPri { 
	PriSystem    = 0,     // implemented by DBus or WvDBus.  Don't use.
	PriSpecific  = 5000,  // strictly limited by interface/method
	PriNormal    = 6000,  // a reasonably selective callback
	PriBridge    = 7000,  // proxy selectively to other listeners
	PriBroadcast = 8000,  // last-resort proxy to all listeners
    };
    
    /**
     * Adds a callback to the connection: all received messages will be
     * sent to all callbacks to look at and possibly process.  This method
     * is simpler and more flexible than add_listener()/add_method(),
     * but it can be slow if you have too many callbacks set.
     *
     * Your application is very unlikely to have "too many" callbacks.  If
     * for some reason you need to register lots of separate callbacks,
     * make your own data structure for them and register just a single
     * callback here that looks things up in your own structure.
     * 
     * 'pri' defines the callback sort order.  When calling callbacks, we
     * call them in priority order until the first callback returns 'true'.
     * If you just want to log certain messages and let other people handle
     * them, use a high priority but return 'false'.
     * 
     * 'cookie' is used to identify this callback for del_callback().  Your
     * 'this' pointer is a useful value here.
     */
    void add_callback(CallbackPri pri, WvDBusCallback cb, void *cookie = NULL);
    
    /**
     * Delete all callbacks that have the given cookie.
     */
    void del_callback(void *cookie);

    /**
     * Called by for each received message.  Returns true if we handled
     * this message, false if not.  You should not need to call or override
     * this; see add_callback() instead.
     */
    virtual bool filter_func(WvDBusMsg &msg);
    
    time_t mintimeout_msec()
    {
	WvTime when = 0;
	PendingDict::Iter i(pending);
	for (i.rewind(); i.next(); )
	{
	    if (!when || when > i->valid_until)
		when = i->valid_until;
	}
	if (!when)
	    return -1;
	else if (when <= wvstime())
	    return 0;
	else
	    return msecdiff(when, wvstime());
    }
    
    WvDynBuf mybuf;
    virtual bool post_select(SelectInfo &si)
    {
	bool ready = WvStreamClone::post_select(si);
	if (si.inherit_request) return ready;
	
	if (!authorized && ready)
	    try_auth();
	
	if (!alarm_remaining())
	{
	    WvTime now = wvstime();
	    PendingDict::Iter i(pending);
	    for (i.rewind(); i.next(); )
	    {
		if (now > i->valid_until)
		{
		    log("Expiring %s\n", i->msg);
		    expire_pending(i.ptr());
		    i.rewind();
		}
	    }
	}
	
	if (authorized && ready)
	{
	    size_t needed = WvDBusMsg::demarshal_bytes_needed(mybuf);
	    size_t amt = needed - mybuf.used();
	    if (amt < 4096)
		amt = 4096;
	    read(mybuf, amt);
	    WvDBusMsg *m;
	    while ((m = WvDBusMsg::demarshal(mybuf)) != NULL)
	    {
		filter_func(*m);
		delete m;
	    }
	}
	
	alarm(mintimeout_msec());
	return false;
    }

    bool isidle()
    {
	return !out_queue.used() && pending.isempty();
    }
    
private:
    struct Pending
    {
	WvDBusMsg msg; // needed in case we need to generate timeout replies
	uint32_t serial;
	WvDBusCallback cb;
	WvTime valid_until;
	
	Pending(WvDBusMsg &_msg, const WvDBusCallback &_cb,
		time_t msec_timeout)
	    : msg(_msg), cb(_cb)
	{
	    serial = msg.get_serial();
	    if (msec_timeout < 0)
		msec_timeout = 5*3600*1000; // "forever" is a few hours
	    valid_until = msecadd(wvstime(), msec_timeout);
	}
    };
    DeclareWvDict(Pending, uint32_t, serial);
    
    PendingDict pending;
    
    WvDynBuf out_queue;
    
    void expire_pending(Pending *p)
    {
	if (p)
	{
	    WvDBusError e(p->msg, DBUS_ERROR_FAILED,
			  "Timed out while waiting for reply");
	    p->cb(*this, e);
	    pending.remove(p);
	}
    }
    
    void cancel_pending(uint32_t serial)
    {
	Pending *p = pending[serial];
	if (p)
	{
	    WvDBusError e(p->msg, DBUS_ERROR_FAILED,
			  "Canceled while waiting for reply");
	    p->cb(*this, e);
	    pending.remove(p);
	}
    }
    
    void add_pending(WvDBusMsg &msg, WvDBusCallback cb,
		     time_t msec_timeout)
    {
	uint32_t serial = msg.get_serial();
	assert(serial);
	if (pending[serial])
	    cancel_pending(serial);
	pending.add(new Pending(msg, cb, msec_timeout), true);
	alarm(mintimeout_msec());
    }
    
    bool _registered(WvDBusConn &c, WvDBusMsg &msg)
    {
	WvDBusMsg::Iter i(msg);
	_uniquename = i.getnext().get_str();
	set_uniquename(_uniquename);
	return true;
    }

    struct CallbackInfo
    {
	CallbackPri pri;
	WvDBusCallback cb;
	void *cookie;
	
	CallbackInfo(CallbackPri _pri,
		     const WvDBusCallback &_cb, void *_cookie)
	    : cb(_cb)
	    { pri = _pri; cookie = _cookie; }
    };
    static int priority_order(const CallbackInfo *a, const CallbackInfo *b);
	
    DeclareWvList(CallbackInfo);
    CallbackInfoList callbacks;
    
};

#endif // __WVDBUSCONN_H
