#include "wvtest.h"
#include "wvtclstring.h"
#include "wvstring.h"

DeclareWvList(WvString);
WVTEST_MAIN("escaping and unescaping")
{
    WvString a, b(""), c("jabba"), d("crab poody-doo"), e("\"quote this!\""), 
        f("pooky{doo"), g("big}monkey {potatoes"), h("hammer\\}time"), 
        i("shameless{frog}parts"), j("wagloo-mas\nuffle"), 
        k("nasty{bad{}}}$break"), result, desired;
    
        
    //null
    result = wvtcl_escape(a);
    WVFAIL(result);
    result = wvtcl_unescape(result);
    if (!WVPASS(result == a))
        printf("   because [%s] != [%s]", result, a);
    
    result = wvtcl_escape(b);
    desired = WvString("{}");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]", result, desired);
    result = wvtcl_unescape(result);
    if (!WVPASS(result == b))
        printf("   because [%s] != [%s]", result, b);
    
    
    //no escape required
    result = wvtcl_escape(c);
    if (!WVPASS(result == c))
        printf("   because [%s] != [%s]", result, c);
    result = wvtcl_unescape(result);
    if (!WVPASS(result == c))
        printf("   because [%s] != [%s]", result, c);
    
    
    //bracket escape required
    result = wvtcl_escape(d);
    desired = WvString("{%s}", d.cstr());
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]", result, desired);
    result = wvtcl_unescape(result);
    WVPASS(result == d);
    
    result = wvtcl_escape(e);
    desired = WvString("{\"quote this!\"}");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]", result, desired);
    result = wvtcl_unescape(WvString("\"quote this!\""));
    desired = WvString("quote this!");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]", result, desired);
        
    result = wvtcl_escape(i);
    desired = WvString("{shameless{frog}parts}");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]", result, desired);
    result = wvtcl_unescape(result);
    if (!WVPASS(result == i))
        printf("   because [%s] != [%s]", result, i);
    
    result = wvtcl_escape(h);
    desired = WvString("{hammer\\}time}");
    printf("%s\n", result.cstr());
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]", result, desired);
    result = wvtcl_unescape(result);
    if (!WVPASS(result == h))
        printf("   because [%s] != [%s]", result, h);


    //\ escaping required
    result = wvtcl_escape(f);
    desired = WvString("pooky\\{doo");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]", result, desired);
    result = wvtcl_unescape(result);
    if (!WVPASS(result == f))
        printf("   because [%s] != [%s]", result, f);

    result = wvtcl_escape(g);
    desired = WvString("big\\}monkey\\ \\{potatoes");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]", result, desired);
    result = wvtcl_unescape(result);
    if (!WVPASS(result == g))
        printf("   because [%s] != [%s]", result, g);


    //escaping with our own nasties
    result = wvtcl_escape(j, "o-\nu");
    desired = WvString("{wagloo-mas\nuffle}");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]", result, desired);
    result = wvtcl_unescape(result);
    if (!WVPASS(result == j))
        printf("   because [%s] != [%s]", result, j);

    result = wvtcl_escape(k, "$ky");
    desired = WvString("nast\\y\\{bad\\{\\}\\}\\}\\$brea\\k");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]", result, desired);
    result = wvtcl_unescape(result);
    printf("%s\n", result.cstr());
    if (!WVPASS(result == k))
        printf("   because [%s] != [%s]", result, k);
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
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]", result, desired);
    
    wvtcl_decode(list, result);
    result = *list.first();
    list.unlink_first();
    if (!WVPASS(result == a))
        printf("   because [%s] != [%s]", result, a);

    result = *list.first();
    list.unlink_first();
    if (!WVPASS(result == b))  
        printf("   because [%s] != [%s]", result, b);

    result = *list.first();
    list.unlink_first();
    if (!WVPASS(result == c))
        printf("   because [%s] != [%s]", result, c);
    
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
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]", result, desired);
    // buffer should be updated properly
    result = buf.getstr();
    desired = WvString("{crab poody-doo} pooky\\{doo");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]", result, desired);
    
    // reset buffer
    buf.putstr(test);
    // unescape
    result = wvtcl_getword(buf, " ");
    desired = WvString("jabba");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]", result, desired);
    // buffer should be updated properly
    result = wvtcl_getword(buf, " ");
    desired = WvString("crab poody-doo");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]", result, desired);
    result = buf.getstr();
    desired = WvString("pooky\\{doo");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]", result, desired);

    // test no word possible
    test = WvString("---{incomplete-");
    buf.putstr(test);
    result = wvtcl_getword(buf, "-");
    // should return null
    WVFAIL(result);
    result = buf.getstr();
    if (!WVPASS(result == test))
        printf("   because [%s] != [%s]", result, test);
    // test no word possible and whitespace eating
    test = WvString("    ");
    buf.putstr(test);
    result = wvtcl_getword(buf, " ");
    // should return null
    WVFAIL(result);
    result = buf.getstr();
    // buffer should be reset to original state, ie unchanged
    if (!WVPASS(result == test))
        printf("   because [%s] != [%s]", result, test);
}
