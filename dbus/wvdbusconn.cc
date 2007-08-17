/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 *
 * Pathfinder Software:
 *   Copyright (C) 2007, Carillon Information Security Inc.
 *
 * This library is licensed under the LGPL, please read LICENSE for details.
 * 
 */ 
#undef interface // windows
#include "wvdbusconn.h"
#include "wvmoniker.h"
#include "wvstrutils.h"
#include <dbus/dbus.h>

static WvString translate(WvStringParm dbus_moniker)
{
    WvStringList l;
    WvStringList::Iter i(l);
    
    if (!strncasecmp(dbus_moniker, "unix:", 5))
    {
	WvString path, tmpdir;
	l.split(dbus_moniker+5, ",");
	for (i.rewind(); i.next(); )
	{
	    if (!strncasecmp(*i, "path=", 5))
		path = *i + 5;
	    else if (!strncasecmp(*i, "abstract=", 9))
		path = WvString("@%s", *i + 9);
	    else if (!strncasecmp(*i, "tmpdir=", 7))
		tmpdir = *i + 7;
	}
	if (!!path)
	    return WvString("unix:%s", path);
	else if (!!tmpdir)
	    return WvString("unix:%s/dbus.sock", tmpdir);
    }
    else if (!strncasecmp(dbus_moniker, "tcp:", 4))
    {
	WvString host, port, family;
	l.split(dbus_moniker+4, ",");
	for (i.rewind(); i.next(); )
	{
	    if (!strncasecmp(*i, "family=", 7))
		family = *i + 7;
	    else if (!strncasecmp(*i, "host=", 5))
		host = *i + 5;
	    else if (!strncasecmp(*i, "port=", 5))
		port = *i + 5;
	}
	if (!!host && !!port)
	    return WvString("tcp:%s:%s", host, port);
	else if (!!host)
	    return WvString("tcp:%s", host);
	else if (!!port)
	    return WvString("tcp:0.0.0.0:%s", port); // localhost
    }
    
    return dbus_moniker; // unrecognized
}


static IWvStream *stream_creator(WvStringParm _s, IObject *)
{
    WvString s(_s);
    
    if (!strcasecmp(s, "starter"))
    {
	WvString startbus(getenv("DBUS_STARTER_ADDRESS"));
	if (!!startbus)
	    return wvcreate<IWvStream>(translate(startbus));
	else
	{
	    WvString starttype(getenv("DBUS_STARTER_BUS_TYPE"));
	    if (!!starttype && !strcasecmp(starttype, "system"))
		s = "system";
	    else if (!!starttype && !strcasecmp(starttype, "session"))
		s = "session";
	}
    }
    
    if (!strcasecmp(s, "system"))
    {
	WvString bus(getenv("DBUS_SYSTEM_BUS_ADDRESS"));
	if (!!bus)
	    return wvcreate<IWvStream>(translate(bus));
    }
    
    if (!strcasecmp(s, "session"))
    {
	WvString bus(getenv("DBUS_SESSION_BUS_ADDRESS"));
	if (!!bus)
	    return wvcreate<IWvStream>(translate(bus));
    }
    
    return wvcreate<IWvStream>(translate(s));
}

static WvMoniker<IWvStream> reg("dbus", stream_creator);


static int conncount;

WvDBusConn::WvDBusConn(IWvStream *_cloned, IWvDBusAuth *_auth, bool _client)
	: WvStreamClone(_cloned),
          log(WvString("DBus %s%s",
		       _client ? "" : "s",
		       ++conncount), WvLog::Debug5),
          pending(10)
{
    init(_auth, _client);
}

    
WvDBusConn::WvDBusConn(WvStringParm moniker, IWvDBusAuth *_auth, bool _client)
	: WvStreamClone(wvcreate<IWvStream>(moniker)),
          log(WvString("DBus %s%s",
		       _client ? "" : "s",
		       ++conncount), WvLog::Debug5),
          pending(10)
{
    init(_auth, _client);
}


void WvDBusConn::init(IWvDBusAuth *_auth, bool _client)
{
    log("Initializing.\n");
    client = _client;
    auth = _auth ? _auth : new WvDBusClientAuth;
    authorized = false;
    if (!client) set_uniquename(WvString(":%s", conncount));
    
    if (!isok()) return;
    
    // this will get enqueued until later, but we want to make sure it
    // comes before anything the user tries to send - including anything
    // goofy they enqueue in the authorization part.
    if (client)
	send_hello();
    
    try_auth();
}


