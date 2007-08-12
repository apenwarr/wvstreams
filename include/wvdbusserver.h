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

struct DBusServer;

class WvDBusServer;


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


class WvDBusServer : public WvIStreamList
{
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
        return !geterr(); 
    }

    void register_conn(WvDBusConn *conn);
    // need an unregister_conn
    
    void proxy_msg(WvStringParm dest, WvDBusConn *src, WvDBusMsg &msg);
    void proxy_msg(uint32_t serial, WvDBusMsg &msg);

    /**
     * get the full, final address (identification guid and all) of the server
     */
    WvString get_addr();

    /**
     * Set the error code of this connection if 'e' is nonempty.
     */
    void maybe_seterr(DBusError &e);

private:
    WvDBusConnDict cdict;
    WvDBusReplySerialDict rsdict;
    WvLog log;
    DBusServer *dbusserver;
    
    static void WvDBusServer::new_connection_cb(DBusServer *dbusserver, 
					DBusConnection *new_connection,
					void *userdata);
	
    bool do_server_msg(WvDBusConn &conn, WvDBusMsg &msg);
    bool do_bridge_msg(WvDBusConn &conn, WvDBusMsg &msg);
    bool do_broadcast_msg(WvDBusConn &conn, WvDBusMsg &msg);
};

#endif // __WVDBUSSERVER_H
