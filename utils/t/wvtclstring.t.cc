#include "wvtest.h"
#include "wvtclstring.h"
#include "wvstring.h"

DeclareWvList(WvString);
WVTEST_MAIN("escaping and unescaping")
{
    WvString a, b("jabba"), c("crab poody-doo"), d("pooky{doo"), result, desired;
    
    //null
    result = wvtcl_escape(a);
    WVFAIL(result);
    result = wvtcl_unescape(result);
    WVPASS(result == a);
    
    //no escape required
    result = wvtcl_escape(b);
    WVPASS(result == b);
    result = wvtcl_unescape(result);
    WVPASS(result == b);
    
    //bracket escape required
    result = wvtcl_escape(c);
    desired = WvString("{%s}", c.cstr());
    WVPASS(result == desired);
    result = wvtcl_unescape(result);
    WVPASS(result == c);
    
    //\ escaping required
    result = wvtcl_escape(d);
    desired = WvString("pooky\\{doo");
    WVPASS(result == desired);
    result = wvtcl_unescape(result);
    WVPASS(result == d);
}

WVTEST_MAIN("encoding and decoding")
{
    WvString a("jabba"), b("crab poody-doo"), c("pooky{doo"), result, desired;
    WvStringList list;
    list.append(&a, false);
    list.append(&b, false);
    list.append(&c, false); 
    list.append(new WvString(), true);
    result = wvtcl_encode(list, " ", " ");
    desired = WvString("%s {%s} pooky\\{doo ", a, b);
    WVPASS(result == desired);
    
    wvtcl_decode(list, result);
    result = *list.first();
    list.unlink_first();
    WVPASS(result == a);

    result = *list.first();
    list.unlink_first();
    WVPASS(result == b);    

    result = *list.first();
    list.unlink_first();
    WVPASS(result == c);
    
    result = *list.first();
    WVFAIL(result);
    list.unlink_first();
}

WVTEST_MAIN("getword")
{
    WvDynBuf buf;
    WvString result, desired, test("jabba {crab poody-doo} pooky\\{doo");
    
    // set buffer
    buf.putstr(test);
    // don't unescape
    result = wvtcl_getword(buf, " ", false);
    desired = WvString("jabba");
    WVPASS(result == desired);
    // buffer should be updated properly
    result = buf.getstr();
    desired = WvString("{crab poody-doo} pooky\\{doo");
    WVPASS(result == desired); 
    
    // reset buffer
    buf.putstr(test);
    // unescape
    result = wvtcl_getword(buf, " ");
    desired = WvString("jabba");
    WVPASS(result == desired);
    // buffer should be updated properly
    result = wvtcl_getword(buf, " ");
    desired = WvString("crab poody-doo");
    WVPASS(result == desired);
    result = buf.getstr();
    desired = WvString("pooky\\{doo");
    WVPASS(result == desired);

    // test no word possible
    test = WvString("    ");
    buf.putstr(test);
    result = wvtcl_getword(buf, " ");
    // should return null
    WVFAIL(result);
    result = buf.getstr();
    // buffer should be reset to original state, ie unchanged
    WVPASS(result == test);
}
