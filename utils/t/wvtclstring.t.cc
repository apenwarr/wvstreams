#include "wvtest.h"
#include "wvtclstring.h"
#include "wvstring.h"
#include "wvstringlist.h"
#include "wvstringmask.h"
#include "wvstream.h"

#include <signal.h>

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
    WVPASSEQ(result, a);
    
    result = wvtcl_escape(b);
    desired = WvString("{}");
    WVPASSEQ(result, desired);
    result = wvtcl_unescape(result);
    WVPASSEQ(result, b);
    
    
    //no escape required
    result = wvtcl_escape(c);
    WVPASSEQ(result, c);
    result = wvtcl_unescape(result);
    WVPASSEQ(result, c);
    
    
    //bracket escape required
    WVPASSEQ(wvtcl_escape(" foo"), "{ foo}");
    WVPASSEQ(wvtcl_escape("foo "), "{foo }");
    
    result = wvtcl_escape(d);
    desired = WvString("{%s}", d.cstr());
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]\n", result.cstr(), desired.cstr());
    result = wvtcl_unescape(result);
    WVPASS(result == d);
    
    result = wvtcl_escape(e);
    desired = WvString("{\"quote this!\"}");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]\n", result.cstr(), desired.cstr());
    result = wvtcl_unescape(WvString("\"quote this!\""));
    desired = WvString("quote this!");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]\n", result.cstr(), desired.cstr());
        
    result = wvtcl_escape(i);
    desired = WvString("{shameless{frog}parts}");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]\n", result.cstr(), desired.cstr());
    result = wvtcl_unescape(result);
    if (!WVPASS(result == i))
        printf("   because [%s] != [%s]\n", result.cstr(), i.cstr());
    
    result = wvtcl_escape(h);
    desired = WvString("{hammer\\}time}");
    printf("%s\n", result.cstr());
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]\n", result.cstr(), desired.cstr());
    result = wvtcl_unescape(result);
    if (!WVPASS(result == h))
        printf("   because [%s] != [%s]\n", result.cstr(), h.cstr());


    //\ escaping required
    result = wvtcl_escape(f);
    desired = WvString("pooky\\{doo");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]\n", result.cstr(), desired.cstr());
    result = wvtcl_unescape(result);
    if (!WVPASS(result == f))
        printf("   because [%s] != [%s]\n", result.cstr(), f.cstr());

    result = wvtcl_escape(g);
    desired = WvString("big\\}monkey\\ \\{potatoes");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]\n", result.cstr(), desired.cstr());
    result = wvtcl_unescape(result);
    if (!WVPASS(result == g))
        printf("   because [%s] != [%s]\n", result.cstr(), g.cstr());


    //escaping with our own nasties
    result = wvtcl_escape(j, WvStringMask("o-\nu"));
    desired = WvString("{wagloo-mas\nuffle}");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]\n", result.cstr(), desired.cstr());
    result = wvtcl_unescape(result);
    if (!WVPASS(result == j))
        printf("   because [%s] != [%s]\n", result.cstr(), j.cstr());

    result = wvtcl_escape(k, WvStringMask("$ky"));
    desired = WvString("nast\\y\\{bad\\{\\}\\}\\}\\$brea\\k");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]\n", result.cstr(), desired.cstr());
    result = wvtcl_unescape(result);
    printf("%s\n", result.cstr());
    if (!WVPASS(result == k))
        printf("   because [%s] != [%s]\n", result.cstr(), k.cstr());
}


WVTEST_MAIN("encoding and decoding")
{
    signal(SIGALRM, SIG_IGN);
    WvString a("jabba"), b("crab poody-doo"), c("pooky{doo"), result, desired;
    WvStringList list;
    list.append(&a, false);
    list.append(&b, false);
    list.append(&c, false); 
    list.append(new WvString(), true);
    result = wvtcl_encode(list, WvStringMask(' '), WvStringMask(' '));
    desired = WvString("%s {%s} pooky\\{doo ", a, b);
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]\n", result.cstr(), desired.cstr());
    
    wvtcl_decode(list, result);
    result = *list.first();
    list.unlink_first();
    if (!WVPASS(result == a))
        printf("   because [%s] != [%s]\n", result.cstr(), a.cstr());

    result = *list.first();
    list.unlink_first();
    if (!WVPASS(result == b))  
        printf("   because [%s] != [%s]\n", result.cstr(), b.cstr());

    result = *list.first();
    list.unlink_first();
    if (!WVPASS(result == c))
        printf("   because [%s] != [%s]\n", result.cstr(), c.cstr());
    
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
    result = wvtcl_getword(buf, WvStringMask(" "), false);
    desired = WvString("jabba");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]\n", result.cstr(), desired.cstr());
    // buffer should be updated properly
    result = buf.getstr();
    desired = WvString(" {crab poody-doo} pooky\\{doo");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]\n", result.cstr(), desired.cstr());
    
    // reset buffer
    buf.putstr(test);
    // unescape
    result = wvtcl_getword(buf, WvStringMask(" "));
    desired = WvString("jabba");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]\n", result.cstr(), desired.cstr());
    // buffer should be updated properly
    result = wvtcl_getword(buf, WvStringMask(" "));
    desired = WvString("crab poody-doo");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]\n", result.cstr(), desired.cstr());
    result = buf.getstr();
    desired = WvString(" pooky\\{doo");
    if (!WVPASS(result == desired))
        printf("   because [%s] != [%s]\n", result.cstr(), desired.cstr());

    // test no word possible
    test = WvString("---{incomplete-");
    buf.putstr(test);
    result = wvtcl_getword(buf, WvStringMask("-"));
    // should return null
    WVFAIL(result);
    result = buf.getstr();
    if (!WVPASS(result == test))
        printf("   because [%s] != [%s]\n", result.cstr(), test.cstr());
    // test no word possible and whitespace eating
    test = WvString("    ");
    buf.putstr(test);
    result = wvtcl_getword(buf, WvStringMask(" "));
    // should return null
    WVFAIL(result);
    result = buf.getstr();
    // buffer should be reset to original state, ie unchanged
    if (!WVPASS(result == test))
        printf("   because [%s] != [%s]\n", result.cstr(), test.cstr());
}


