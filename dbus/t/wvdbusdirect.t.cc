#include "wvdbusconn.h"
#include "wvtest.h"
#include <dbus/dbus.h>


static bool got_name(WvDBusConn &c, WvDBusMsg &msg)
{
    WVPASS(!msg.iserror());
    return true;
}


static bool got_error(WvDBusConn &c, WvDBusMsg &msg)
{
    WVPASS(msg.iserror());
    return true;
}


static bool got_timeout(WvDBusConn &c, WvDBusMsg &msg)
{
    WVPASS(msg.iserror());
    return true;
}


WVTEST_MAIN("dbus direct")
{
    WvDBusConn c("unix:@/tmp/dbus-DKjFYBM884");

    c.request_name("a.b.boink", got_name);
    c.request_name("/a/b/invalid", got_error);
    c.send(WvDBusMsg("a.b.boink", "/a/b/stinky",
		     "a.b.stinky", "winky"),
	   got_timeout, 100);
    
    while (c.isok() && !c.isidle())
	c.runonce(-1);
    
    if (c.geterr())
	fprintf(stderr, "Error was: %s\n", c.errstr().cstr());
    WVPASS(!c.geterr());
}


