#include "wvtest.h"
#include "wvstring.h"


WVTEST_MAIN("basic")
{
    WvString a, b, c(""), d(""), e("hello"), f("Hello"), g(0), h(1), i(1.0),
             j(NULL);
    
    // null
    WVFAIL(a);
    WVPASS(a == NULL);
    WVFAIL(a == "");
    WVPASS(a.isnull());
    WVPASSEQ(a.ifnull("x"), "x");
    WVPASS(!a);
    WVFAIL(!!a);
    WVFAIL(a != NULL);
    WVPASS(a == b);
    WVFAIL(a != b);
    WVPASS(a.len() == 0);
    
    // blank
    WVPASS(c);
    WVPASS(c == "");
    WVFAIL(c == NULL);
    WVFAIL(c.isnull());
    WVPASSEQ(c.ifnull("x"), "");
    WVPASS(!c);
    WVFAIL(!!c);
    WVPASS(c == d);
    WVFAIL(c != d);
    WVFAIL(c == a);
    WVPASS(c != a);
    
    // real
    WVPASS(e);
    WVPASS(e == "hello");
    WVFAIL(e == "Hello");
    WVFAIL(e == f);
    WVFAIL(e+0 == f+0); // not a wvstring, == now compares pointers!
    WVPASS(e+0 == e+0);
    WVPASS(WvString(e+1) == "ello");
    WVPASS(WvString(e+1) == f+1);
    
    // numbers
    WVPASS(g == "0");
    WVPASS(h == "1");
    WVPASS(i == "1");
    WVFAIL(h != i);
    WVPASS(i.num() == 1);
    WVPASS(WvString(-1).num() == -1);
    
    // silly NULL crap should at least be well-defined
    WVPASS(j == "0");
}


WVTEST_MAIN("copying")
{
    WvString a1, b1, c1(""), d1(""), e1("hello"), f1("Hello"), g1(0),
    		h1(1), i1(1.0);
    WvString a2(a1), b2(b1), c2(c1), d2(d1), e2(e1), f2(f1), g2(g1), 
    		h2(h1), i2(i1);
    
    // if we didn't crash yet, we're halfway there!
    
    // equivalent pointers
    WVPASS(e1+0 == e2+0);
    WVPASS(e1.edit()+0 != e2+0);
    const char *olde1 = e1;
    { WvString x(e1); } // copy and destroy
    WVPASS(e1.edit() == olde1); // no unnecessary copies
    
    // make sure values are equivalent
    WVPASS(a1 == a2);
    WVPASS(b1 == b2);
    WVPASS(c1 == c2);
    WVPASS(d1 == d2);
    WVPASS(e1 == e2);
    WVPASS(f1 == f2);
    WVPASS(g1 == g2);
    WVPASS(h1 == h2);
    WVPASS(i1 == i2);
    WVFAIL(a2 == c2);
    
    // null/empty assignment
    a2 = c2;
    WVPASS(a2 == d1);
    d2 = b2;
    WVPASS(d2 == a1);
}


WVTEST_MAIN("append")
{
    WvString a, b, c(""), d("hello");
    
    // append
    a.append(b);
    b.append(c);
    c.append(d);
    d.append(a);
    WVPASS(a == NULL);
    WVPASS(b == "");
    WVPASS(c == "hello");
    WVPASS(d == "hello");
}


WVTEST_MAIN("formatting")
{
    WvString a, b, c(""), d("hello");
    
    // basic formatter
    WvString x("%s%s", a, b); // undefined, but shouldn't crash
    WVPASS(WvString("%s%s", c, d) == "hello");
    WVPASS(WvString("%s%s", d, d) == "hellohello");
    
    // format d, then assign to d
    d = WvString("%s%s%s%s", d, d, d, d);
    WVPASS(d == "hellohellohellohello");
    d = WvString(d);
    WVPASS(d == "hellohellohellohello");
    WVPASS(d.len() == 20);
}


WVTEST_MAIN("fancy formatting")
{
    // fancy formatter tests
    WVPASS(WvString("%s") == "%s");
    WVPASS(WvString("%%s") == "%%s");
    WVPASS(WvString("%s", "x") == "x");
    WVPASS(WvString("%%s", "x") == "%s");
    WVPASS(WvString("%-5s", "a") == "a    ");
    WVPASS(WvString("%5s", "a") == "    a");
    WVPASS(WvString("%3s", "hello") == "hello");
    WVPASS(WvString("%-3s", "hello") == "hello");
    WVPASS(WvString("%.3s", "hello") == "hel");
    WVPASS(WvString("%-6.3s", "hello") == "hel   ");
    WVPASS(WvString("%6.3s", "hello") == "   hel");
    WVPASS(WvString("%6.3s", "a") == "     a");
}

WVTEST_MAIN("conversion from int")
{
    for (int i = 0; i < 1000000; ++i)
    {
	WvString number(i);
    }
    WVPASSEQ(WvString(0), "0");
    WVPASSEQ(WvString(1), "1");
    WVPASSEQ(WvString(-1), "-1");
    WVPASSEQ(WvString(12), "12");
    WVPASSEQ(WvString(32767), "32767");
    WVPASSEQ(WvString(65535), "65535");
}

WVTEST_MAIN("offset")
{
    WvString emptystr("");

    WVPASSEQ(emptystr.offset(0), "");
    WVPASSEQ(emptystr.offset(1), "");
    WVPASSEQ(emptystr.offset(1000), "");

    WvString nullstr(WvString::null);

    WVPASSEQ(nullstr.offset(0), WvString::null);
    WVPASSEQ(nullstr.offset(1), WvString::null);
    WVPASSEQ(nullstr.offset(2), WvString::null);

    WvString str("foobar");

    WVPASSEQ(str.offset(0), "foobar");
    WVPASSEQ(str.offset(1), "oobar");
    WVPASSEQ(str.offset(1).offset(2), "bar");
    WVPASSEQ(str.offset(3), str.offset(1).offset(2));
    WVPASSEQ(str.offset(6), "");
    WVPASSEQ(str.offset(100), "");
}

class WvFooString : public WvFastString
{
public:
	static unsigned get_nullbuf_links()
	{ return nullbuf.links; }
};
WVTEST_MAIN("nullbuf link counting")
{
    const WvString &ws = "WvString";
    const WvFastString &wfs = "WvString";

    unsigned before = WvFooString::get_nullbuf_links();
    printf("nullbuf has %u links before\n", before);
    {
	WvString a("b");
	WvString b(ws);
	WvString c(wfs);
    }
    unsigned after = WvFooString::get_nullbuf_links();
    printf("nullbuf has %u links after\n", after);

    // BUGZID:20626
    // ensure that we don't leak references when creating WvStrings
    WVPASS(before == after);
}
