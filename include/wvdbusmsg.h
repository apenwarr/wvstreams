/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 * WvDBusMsg and WvDBusReplyMsg are intended to be easy-to-use abstractions
 * over the low-level D-Bus DBusMessage structure. They represent messages
 * being passed around on the bus.
 */ 
#ifndef __WVDBUSMSG_H
#define __WVDBUSMSG_H
#include "iwvdbuslistener.h"
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
    /**
     * Constructs a new WvDBus message. If destination is blank, no 
     * destination is set; this is appropriate when using D-BUS in a 
     * peer-to-peer context (no message bus).
     *
     */
    WvDBusMsg(WvStringParm busname, WvStringParm objectname, 
              WvStringParm interface, WvStringParm method);

    /**
     * Constructs a new WvDBus message, copying it out of an old one.
     */
    WvDBusMsg(WvDBusMsg &_msg)
    {
        msg = _msg.msg;
        dbus_message_ref(msg);
    }

    /**
     * Constructs a new WvDBus message from an existing low-level D-Bus 
     * message.
     */
    WvDBusMsg(DBusMessage *_msg)
    {
        msg = _msg;
        dbus_message_ref(msg);
    }

    WvDBusMsg() {}

    virtual ~WvDBusMsg()
    {
        dbus_message_unref(msg);
    }

    operator DBusMessage* () const
    {
        return msg;
    }

    /*
     * The follow methods are designed to allow appending various arguments
     * to the message. I will assume that their names are self-explanatory..
     */

    void append(WvStringParm s);
    void append(bool b);
    void append(char c);
    void append(int16_t i);
    void append(uint16_t i);
    void append(int32_t i);
    void append(uint32_t i);
    void append(double d);

protected:
    mutable DBusMessage *msg;
};

class WvDBusReplyMsg : public WvDBusMsg
{
public:
    /**
     * Constructs a new reply message (a message intended to be a reply to
     * an existing D-Bus message).
     */
    WvDBusReplyMsg(DBusMessage *_msg);
    virtual ~WvDBusReplyMsg() {}
};

#endif // __WVDBUSMSG_H
