#include "wvtest.h"
#include "uniconfkey.h"

WVTEST_MAIN("slash collapsing")
{
    WVPASSEQ(UniConfKey().printable(), "");
    WVPASSEQ(UniConfKey().numsegments(), 0);
    WVPASSEQ(UniConfKey("").printable(), "");
    WVPASSEQ(UniConfKey("").numsegments(), 0);
    WVPASSEQ(UniConfKey("/").printable(), "");
    WVPASSEQ(UniConfKey("////").printable(), "");
    WVPASSEQ(UniConfKey("///").numsegments(), 0);

    WVPASSEQ(UniConfKey("foo").printable(), "foo");
    WVPASSEQ(UniConfKey("foo").numsegments(), 1);
    WVPASSEQ(UniConfKey("/foo").printable(), "foo");
    WVPASSEQ(UniConfKey("/foo").numsegments(), 1);
    WVPASSEQ(UniConfKey("foo/").printable(), "foo/");
    WVPASSEQ(UniConfKey("foo/").numsegments(), 2);
    WVPASSEQ(UniConfKey("/foo/").printable(), "foo/");
    WVPASSEQ(UniConfKey("/foo/").numsegments(), 2);

    WVPASSEQ(UniConfKey("//bar").printable(), "bar");
    WVPASSEQ(UniConfKey("///bar").printable(), "bar");
    WVPASSEQ(UniConfKey("bar//").printable(), "bar/");
    WVPASSEQ(UniConfKey("bar///").printable(), "bar/");
    WVPASSEQ(UniConfKey("///bar////").printable(), "bar/");

    WVPASSEQ(UniConfKey("fred/barney").printable(), "fred/barney");
    WVPASSEQ(UniConfKey("/fred/barney").printable(), "fred/barney");
    WVPASSEQ(UniConfKey("fred//barney").printable(), "fred/barney");
    WVPASSEQ(UniConfKey("fred///barney").printable(), "fred/barney");
    WVPASSEQ(UniConfKey("/fred///barney").printable(), "fred/barney");
    WVPASSEQ(UniConfKey("///fred///barney").printable(), "fred/barney");
    WVPASSEQ(UniConfKey("fred/barney/").printable(), "fred/barney/");
    WVPASSEQ(UniConfKey("fred/barney///").printable(), "fred/barney/");
    WVPASSEQ(UniConfKey("/fred/barney/").printable(), "fred/barney/");
    WVPASSEQ(UniConfKey("//fred///barney/").printable(), "fred/barney/");
    WVPASSEQ(UniConfKey("///fred////barney///").printable(), "fred/barney/");

    WVPASSEQ(UniConfKey("larry//////curly//////moe").printable(),
	     "larry/curly/moe");
    WVPASSEQ(UniConfKey("larry//////curly//////moe////////").printable(),
	     "larry/curly/moe/");
    WVPASSEQ(UniConfKey("////larry/////curly////moe///////").printable(),
	     "larry/curly/moe/");
}

WVTEST_MAIN("equality")
{
    WVPASS(UniConfKey() == UniConfKey("/"));
    WVPASS(UniConfKey("") == UniConfKey("/"));
    WVPASS(UniConfKey("baz") == UniConfKey("/baz"));
    WVPASS(UniConfKey("ack/nak") == UniConfKey("//ack///nak"));
    WVFAIL(UniConfKey("a") == UniConfKey("a/"));
    WVFAIL(UniConfKey("/a") == UniConfKey("a/"));
}

WVTEST_MAIN("composition")
{
    WVPASS(UniConfKey(UniConfKey("simon"), UniConfKey(""))
	   == UniConfKey("simon/"));
    WVPASSEQ(UniConfKey(UniConfKey("simon"), UniConfKey("")).printable(),
	     "simon/");

    WVPASS(UniConfKey(UniConfKey("simon"), UniConfKey("/"))
	   == UniConfKey("simon/"));
    WVPASSEQ(UniConfKey(UniConfKey("simon"), UniConfKey("/")).printable(),
	     "simon/");

    UniConfKey tmp(UniConfKey("simon"), UniConfKey("/"));
    WVPASS(UniConfKey(tmp, UniConfKey("law")) == UniConfKey("simon/law"));
    WVPASSEQ(UniConfKey(tmp, UniConfKey("law")).printable(), "simon/law");

    WVPASS(UniConfKey(UniConfKey("simon/"), UniConfKey(""))
	   == UniConfKey("simon/"));
    WVPASSEQ(UniConfKey(UniConfKey("simon/"), UniConfKey("")).printable(),
	     "simon/");

    WVPASS(UniConfKey(UniConfKey("simon/"), UniConfKey("law"))
	   == UniConfKey("simon/law"));
    WVPASSEQ(UniConfKey(UniConfKey("simon/"), UniConfKey("law")).printable(),
	     "simon/law");
}
