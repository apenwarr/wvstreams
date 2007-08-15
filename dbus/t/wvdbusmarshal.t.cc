#include "wvtest.h"
#include "wvbuf.h"
#include "wvdbusmsg.h"
#include "wvstream.h"
#include "wvstrutils.h"

#define DBUS_COMPILATION
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
extern "C" {
#include <dbus/dbus-marshal-header.h>
}
#include <dbus/dbus-internals.h>
#include <dbus/dbus-string.h>
#include <dbus/dbus-message-private.h>

static bool _marshal(DBusMessage *msg, char **cbuf, size_t *len)
{
    static uint32_t global_serial = 1000;
    DBusString tmp;
    
    if (!_dbus_string_init (&tmp))
	return false;
    
    if (!dbus_message_get_serial(msg))
	_dbus_message_set_serial(msg, ++global_serial);
    
    _dbus_message_lock(msg);
    _dbus_string_copy(&msg->header.data, 0, &tmp, 0);
    *len = _dbus_string_get_length(&tmp);
    _dbus_string_copy(&msg->body, 0, &tmp, *len);
    *len = _dbus_string_get_length(&tmp);
    
    _dbus_string_steal_data(&tmp, cbuf);
    _dbus_string_free(&tmp);
    return true;
}


static bool marshal(const WvDBusMsg &msg, WvBuf &buf)
{
    size_t len;
    char *cbuf;
    
    if (_marshal(msg, &cbuf, &len))
    {
	buf.put(cbuf, len);
	free(cbuf);
	return true;
    }
    return false;
}


static size_t message_length(const void *buf, size_t len)
{
    if (!buf || len < DBUS_MINIMUM_HEADER_SIZE)
	return DBUS_MINIMUM_HEADER_SIZE;
    
    // doesn't copy - no need to free
    DBusString buftmp;
    _dbus_string_init_const_len(&buftmp, (const char *)buf, len);
    
    int byte_order, fields_array_len, header_len, body_len;
    DBusValidity validity = DBUS_VALID;
    bool have_message
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


static DBusMessage *_demarshal(const void *buf, size_t len, size_t *used)
{
    DBusMessageLoader *loader;
    DBusString *lbuf;
    DBusMessage *msg;
    
    if (!buf)
    {
	*used = 0;
	return NULL;
    }
    
    size_t real_len = message_length(buf, len);
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
	goto fail_oom;
    
    if (_dbus_message_loader_get_is_corrupted(loader))
	goto fail_corrupt;
    
    msg = _dbus_message_loader_pop_message(loader);
    if (!msg)
	goto fail_oom;
    
    _dbus_message_loader_unref(loader);
    *used = real_len;
    return msg;
    
fail_corrupt:
    fprintf(stderr, "\n\nCORRUPT!\n\n");
    _dbus_message_loader_unref(loader);
    *used = len;
    return NULL;
    
fail_oom:
    fprintf(stderr, "\n\nOOM!\n\n");
    _dbus_message_loader_unref(loader);
    *used = 0;
    return NULL;
}


static WvDBusMsg *demarshal(WvBuf &buf)
{
    size_t used = 0;
    DBusMessage *_msg = _demarshal(buf.peek(0, buf.used()), buf.used(),
				   &used);
    buf.get(used);
    if (_msg)
    {
	WvDBusMsg *msg = new WvDBusMsg(_msg);
	dbus_message_unref(_msg);
	return msg;
    }
    else
	return NULL;
}


WVTEST_MAIN("dbusmarshal")
{
    WvDBusMsg msg("a.b.c", "/d/e/f", "g.h.i", "j"), *decoded = NULL;
    msg.append("string1");
    msg.append(2);
    msg.array_start("v")
	.varray_start("i").append(10).append(11).varray_end()
	.varray_start("s").append("wX").append("Yz").varray_end()
	.array_end()
	.append(42);
    WvDBusMsg msg2("a.a", "/", "c.c", "d");
    
    WvDynBuf buf;
    decoded = demarshal(buf);
    WVPASS(!decoded);
    
    marshal(msg, buf);
    WVPASS(buf.used() > 0);
    buf.putstr("BOOGA");
    marshal(msg2, buf);
    size_t used = buf.used();
    WVPASS(buf.used() > 0);
    WVPASSEQ(msg.get_argstr(), "string1,2,[[10,11],[wX,Yz]],42");
    WVPASSEQ(msg2.get_argstr(), "");
    wvout->print(hexdump_buffer(buf.peek(0, used), used));
    
    WVPASS(buf.used() > 0);
    decoded = demarshal(buf);
    WVPASS(buf.used() > 5); // still data left
    WVPASS(decoded);
    if (decoded)
	WVPASSEQ(decoded->get_argstr(), "string1,2,[[10,11],[wX,Yz]],42");
    
    if (buf.used() > 5)
	WVPASSEQ(buf.getstr(5), "BOOGA");
    if (decoded)
	delete decoded;
    decoded = demarshal(buf);
    WVPASS(buf.used() == 0);
    WVPASS(decoded);
    if (decoded)
    {
	WVPASSEQ(decoded->get_argstr(), "");
	delete decoded;
    }
}
