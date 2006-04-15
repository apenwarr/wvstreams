/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 */ 
#ifndef __WVDBUSMSG_H
#define __WVDBUSMSG_H
#include "iwvdbusmarshaller.h"
#include "wvfdstream.h"
#include "wvhashtable.h"
#include "wvistreamlist.h"
#include "wvlog.h"
#include "wvstringlist.h"
#include <dbus/dbus.h>
#include <stdint.h>


class WvDBusMsg
{
public:
    /*
     * Constructs a new DBus message. If destination is blank, no destination 
     * is set; this is appropriate when using D-BUS in a peer-to-peer context 
     * (no message bus).
     *
     * @param destination name that the message should be sent to or #NULL
     * @param path object path the message should be sent to
     * @param interface interface to invoke method on
     * @param method method to invoke
     *
     */
    WvDBusMsg(WvStringParm busname, WvStringParm objectname, 
              WvStringParm interface, WvStringParm method);

    WvDBusMsg(DBusMessage *_msg)
    {
        msg = _msg;
        dbus_message_ref(msg);
    }

    virtual ~WvDBusMsg()
    {
        if (msg) dbus_message_unref(msg);
    }

    operator DBusMessage* () const
    {
        return msg;
    }

    void append(WvStringParm s);
    void append(uint32_t i);

private:
    mutable WvStringList args;
    mutable DBusMessage *msg;
};

class WvDBusReplyMsg : public WvDBusMsg
{
public:
//     WvDBusReplyMsg(WvDBusMsg *_msg);
    WvDBusReplyMsg(DBusMessage *_msg);

};

#endif // __WVDBUSMSG_H
