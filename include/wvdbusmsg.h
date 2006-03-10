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


class WvDBusMsg
{
public:
    WvDBusMsg(DBusMessage *_msg)
    {
        msg = _msg;
        dbus_message_ref(msg);
    }
    WvDBusMsg(WvStringParm busname, WvStringParm objectname, 
              WvStringParm interface, WvStringParm method);

#if 0
    WvDBusMsg(const WvDBusMsg &m)
    {
        dbus_message_ref(m);
    }
#endif
    virtual ~WvDBusMsg()
    {
        if (msg) dbus_message_unref(msg);
    }

    operator DBusMessage* () const
    {
        return msg;
    }

    void append(WvStringParm s);

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
