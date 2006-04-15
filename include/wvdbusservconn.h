/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 */ 
#ifndef __WVDBUSSERVCONN_H
#define __WVDBUSSERVCONN_H
#include "wvdbusconn.h"
#include <stdint.h>

class WvDBusServConn : public WvDBusConn
{
public:
    WvDBusServConn(DBusConnection *c);
    virtual ~WvDBusServConn()
    {}

    virtual bool isok() const
    {
        return true; //conn;
    }

    virtual void add_marshaller(WvStringParm interface, WvStringParm path,
                                IWvDBusMarshaller *marshaller);
//     virtual void add_method(WvStringParm interface, WvStringParm path,
//                     IWvDBusMarshaller *listener);

private:
    void hello_cb(WvDBusReplyMsg &reply);
    void request_name_cb(WvDBusReplyMsg &reply, WvString name,
                         uint32_t flags);
    WvLog log;
};

#endif // __WVDBUSSERVCONN_H
