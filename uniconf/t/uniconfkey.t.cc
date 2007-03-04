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

WVTEST_MAIN("subkeys")
{
    WVPASS(UniConfKey().suborsame(UniConfKey("")));
    WVPASS(UniConfKey().suborsame(UniConfKey("cfg/ini")));
    WVPASS(UniConfKey("").suborsame(UniConfKey("cfg/ini")));
    WVPASS(UniConfKey("/").suborsame(UniConfKey("cfg/ini")));
    WVPASS(UniConfKey("cfg").suborsame(UniConfKey("cfg/ini")));
    WVPASS(UniConfKey("cfg/").suborsame(UniConfKey("cfg/ini")));
    WVPASS(UniConfKey("/cfg/ini").suborsame(UniConfKey("cfg/ini")));
    WVFAIL(UniConfKey("/cfg/ini/foo").suborsame(UniConfKey("cfg/ini")));
    WVFAIL(UniConfKey("/ini/cfg").suborsame(UniConfKey("cfg/ini")));

    WVPASSEQ(UniConfKey().subkey(UniConfKey("")).printable(), "");
    WVPASSEQ(UniConfKey().subkey(UniConfKey("cfg/ini")).printable(), "cfg/ini");
    WVPASSEQ(UniConfKey("/").subkey(UniConfKey("cfg/ini")).printable(), "cfg/ini");
    WVPASSEQ(UniConfKey("cfg").subkey(UniConfKey("cfg/ini")).printable(), "ini");
    WVPASSEQ(UniConfKey("/cfg/ini").subkey(UniConfKey("cfg/ini")).printable(), "");
}

WVTEST_MAIN("members")
{
    UniConfKey key("//this///is/a/key");
    WVPASSEQ(key.printable(), "this/is/a/key");
    key.append("/end//key");
    WVPASSEQ(key.printable(), "this/is/a/key/end/key");
    key.prepend("/start/key");
    WVPASSEQ(key.printable(), "start/key/this/is/a/key/end/key");
    WVPASS(!key.isempty());
    WVPASS(!key.iswild());
    WVPASS(!key.hastrailingslash());
    WVPASSEQ(key.numsegments(), 8);
    WVPASSEQ(key.segment(0).printable(), "start");
    UniConfKey sub(key.segment(3));
    WVPASSEQ(key.segment(3).printable(), "is");
    WVPASSEQ(key.segment(7).printable(), "key");
    WVPASSEQ(key.pop(3).printable(), "start/key/this");
    WVPASSEQ(key.printable(), "is/a/key/end/key");
    WVPASSEQ(key.first(3).printable(), "is/a/key");
    WVPASSEQ(key.last(3).printable(), "key/end/key");
    WVPASSEQ(key.removefirst(3).printable(), "end/key");
    WVPASSEQ(key.removelast(3).printable(), "is/a");
    WVPASSEQ(key.range(1, 4).printable(), "a/key/end");
    key = UniConfKey("foo/bar");
    WVPASSEQ(key.printable(), "foo/bar");
    WVPASS(key.compareto("foo/bar") == 0);
    WVPASS(key.compareto("Foo/Bar") == 0);
    WVPASS(key.compareto("foo/a") > 0);
    WVPASS(key.compareto("foo/bar/a") < 0);
    WVPASS(key.compareto("foo/z") < 0);
    WVPASS(key.compareto("g") < 0);
    WVPASS(key.suborsame("foo/bar"));
    WVPASS(key.suborsame("foo/bar/baz"));
    WVPASS(!key.suborsame("foo"));
    WVPASS(!key.suborsame("foo/baz"));
    WVPASS(!key.suborsame("green"));
    WVPASSEQ(key.subkey("foo/bar/baz").printable(), "baz");
}
WVTEST_MAIN("range")
{
    WVPASSEQ(UniConfKey().range(0,0).printable(), "");
    WVPASSEQ(UniConfKey().range(0,1).printable(), "");
    WVPASSEQ(UniConfKey().range(1,2).printable(), "");
    WVPASSEQ(UniConfKey("").range(0,0).printable(), "");
    WVPASSEQ(UniConfKey("").range(0,1).printable(), "");
    WVPASSEQ(UniConfKey("").range(1,2).printable(), "");
    WVPASSEQ(UniConfKey("fred").range(0,0).printable(), "");
    WVPASSEQ(UniConfKey("fred").range(0,1).printable(), "fred");
    WVPASSEQ(UniConfKey("fred").range(1,2).printable(), "");
    WVPASSEQ(UniConfKey("fred/barney").range(0,0).printable(), "");
    WVPASSEQ(UniConfKey("fred/barney").range(0,1).printable(), "fred");
    WVPASSEQ(UniConfKey("fred/barney").range(1,2).printable(), "barney");
    WVPASSEQ(UniConfKey("fred/barney").range(0,2).printable(), "fred/barney");
    WVPASSEQ(UniConfKey("fred/barney/betty").range(0,2).printable(), "fred/barney");
    WVPASSEQ(UniConfKey("fred/barney/betty").range(1,3).printable(), "barney/betty");
    WVPASSEQ(UniConfKey("fred/barney/betty").range(2,3).printable(), "betty");
}
