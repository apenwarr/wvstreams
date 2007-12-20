#include "wvtest.h"
#include "wvdbusmsg.h"

WVTEST_MAIN("dbusmsg basics")
{
    WvDBusMsg msg("my.dest", "/my/path", "my.ifc", "method");
    msg.append("yoink").append((int16_t)-1)
	.append((uint16_t)-1).append(true).append("-2")
	.append((int64_t)UINT_MAX+1).append(123.45);
    
    WVPASSEQ(msg.get_dest(), "my.dest");
    WVPASSEQ(msg.get_path(), "/my/path");
    WVPASSEQ(msg.get_interface(), "my.ifc");
    WVPASSEQ(msg.get_member(), "method");
    
    WVPASSEQ(msg.get_argstr(), "yoink,-1,65535,b1,-2,4294967296,123.45");
    
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
	
        int64_t ll = (int64_t)i.getnext();
        WVPASSEQ(ll, UINT_MAX+1);

        WVPASS(i.next());
	// WvTest doesn't have WVPASSEQ for doubles
	WVFAILEQ(WvString((double)i), "");
        WVPASS((double)i == 123.45);

	WVFAIL(i.next()); // no more parameters
	WVPASSEQ(i.type(), 0);
	WVPASS(i.get_str().isnull());
	WVPASSEQ((int)i, 0);
	WVPASSEQ((unsigned)i, 0);
    }
}


WVTEST_MAIN("dbusmsg arrays")
{
    WvDBusMsg msg("my.dest", "/my/path", "my.ifc", "method");
    WVPASS(true);
    msg.variant_start("i").append(5).variant_end();
    WVPASS(true);
    msg.struct_start("sib").append("str").append(5).append(true).struct_end();
    WVPASS(true);
    msg.array_start("s").append("one").append("two").array_end();
    WVPASS(true);
    msg.array_start("i").append(5).append(6).array_end();
    WVPASS(true);
    msg.array_start("v")
	.varray_start("s").append("str1").append("str2").varray_end()
	.varray_start("i").append(-5).append(-6).varray_end()
	.array_end();
    WVPASS(true);
    WVPASSEQ(msg.get_argstr(), "{5},[str,5,b1],[one,two],[5,6],[{[str1,str2]},{[-5,-6]}]");
}

WVTEST_MAIN("dbusmsg char signedness")
{
    unsigned char uc = UCHAR_MAX;
    signed char sc1 = CHAR_MAX;
    signed char sc2 = -1;
    signed char sc3 = CHAR_MIN;
    WvDBusMsg msg("my.dest", "/my/path", "my.ifc", "method");
    msg.array_start("y").append(uc).append(sc1).append(sc2).append(sc3).array_end();
    WVPASSEQ(msg.get_argstr(), "[y255,y127,y255,y128]");

    WvDBusMsg::Iter it(msg);
    it.rewind();
    it.next();
    WvDBusMsg::Iter ait(it.open());
    ait.rewind(); 
    ait.next();
    WVPASSEQ((unsigned char)ait, uc);
    WVPASSEQ(ait.get_int(), uc);
    ait.next();
    WVPASSEQ((signed char)ait, sc1);
    WVPASSEQ(ait.get_int(), (unsigned char)sc1);
    ait.next();
    WVPASSEQ((signed char)ait, sc2);
    WVPASSEQ(ait.get_int(), (unsigned char)sc2);
    ait.next();
    WVPASSEQ((signed char)ait, sc3);
    WVPASSEQ(ait.get_int(), (unsigned char)sc3);
}