WvDBusConn::~WvDBusConn()
{
    log("Shutting down.\n");
    if (geterr())
	log("Error was: %s\n", errstr());
    
    close();
}


void WvDBusConn::close()
{
    log("Closing.\n");
    WvStreamClone::close();
}


WvString WvDBusConn::uniquename() const
{
    return _uniquename;
}


void WvDBusConn::request_name(WvStringParm name, const WvDBusCallback &onreply,
			      time_t msec_timeout)
{
    uint32_t flags = (DBUS_NAME_FLAG_ALLOW_REPLACEMENT |
		      DBUS_NAME_FLAG_REPLACE_EXISTING);
    WvDBusMsg msg("org.freedesktop.DBus", "/org/freedesktop/DBus",
		  "org.freedesktop.DBus", "RequestName");
    msg.append(name).append(flags);
    send(msg, onreply, msec_timeout);
}


uint32_t WvDBusConn::send(WvDBusMsg msg)
{
    msg.marshal(out_queue);
    if (authorized)
    {
	log(" >> %s\n", msg);
	write(out_queue);
    }
    else
	log(" .. %s\n", msg);
    return msg.get_serial();
}


void WvDBusConn::out(WvStringParm s)
{
    log(" >> %s", s);
    print(s);
}


const char *WvDBusConn::in()
{
    const char *s = trim_string(getline(0));
    if (s)
	log("<<  %s\n", s);
    return s;
}


void WvDBusConn::send_hello()
{
    WvDBusMsg msg("org.freedesktop.DBus", "/org/freedesktop/DBus",
		  "org.freedesktop.DBus", "Hello");
    send(msg, WvDBusCallback(this, &WvDBusConn::_registered));
    WvDBusMsg msg2("org.freedesktop.DBus", "/org/freedesktop/DBus",
		   "org.freedesktop.DBus", "AddMatch");
    msg2.append("type='signal'");
    send(msg2); // don't need to monitor this for completion
}


void WvDBusConn::set_uniquename(WvStringParm s)
{
    // we want to print the message before switching log.app, so that we
    // can trace which log.app turned into which
    log("Assigned name '%s'\n", s);
    _uniquename = s;
    log.app = WvString("DBus %s%s", client ? "" : "s", uniquename());
}



void WvDBusConn::try_auth()
{
    bool done = auth->authorize(*this);
    if (done)
    {
	delete auth;
	auth = NULL;
	
	// ready to send messages!
	write(out_queue);
	
	authorized = true;
    }
}


void WvDBusConn::add_callback(CallbackPri pri, WvDBusCallback cb, void *cookie)
{
    callbacks.append(new CallbackInfo(pri, cb, cookie), true);
}


void WvDBusConn::del_callback(void *cookie)
{
    // remember, there might be more than one callback with the same cookie.
    CallbackInfoList::Iter i(callbacks);
    for (i.rewind(); i.next(); )
	if (i->cookie == cookie)
	    i.xunlink();
}


int WvDBusConn::priority_order(const CallbackInfo *a, const CallbackInfo *b)
{
    return a->pri - b->pri;
}


bool WvDBusConn::filter_func(WvDBusMsg &msg)
{
    log("<<  %s\n", msg);
    
    // handle replies
    uint32_t rserial = msg.get_replyserial();
    if (rserial)
    {
	Pending *p = pending[rserial];
	if (p)
	{
	    p->cb(*this, msg);
	    pending.remove(p);
	    return true; // handled it
	}
    }
    
    // handle all the generic filters
    CallbackInfoList::Sorter i(callbacks, priority_order);
    for (i.rewind(); i.next(); )
    {
	bool handled = i->cb(*this, msg);
	if (handled) return true;
    }

    return false; // couldn't handle the message, sorry
}


WvDBusClientAuth::WvDBusClientAuth()
{
    sent_request = false;
}


bool WvDBusClientAuth::authorize(WvDBusConn &c)
{
    if (!sent_request)
    {
	c.write("\0", 1);
#ifndef _WIN32
	long uid = getuid();
#else
        long uid = 0;
#endif
	c.out("AUTH EXTERNAL %s\r\n\0", WvHexEncoder().strflushstr(uid));
	sent_request = true;
    }
    else
    {
	const char *line = c.in();
	if (line)
	{
	    if (!strncasecmp(line, "OK ", 3))
	    {
		c.out("BEGIN\r\n");
		return true;
	    }
	    else if (!strncasecmp(line, "ERROR ", 6))
		c.seterr("Auth failed: %s", line);
	    else
		c.seterr("Unknown AUTH response: '%s'", line);
	}
    }
    
    return false;
}
