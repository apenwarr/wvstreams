#include "wvdbusmsg.h"


WvDBusMsg::WvDBusMsg(WvStringParm busname, WvStringParm objectname, 
                     WvStringParm interface, WvStringParm method)
{
    msg = dbus_message_new_method_call(busname, objectname, interface, method);
    if (msg == NULL) 
    { 
        fprintf(stderr, "Message Null\n");
        // FIXME: we've got a problem here.. what to do about it?
    }
}


void WvDBusMsg::append(WvStringParm s1)
{
    const char *tmp;
    if (!s1.isnull())
    {
	tmp = s1.cstr();
	dbus_message_append_args(msg, DBUS_TYPE_STRING, &tmp,
				 DBUS_TYPE_INVALID);
    }
}


// WvDBusReplyMsg::WvDBusReplyMsg(WvDBusMsg *_msg) :
//     WvDBusMsg(dbus_message_new_method_return(_msg->msg))
// {
// }

WvDBusReplyMsg::WvDBusReplyMsg(DBusMessage *_msg) :
    WvDBusMsg(dbus_message_new_method_return(_msg))
{
}
