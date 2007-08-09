/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 *
 * Pathfinder Software:
 *   Copyright (C) 2007, Carillon Information Security Inc.
 *
 * This library is licensed under the LGPL, please read LICENSE for details.
 * 
 * This class represents a dbus server, which may have multiple connections
 * at the same time. It is intended purely for unit testing and debugging
 * purposes and by no means should be used in production code (use the
 * dbus daemon for that).
 * 
 */ 
#ifndef __WVDBUSSERVER_H
#define __WVDBUSSERVER_H

#include "iwvdbuslistener.h"
#include "wvdbusconn.h"
#include "wvhashtable.h"
#include "wvlog.h"
#include "wvistreamlist.h"

class WvDBusServer;
class WvDBusServerPrivate;


// association between reply serials and their respective connection
typedef struct WvDBusReplySerial
{
    WvDBusReplySerial(int _serial, WvDBusConn *_conn) 
    {
        serial = _serial;
        conn = _conn;
    }
        
    int serial;
    WvDBusConn *conn;
} WvDBusReplySerial;


DeclareWvDict(WvDBusReplySerial, int, serial);
DeclareWvDict(WvDBusConn, WvString, name);


class WvDBusServConn : public WvDBusConn
{
public:
    WvDBusServConn(DBusConnection *c, WvDBusServer *s);
    virtual ~WvDBusServConn()
    {}

private:
    void proxy_msg(WvDBusMsg &msg);
    void hello_cb(WvDBusConn &conn, WvDBusMsg &msg, WvError err);
    void request_name_cb(WvDBusConn &conn, WvDBusMsg &msg, WvString name,
                         uint32_t flags, WvError err);
    void release_name_cb(WvDBusConn &conn, WvDBusMsg &msg, WvString _name,
			 WvError err);
    virtual bool filter_func(WvDBusConn &conn, WvDBusMsg &msg);
    WvDBusServer *server;
};


class WvDBusServer : public WvIStreamList
{
    friend class WvDBusServConn;
public:
    /* 
     * Constructs a new DBus server at the specified address. 
     *
     * For example:
     *    WvDBusServer s("unix:path=/tmp/foo");
     *
     */
    WvDBusServer(WvStringParm addr);
    WvDBusServer(WvDBusServer &c);
    virtual ~WvDBusServer();

    virtual bool isok() const
    {
        return true; 
    }

    virtual void execute();

    void register_conn(WvDBusConn *conn);
    // need an unregister_conn
    void proxy_msg(WvStringParm dest, WvDBusConn *src, WvDBusMsg &msg);
    void proxy_msg(uint32_t serial, WvDBusMsg &msg);

    // get the full, final address (identification guid and all) of the server
    WvString get_addr();

private:
    WvDBusServerPrivate *priv;    
    WvDBusConnDict cdict;
    WvDBusReplySerialDict rsdict;
    WvLog log;
};

#endif // __WVDBUSSERVER_H