static void _do_word(WvBuf &buf, WvStringParm word, size_t expect)
{
    WvString new_word = wvtcl_getword(buf, WVTCL_NASTY_NEWLINES);
    WVPASSEQ(buf.used(), expect);
    WVPASSEQ(word, new_word);
}


static void do_word(WvBuf &buf, WvStringParm word, size_t expect)
{
    buf.putstr("%s\n", word);
    _do_word(buf, word, expect);
}


WVTEST_MAIN("getword with dynamic buffer")
{
    WvDynBuf buf;
    WvString word;
    
    buf.putstr("\n\n\n");
    do_word(buf, "foo", 1);
    buf.zap();
    
    do_word(buf, "n = 6", 1);
    do_word(buf, "", 2); // left old newline *and* new newline
    do_word(buf, "[S1]", 1);
    do_word(buf, "a = b", 1);
    do_word(buf, "", 2);
    do_word(buf, "[S2]", 1);
    do_word(buf, "Enable = 0", 1);
}


WVTEST_MAIN("wvtcl_encode specific failures")
{
    WvStringList words;
    wvout->print(wvtcl_encode(words));
    words.append("{{2304 5816322} var} 1102878961 DIR");
    words.append("{{2304 5816323} lib} 1098721377 DIR");
    words.append("{{2304 5852807} defoma} 1098721283 DIR");
    words.append("{{2304 5852904} gs.d} 1098721283 DIR");
    words.append("{{2304 5852905} dirs} 1098721269 DIR");
    words.append("{{2304 5852906} fonts} 1098721283 DIR");
    words.append("{{2304 5853099} Fontmap} 1098721283 REG");
    wvout->print(wvtcl_encode(words));
}


WVTEST_MAIN("mismatched braces")
{
    WvDynBuf buf;
    buf.putstr("close}\n{brace}\n{me\n");
    _do_word(buf, "close}", 13); // first closebrace isn't "special"
    _do_word(buf, "brace", 5);   // second word is in braces
    _do_word(buf, WvString(), 5); // last word is incomplete
}


WVTEST_MAIN("backslashed braces")
{
    WvString line("VAL a b\\}\\\n\\{c\\}\\ =\\ d\\\ne\\ =\\ f\\{");
    WvString encoded("%s\n", line);
    WvString word1("VAL"), word2("a"), word3("b}\n{c} = d\ne = f{");
    
    WvDynBuf buf;
    buf.putstr(encoded);
    WVPASSEQ(buf.used(), 34);
    
    WvString oline = wvtcl_getword(buf, WVTCL_NASTY_NEWLINES, false);
    WVPASSEQ(line, oline);
    WVPASSEQ(buf.used(), 1); // trailing newline
    
    buf.zap();
    buf.putstr(line);
    WvString w1 = wvtcl_getword(buf, WvStringMask(" "));
    WvString w2 = wvtcl_getword(buf, WvStringMask(" "));
    WvString w3 = wvtcl_getword(buf, WvStringMask(" "));
    WVPASSEQ(w1, word1);
    WVPASSEQ(w2, word2);
    WVPASSEQ(w3, word3);
}

WVTEST_MAIN("BUGZID:17077")
{
    const char *dirname = "{directory name ending in backslash \\}";
    WvDynBuf buf;
    buf.putstr(dirname);
    WVPASSEQ(wvtcl_getword(buf, WVTCL_NASTY_NEWLINES, false), dirname);

    const char *another_dirname = "directory name ending in backslash \\\\";
    buf.putstr(another_dirname);
    WVPASSEQ(wvtcl_getword(buf, WVTCL_NASTY_NEWLINES, false), another_dirname);
}
