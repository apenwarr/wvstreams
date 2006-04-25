/* -*- mode: c++ -*-
 */

#ifndef __WVDBUS_H
#define __WVDBUS_H

#include "wvfdstream.h"
#include "wvlog.h"
#include "wvstringlist.h"
#include <dbus/dbus.h>

class WvDBus : public WvFdStream
{
    static DBusHandlerResult filter_func(DBusConnection *conn,
					 DBusMessage *msg,
					 void *userdata);
    static dbus_bool_t add_watch(DBusWatch *watch, void *data);
    static void remove_watch(DBusWatch *watch, void *data);

public:
    class Err : public DBusError
    {
	WvLog log;
	WvString why;

    public:
	Err();
	~Err();

	Err *reset(WvStringParm _why = "??");
	Err *operator()(WvStringParm _why = "??")
	{
	    return reset(_why);
	}

	void print();
    };

    class Msg
    {
    public:
	mutable DBusMessage *msg;

	Msg(DBusMessage *_msg)
	{
	    msg = _msg;
	}
	Msg(const Msg &m)
	{
	    dbus_message_ref(m);
	}
	~Msg()
	{
	    if (msg) dbus_message_unref(msg);
	}

	operator DBusMessage* () const
	{
	    return msg;
	}

	void append(WvStringParm s1, WvStringParm s2 = WvString::null,
		    WvStringParm s3 = WvString::null);

	void decode(WvStringList &l) const;

	WvString arg(int n) const;

    private:
	mutable WvStringList args;

    };

    class CallMsg : public Msg
    {
    public:
	CallMsg(WvStringParm svc, WvStringParm obj, WvStringParm ifc,
		WvStringParm method)
	    : Msg(dbus_message_new_method_call(svc, obj, ifc, method))
	{ }
    };

    class ReplyMsg : public Msg
    {
    public:
	ReplyMsg(const Msg &m)
	    : Msg(dbus_message_new_method_return(m))
	    { }
    };

    class SignalMsg : public Msg
    {
    public:
	SignalMsg(WvStringParm obj, WvStringParm ifc, WvStringParm method)
	    : Msg(dbus_message_new_signal(obj, ifc, method))
	{ }
    };

protected:
    Err xerr;
    DBusConnection *conn;
    void *userdata;

public:
    WvDBus(DBusBusType bus = DBUS_BUS_SESSION);
    WvDBus(DBusConnection *c);
    WvDBus(WvDBus &c);
    ~WvDBus();

    operator DBusConnection*()
    {
	return conn;
    }

    virtual bool isok() const
    {
	return conn;
    }

    void close();

    void request_name(WvStringParm srv, int flag = 0);

    Msg sendsync(const Msg &msg, time_t msec_timeout);
    void send(const Msg &msg);

    typedef WvCallback<void, WvDBus&, const Msg& > Callback;
    Callback callback;
    void setcallback(Callback cb)
    {
	callback = cb;
    }

    const char *unique_name()
    {
	return dbus_bus_get_unique_name(conn);
    }

    void add_match(WvStringParm filter);

    // Used to maintain state.
    void set_userdata(void *d);
    void *get_userdata() const;
};


#endif // __WVDBUS_H
