/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 * This class represents a connection to this application, which exports
 * a message bus interface. This class is mostly useful for debugging and
 * unit test purposes and should not be needed under normal circumstances.
 */ 
#ifndef __WVDBUSSERVCONN_H
#define __WVDBUSSERVCONN_H
#include "wvdbusconn.h"
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
    void hello_cb(WvDBusReplyMsg &reply, WvError err);
    void request_name_cb(WvDBusReplyMsg &reply, WvString name,
                         uint32_t flags, WvError err);
    void release_name_cb(WvDBusReplyMsg &reply, WvString _name, WvError err);
    WvDBusServer *server;
    WvLog log;
};

#endif // __WVDBUSSERVCONN_H
