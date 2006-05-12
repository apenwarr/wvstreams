#include "wvdbusmarshaller.h"


template<typename P>
P convert_to_type(DBusMessageIter *iter, bool &success)
{
    fprintf(stderr, "ERROR: type does not compute!\n");
    success = false;
    P null;
    return null;
}

template<>
int convert_to_type(DBusMessageIter *iter, bool &success)
{
    fprintf(stderr, "Yay! type computes!\n");
    success = true;
    return 0;
}


bool convert_next(DBusMessageIter *iter, int &i)
{
    dbus_int32_t x;
    dbus_message_iter_get_basic(iter, &x);
    
    i = x;
    return true;
}


bool convert_next(DBusMessageIter *iter, uint32_t &i)
{
    dbus_int32_t x;
    dbus_message_iter_get_basic(iter, &x);
    
    i = x;
    return true;
}


bool convert_next(DBusMessageIter *iter, WvString &s)
{
    char *tmp;
    dbus_message_iter_get_basic(iter, &tmp);
    
    s = tmp;
    return true;
}
