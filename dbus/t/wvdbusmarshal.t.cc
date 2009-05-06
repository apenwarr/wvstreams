#include "wvtest.h"
#include "wvbuf.h"
#include "wvdbusmsg.h"
#include "wvstream.h"
#include "wvstrutils.h"

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
    decoded = WvDBusMsg::demarshal(buf);
    WVPASS(!decoded);
    
    msg.marshal(buf);
    WVPASS(buf.used() > 0);
    buf.putstr("BOOGA");
    msg2.marshal(buf);
    size_t used = buf.used();
    WVPASS(buf.used() > 0);
    WVPASSEQ(msg.get_argstr(), "string1,2,[{[10,11]},{[wX,Yz]}],42");
    WVPASSEQ(msg2.get_argstr(), "");
    wvout->print("%s\n", hexdump_buffer(buf.peek(0, used), used));
    
    WVPASS(buf.used() > 0);
    decoded = WvDBusMsg::demarshal(buf);
    WVPASS(buf.used() > 5); // still data left
    WVPASS(decoded);
    if (decoded)
	WVPASSEQ(decoded->get_argstr(), "string1,2,[{[10,11]},{[wX,Yz]}],42");
    
    if (buf.used() > 5)
	WVPASSEQ(buf.getstr(5), "BOOGA");
    if (decoded)
	delete decoded;
    decoded = WvDBusMsg::demarshal(buf);
    WVPASS(buf.used() == 0);
    WVPASS(decoded);
    if (decoded)
    {
	WVPASSEQ(decoded->get_argstr(), "");
	delete decoded;
    }
}
