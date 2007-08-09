/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 * Pathfinder Software:
 *   Copyright (C) 2007, Carillon Information Security Inc.
 *
 * This library is licensed under the LGPL, please read LICENSE for details.
 *
 * This class represents a connection to this application, which exports
 * a message bus interface. This class is mostly useful for debugging and
 * unit test purposes and should not be needed under normal circumstances.
 */ 
#ifndef __WVDBUSSERVCONN_H
#define __WVDBUSSERVCONN_H
#include "wvdbusconnp.h"
#include "wvdbusserver.h"
#include <stdint.h>


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
    void release_name_cb(WvDBusConn &conn, WvDBusMsg &msg, WvString _name, WvError err);
    virtual DBusHandlerResult filter_func(DBusConnection *_conn,
 DBusMessage *_msg);
    WvDBusServer *server;
};

#endif // __WVDBUSSERVCONN_H
