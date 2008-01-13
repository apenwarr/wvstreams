/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2005-2006 Net Integration Technologies, Inc.
 * 
 * Pathfinder Software:
 *   Copyright (C) 2007, Carillon Information Security Inc.
 *
 * This library is licensed under the LGPL, please read LICENSE for details.
 *
 */ 
#include "wvdbusserver.h"
#include "wvdbusconn.h"
#include "wvtcp.h"
#include "wvstrutils.h"
#undef interface // windows
#include <dbus/dbus.h>

class WvDBusServerAuth : public IWvDBusAuth
{
    enum State { NullWait, AuthWait, BeginWait };
    State state;
    long client_uid;
public:
    WvDBusServerAuth();
    virtual bool authorize(WvDBusConn &c);

    virtual long get_unix_uid() { return client_uid; }
};


WvDBusServerAuth::WvDBusServerAuth()
{
    state = NullWait;
    client_uid = -1;
}


bool WvDBusServerAuth::authorize(WvDBusConn &c)
{
    c.log("State=%s\n", state);
    if (state == NullWait)
    {
	char buf[1];
	size_t len = c.read(buf, 1);
	if (len == 1 && buf[0] == '\0')
	{
	    state = AuthWait;
	    // fall through
	}
	else if (len > 0)
	    c.seterr("Client didn't start with NUL byte");
	else
	    return false; // no data yet, come back later
    }
    
    const char *line = c.in();
    if (!line)
	return false; // not done yet
    
    if (state == AuthWait)
    {
	if (!strncasecmp(line, "AUTH ", 5))
	{
	    // FIXME actually check authentication information!
            if (!strncasecmp(line + 5, "EXTERNAL ", 9))
            {
                WvString uid = 
                    WvHexDecoder().strflushstr(line + 5 + 9);
                if (!!uid)
                {
                    // FIXME: Check that client is on the same machine!
                    client_uid = uid.num();
                }
            }
	    state = BeginWait;
	    c.out("OK f00f\r\n");
	}
	else
	    c.seterr("AUTH command expected: %s", line);
    }
    else if (state == BeginWait)
    {
	if (!strcasecmp(line, "BEGIN"))
	    return true; // done
	else
	    c.seterr("BEGIN command expected: %s", line);
    }

    return false;
}


WvDBusServer::WvDBusServer(WvStringParm addr)
    : log("DBus Server", WvLog::Debug)
{
    listener = new WvTCPListener(addr);
    append(listener, false);
    log(WvLog::Info, "Listening on '%s'\n", *listener->src());
    listener->onaccept(wv::bind(&WvDBusServer::new_connection_cb, this,
				_1));
}


WvDBusServer::~WvDBusServer()
{
    close();
    zap();
    WVRELEASE(listener);
}


WvString WvDBusServer::get_addr()
{
    return WvString("tcp:%s", *listener->src());
}


void WvDBusServer::register_name(WvStringParm name, WvDBusConn *conn)
{
    name_to_conn[name] = conn;
}


void WvDBusServer::unregister_name(WvStringParm name, WvDBusConn *conn)
{
    assert(name_to_conn[name] == conn);
    name_to_conn.erase(name);
}


void WvDBusServer::unregister_conn(WvDBusConn *conn)
{
    {
	std::map<WvString,WvDBusConn*>::iterator i;
	for (i = name_to_conn.begin(); i != name_to_conn.end(); )
	{
	    if (i->second == conn)
	    {
		name_to_conn.erase(i->first);
		i = name_to_conn.begin();
	    }
	    else
		++i;
	}
    }
    
    {
	std::map<uint32_t,WvDBusConn*>::iterator i;
	for (i = serial_to_conn.begin(); i != serial_to_conn.end(); )
	{
	    if (i->second == conn)
	    {
		serial_to_conn.erase(i->first);
		i = serial_to_conn.begin();
	    }
	    else
		++i;
	}
    }
    
    all_conns.unlink(conn);
}


