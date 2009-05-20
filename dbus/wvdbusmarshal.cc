/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 * 
 * Wrapper code for marshalling/demarshalling WvDBusMsg objects.  This is
 * in a separate file from WvDBusMsg in case you want to use a WvDBusMsg
 * but not our special/gross marshalling code from wvdbusmarshal_c.c.
 * 
 */ 
#include "wvdbusmsg.h"
#undef interface // windows
#include <dbus/dbus.h>


static int wvdbus_message_length(const char *buf, size_t len)
{
    int msglen = dbus_message_demarshal_bytes_needed(buf, len);
    if (msglen > 0)
        return msglen;
    else if (msglen == 0)
        return DBUS_MINIMUM_HEADER_SIZE;

    return 0;
}


WvDBusMsg *WvDBusMsg::demarshal(WvBuf &buf)
{
    // to make sure bytes are aligned (as required by d-bus), copy them into a 
    // new buffer (not very efficient, but what can you do without reworking
    // our buffer implementation)
    WvDynBuf alignedbuf;
    size_t buflen = buf.used();
    alignedbuf.put(buf.peek(0, buflen), buflen);

    // first get size of message to demarshal. if too little or bad length,
    // return NULL (possibly after consuming the bad data)
    size_t messagelen = wvdbus_message_length((const char *)
                                              alignedbuf.peek(0, buflen), 
                                              buflen);
    if (messagelen == 0) // invalid message data
    {
	buf.get(buflen); // clear invalid crap - the best we can do
	return NULL;
    }
    else if (messagelen > buflen) // not enough data
	return NULL;

    // Assuming that worked and we can demarshal a message, try to do so
    DBusError error;
    dbus_error_init(&error);
    DBusMessage *_msg = dbus_message_demarshal((const char *)
                                               alignedbuf.peek(0, buflen), 
                                               messagelen, &error);
    if (dbus_error_is_set(&error))
        dbus_error_free (&error);
    buf.get(messagelen);

    if (_msg)
    {
	WvDBusMsg *msg = new WvDBusMsg(_msg);
	dbus_message_unref(_msg);
	return msg;
    }
    else
	return NULL;
}


size_t WvDBusMsg::demarshal_bytes_needed(WvBuf &buf)
{
    // to make sure bytes are aligned (as required by d-bus), copy them into a 
    // new buffer (not very efficient, but what can you do without reworking
    // our buffer implementation)
    WvDynBuf alignedbuf;
    size_t used = buf.used();
    alignedbuf.put(buf.peek(0, used), used);

    return wvdbus_message_length((const char *)alignedbuf.peek(0, used), used);
}


void WvDBusMsg::marshal(WvBuf &buf)
{
    DBusMessage *msg = *this;

    static uint32_t global_serial = 1000;   
    if (!dbus_message_get_serial(msg))
    {
        dbus_message_set_serial(msg, ++global_serial);
    }

    dbus_message_lock (msg); 
    char *cbuf;
    int len;
    dbus_message_marshal(msg, &cbuf, &len);
    buf.put(cbuf, len);
    free(cbuf);
}
