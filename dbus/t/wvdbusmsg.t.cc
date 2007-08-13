#include "wvtest.h"
#include "wvdbusmsg.h"
#include <dbus/dbus.h>

WVTEST_MAIN("basics")
{
    WvDBusMsg msg("my.dest", "/my/path", "my.ifc", "method");
    msg.append("yoink").append((int16_t)-1)
	.append((uint16_t)-1).append(true).append("-2");
    
    WVPASSEQ(msg.get_dest(), "my.dest");
    WVPASSEQ(msg.get_path(), "/my/path");
    WVPASSEQ(msg.get_interface(), "my.ifc");
    WVPASSEQ(msg.get_member(), "method");
    
    WVPASSEQ(msg.get_argstr(), "yoink,-1,65535,1,-2");
    
    {
	WvDBusMsg::Iter i(msg);
	WvString s = i.getnext();
	int n1 = i.getnext();
	unsigned n2 = i.getnext();
	bool b = (int)i.getnext();
	
	WVPASSEQ(s, "yoink");
	WVPASSEQ(n1, -1);
	WVPASSEQ(n2, 0xFFFF);
	WVPASSEQ(b, true);
	
	WVPASS(i.next());
	WVPASSEQ(*i, "-2");
	WVPASSEQ(i, -2);
	WVPASSEQ((bool)i, true);
	
	WVFAIL(i.next()); // no more parameters
	WVPASSEQ(i.type(), DBUS_MESSAGE_TYPE_INVALID);
	WVPASS(i.get_str().isnull());
	WVPASSEQ((int)i, 0);
	WVPASSEQ((unsigned)i, 0);
    }
}


