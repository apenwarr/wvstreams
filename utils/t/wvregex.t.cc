#include "wvtest.h"
#include "wvregex.h"
#include "wvstream.h"


WVTEST_MAIN("basic syntax")
{
    WvRegex re("ab+c", WvRegex::BASIC);
    
    WVFAIL(re.match(""));
    WVFAIL(re.match("a"));
    WVFAIL(re.match("ac"));
    WVFAIL(re.match("abc"));
    WVPASS(re.match("ab+c"));
    WVFAIL(re.match("prefixabbcsuffix"));
    WVFAIL(re.match("abbc"));
    WVFAIL(re.match("abbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbc"));
    WVFAIL(re.match("adc"));
}


WVTEST_MAIN("extended syntax")
{
    WvRegex re("ab+c", WvRegex::EXTENDED);
    
    WVFAIL(re.match(""));
    WVFAIL(re.match("a"));
    WVFAIL(re.match("ac"));
    WVPASS(re.match("abc"));
    WVPASS(re.match("prefixabbcsuffix"));
    WVPASS(re.match("abbc"));
    WVPASS(re.match("abbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbc"));
    WVFAIL(re.match("adc"));
}


WVTEST_MAIN("registers")
{
    WvRegex re("(xyz)a(b*c)([0-9]*)");
    WvString reg1, reg2, reg3;
    
    WVFAIL(re.match("asodfbaiosdfbn", reg1, reg2, reg3));
    WVPASS(re.match("xyzabbbbc123456789", reg1, reg2, reg3));
    WVPASS(reg1 == "xyz");
    WVPASS(reg2 == "bbbbc");
    WVPASS(reg3 == "123456789");
}


WVTEST_MAIN("match start/end")
{
    WvRegex re("Wv(Stream|String)");
    int match_start, match_end;
    WvString reg;
    
    WVPASS(re.match("This is WvStreams", match_start, match_end));
    WVPASS(match_start == 8);
    WVPASS(match_end == 16);
    
    WVPASS(re.match("WvString is part of WvStreams",
    	    match_start, match_end, reg));
    WVPASS(match_start == 0);
    WVPASS(match_end == 8);
    WVPASS(reg == "String");
}



WVTEST_MAIN("eflags")
{
    WvRegex re("^WvStream$");
    
    WVPASS(re.match("WvStream"));
    WVFAIL(re.match("WvStream", WvRegex::NOTBOL));
    WVFAIL(re.match("WvStream", WvRegex::NOTEOL));
    WVFAIL(re.match("WvStream", WvRegex::NOTBOL | WvRegex::NOTEOL));
}


WVTEST_MAIN("all match arguments at once")
{
    WvRegex re("^WvStream");
    int match_start, match_end;
    WvString reg;
    
    WVPASS(re.match("WvStream", match_start, match_end, reg));
    WVFAIL(re.match("WvStream", WvRegex::NOTBOL, match_start, match_end, reg));
}
