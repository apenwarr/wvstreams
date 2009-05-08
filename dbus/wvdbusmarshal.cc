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

WvDBusMsg *WvDBusMsg::demarshal(WvBuf &buf)
{
    // to make sure bytes are aligned (as required by d-bus), copy them into a 
    // new buffer (not very efficient, but what can you do without reworking
    // our buffer implementation)
    WvDynBuf alignedbuf;
    size_t used = buf.used();
    alignedbuf.put(buf.peek(0, used), used);

    size_t bytes_gotten = 0;
    DBusMessage *_msg = wvdbus_demarshal(alignedbuf.peek(0, used), used,
					 &bytes_gotten);
    buf.get(bytes_gotten);
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

    return wvdbus_message_length(alignedbuf.peek(0, used), used);
}


void WvDBusMsg::marshal(WvBuf &buf)
{
    int len;
    char *cbuf;
    
    if (wvdbus_marshal(*this, &cbuf, &len))
    {
	buf.put(cbuf, len);
	free(cbuf);
    }
}