bool WvDBusServer::do_server_msg(WvDBusConn &conn, WvDBusMsg &msg)
{
    WvString method(msg.get_member());
    
    if (msg.get_path() == "/org/freedesktop/DBus/Local")
    {
	if (method == "Disconnected")
	    return true; // nothing to do until their *stream* disconnects
    }
    
    if (msg.get_dest() != "org.freedesktop.DBus") return false;
    if (msg.get_path() != "/org/freedesktop/DBus") return false;
    
    // I guess it's for us!
    
    if (method == "Hello")
    {
	log("hello_cb\n");
	msg.reply().append(conn.uniquename()).send(conn);
	return true;
    }
    else if (method == "RequestName")
    {
	WvDBusMsg::Iter args(msg);
	WvString _name = args.getnext();
	// uint32_t flags = args.getnext(); // supplied, but ignored
	
	log("request_name_cb(%s)\n", _name);
	register_name(_name, &conn);
	
	msg.reply().append((uint32_t)DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
	    .send(conn);
	return true;
    }
    else if (method == "ReleaseName")
    {
	WvDBusMsg::Iter args(msg);
	WvString _name = args.getnext();
	
	log("release_name_cb(%s)\n", _name);
	unregister_name(_name, &conn);
	
	msg.reply().append((uint32_t)DBUS_RELEASE_NAME_REPLY_RELEASED)
	    .send(conn);
	return true;
    }
    else if (method == "AddMatch")
    {
	// we just proxy everything to everyone for now
	msg.reply().send(conn);
	return true;
    }
    else if (method == "GetConnectionUnixUser")
    {
	WvDBusMsg::Iter args(msg);
	WvString _name = args.getnext();
        WvDBusConn *target = name_to_conn[_name];
        long client_uid = -1;
        if (target != NULL)
            client_uid = target->get_unix_uid();

        if (client_uid != -1)
        {
            log("Found unix user for '%s', uid is %s.\n", _name, client_uid);
            msg.reply().append((uint32_t)client_uid).send(conn);
        }
        else 
        {
            log("Could not find unix user for '%s'.\n", _name);
            WvDBusError(msg, "org.freedesktop.DBus.Error.Failed", 
                "Could not find the requested connection").send(conn);
        }
            
        return true;
    }
    
    return false; // didn't recognize the method
}


bool WvDBusServer::do_bridge_msg(WvDBusConn &conn, WvDBusMsg &msg)
{
    // if we get here, nobody handled the message internally, so we can try
    // to proxy it.
    if (msg.is_reply())
    {
	uint32_t rserial = msg.get_replyserial();
	std::map<uint32_t,WvDBusConn *>::iterator i 
	    = serial_to_conn.find(rserial);
	if (i != serial_to_conn.end())
	{
	    WvDBusConn *dconn = i->second;
	    log("Proxy reply: target is %s\n", dconn->uniquename());
	    dbus_message_set_sender(msg, conn.uniquename().cstr());
	    dconn->send(msg);
	    serial_to_conn.erase(rserial);
	    return true;
	}
	else
	{
	    log("Proxy reply: unknown serial #%s!\n", rserial);
	    // fall through and let someone else look at it
	}
    }
    else if (!!msg.get_dest()) // don't handle blank (broadcast) paths here
    {
	std::map<WvString,WvDBusConn*>::iterator i 
	    = name_to_conn.find(msg.get_dest());
	WvDBusConn *dconn = (i == name_to_conn.end()) ? NULL : i->second;
	log("Proxying #%s -> %s\n",
	    msg.get_serial(),
	    dconn ? dconn->uniquename() : WvString("(UNKNOWN)"));
	dbus_message_set_sender(msg, conn.uniquename().cstr());
	if (dconn)
	{
	    uint32_t serial = dconn->send(msg);
	    serial_to_conn[serial] = &conn;
	    log("Proxy: now expecting reply #%s to %s\n",
		serial, conn.uniquename());
	}
	else
	    log(WvLog::Warning,
		"Proxy: no connection for '%s'\n", msg.get_dest());
        return true;
    }
    return false;
}


bool WvDBusServer::do_broadcast_msg(WvDBusConn &conn, WvDBusMsg &msg)
{
    if (!msg.get_dest())
    {
	log("Broadcasting #%s\n", msg.get_serial());
	
	// note: we broadcast messages even back to the connection where
	// they originated.  I'm not sure this is necessarily ideal, but if
	// you don't do that then an app can't signal objects that might be
	// inside itself.
	WvDBusConnList::Iter i(all_conns);
	for (i.rewind(); i.next(); )
	    i->send(msg);
        return true;
    }
    return false;
}


void WvDBusServer::conn_closed(WvStream &s)
{
    WvDBusConn *c = (WvDBusConn *)&s;
    unregister_conn(c);
}


void WvDBusServer::new_connection_cb(IWvStream *s)
{
    WvDBusConn *c = new WvDBusConn(s, new WvDBusServerAuth, false);
    all_conns.append(c, false);
    register_name(c->uniquename(), c);
    c->setclosecallback(wv::bind(&WvDBusServer::conn_closed, this,
				 wv::ref(*c)));
    
    c->add_callback(WvDBusConn::PriSystem,
		    wv::bind(&WvDBusServer::do_server_msg, this,
			     wv::ref(*c), _1));
    c->add_callback(WvDBusConn::PriBridge, 
		    wv::bind(&WvDBusServer::do_bridge_msg, this,
			     wv::ref(*c), _1));
    c->add_callback(WvDBusConn::PriBroadcast,
		    wv::bind(&WvDBusServer::do_broadcast_msg, this,
			     wv::ref(*c), _1));
    
    append(c, true, "wvdbus servconn");
}
