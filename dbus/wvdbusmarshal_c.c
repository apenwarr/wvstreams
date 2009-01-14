/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 2004-2006 Net Integration Technologies, Inc.
 *
 * Ugly routines that call directly into DBus internals for marshalling and
 * demarshalling DBusMessage objects into data buffers.
 * 
 * We separate this into a separate file mostly so we don't mix DBus
 * internal header files with other WvStreams header files.
 */ 
#include <inttypes.h>
#define DBUS_COMPILATION
//#undef PACKAGE_BUGREPORT
//#undef PACKAGE_NAME
//#undef PACKAGE_STRING
//#undef PACKAGE_TARNAME
//#undef PACKAGE_VERSION
#undef interface
#include <dbus/dbus.h>
#include <dbus-upstream/dbus/dbus-marshal-header.h>
#include <dbus-upstream/dbus/dbus-internals.h>
#include <dbus-upstream/dbus/dbus-string.h>
#include <dbus-upstream/dbus/dbus-message-private.h>

int wvdbus_marshal(DBusMessage *msg, char **cbuf, size_t *len)
{
    static uint32_t global_serial = 1000;
    DBusString tmp;
    
    if (!_dbus_string_init (&tmp))
	return 0;
    
    if (!dbus_message_get_serial(msg))
	_dbus_message_set_serial(msg, ++global_serial);
    
    _dbus_message_lock(msg);
    _dbus_string_copy(&msg->header.data, 0, &tmp, 0);
    *len = _dbus_string_get_length(&tmp);
    _dbus_string_copy(&msg->body, 0, &tmp, *len);
    *len = _dbus_string_get_length(&tmp);
    
    _dbus_string_steal_data(&tmp, cbuf);
    _dbus_string_free(&tmp);
    return 1;
}

#if 0

int wvdbus_message_length(const void *buf, size_t len)
{
    if (!buf || len < DBUS_MINIMUM_HEADER_SIZE)
	return DBUS_MINIMUM_HEADER_SIZE;
    
    // doesn't copy - no need to free

    DBusString buftmp;
    _dbus_string_init_const_len(&buftmp, (const char *)buf, len);
    
    int byte_order, fields_array_len, header_len, body_len;
    DBusValidity validity = DBUS_VALID;
    int have_message
	= _dbus_header_have_message_untrusted(DBUS_MAXIMUM_MESSAGE_LENGTH,
					      &validity, &byte_order,
					      &fields_array_len,
					      &header_len,
					      &body_len,
					      &buftmp, 0,
					      len);
    if (have_message || validity == DBUS_VALID)
	return header_len + body_len;
    else
	return 0; // broken!
}
#endif
#if 0
DBusMessage *wvdbus_demarshal(const void *buf, size_t len, size_t *used)
{
    DBusMessageLoader *loader;
    DBusString *lbuf;
    DBusMessage *msg;
    
    if (!buf)
    {
	*used = 0;
	return NULL;
    }
    
    size_t real_len = wvdbus_message_length(buf, len);
    if (real_len == 0) // invalid message data
    {
	*used = len; // clear invalid crap - the best we can do
	return NULL;
    }
    else if (real_len > len) // not enough data
    {
	*used = 0;
	return NULL;
    }
    
    loader = _dbus_message_loader_new();
    if (!loader)
	return NULL;
    
    _dbus_message_loader_get_buffer(loader, &lbuf);
    _dbus_string_append_len(lbuf, (const char *)buf, real_len);
    _dbus_message_loader_return_buffer(loader, lbuf, real_len);
    
    if (!_dbus_message_loader_queue_messages(loader))
	goto fail;
    
    if (_dbus_message_loader_get_is_corrupted(loader))
	goto fail;
    
    msg = _dbus_message_loader_pop_message(loader);
    if (!msg)
	goto fail;
    
    _dbus_message_loader_unref(loader);
    *used = real_len;
    return msg;
    
fail:
    _dbus_message_loader_unref(loader);
    *used = real_len ? real_len : len;
    return NULL;
}
#endif
