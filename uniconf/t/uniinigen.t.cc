#include "wvtest.h"
#include "wvfile.h"
#include "uniconfroot.h"


// Returns the filename where the content was written.  This file must be
// deleted when no longer needed.  Returns WvString::null if unable to create
// the file.
WvString inigen(WvStringParm content)
{
    int fd;
    WvString ininame = "/tmp/inigen_test.ini-XXXXXX";
    if ((fd = mkstemp(ininame.edit())) == (-1))
        return WvString::null;
    WvFile file(fd);
    file.write(content);
    WVPASS(file.isok());

    return ininame;
}


static int childcount(UniConf cfg)
{
    int count = 0;
    UniConf::Iter i(cfg);
    for (i.rewind(); i.next(); )
	count++;
    return count;
}
//FIXME: this test leaks because mounting now sets / to ""
/*
WVTEST_MAIN("commit-without-refresh")
{

    UniConfRoot cfg("ini:/dev/does-not-exist");
    cfg.commit();
    cfg.refresh();
    cfg.commit();
    WVFAIL(cfg.haschildren());
}*/


WVTEST_MAIN("parsing1")
{
    WvString ininame = inigen("[S1]\n"
	   "a = b\n"
	   "[{S2}]  \n"
	   "c=d  \n"
	   "[{S\n3}]\n"
	   "e=f\n");
    UniConfRoot cfg(WvString("ini:%s", ininame));
    
    WVPASSEQ(cfg["S1/a"].getme(), "b");
    WVPASSEQ(cfg["S2/c"].getme(), "d");
    WVPASSEQ(cfg["S\n3/e"].getme(), "f");
    WVPASSEQ(childcount(cfg), 3);

    ::unlink(ininame);
}


WVTEST_MAIN("parsing2")
{
    WvString ininame = inigen("[x]\n"
	   "[]\n"
	   "  a    =    b c   \n"
	   "  { a\n  b}  {c  }   = {  a\n  b2}  {c  }\n"
	   "apenwarr = {OBFU}scation!");
    UniConfRoot cfg(WvString("ini:%s", ininame));
    
    WVPASSEQ(cfg["a"].getme(), "b c");
    WVPASSEQ(cfg[" a\n  b}  {c  "].getme(), "  a\n  b2}  {c  ");
    WVPASSEQ(cfg["apenwarr"].getme(), "{OBFU}scation!");

    ::unlink(ininame);
}


WVTEST_MAIN("parsing3")
{
    WvString ininame = inigen("/ = foo\n");
    UniConfRoot cfg(WvString("ini:%s", ininame));
    WVPASSEQ(cfg.getme(), "foo");
    WVFAIL(cfg.haschildren());

    ::unlink(ininame);
}


WVTEST_MAIN("Setting and getting (bug 6090)")
{
    WvString ininame = inigen("");
    UniConfRoot cfg(WvString("ini:%s", ininame));
    cfg["mrwise"].setme("{{bork!");
    
    WVPASSEQ(cfg["mrwise"].getme(), "{{bork!");

    cfg.commit();
    UniConfRoot cfg2(ininame);
    //WVPASSEQ(cfg2["mrwise"].getme(), "{{bork!");
    
    ::unlink(ininame);
}

WVTEST_MAIN("Trailing slashes")
{
    UniConfRoot cfg("ini:tmp.ini");

    cfg["sfllaw"].setme("law");
    WVPASSEQ(cfg["sfllaw"].getme(), "law");
    WVPASSEQ(cfg["sfllaw/"].getme(), "");

    cfg["sfllaw/"].setme("LAW");
    WVPASSEQ(cfg["sfllaw"].getme(), "law");
    WVPASSEQ(cfg["sfllaw/"].getme(), "");
}

static void inicmp(WvStringParm key, WvStringParm val, WvStringParm content)
{
    WvString ininame = inigen("");
    UniConfRoot cfg(WvString("ini:%s", ininame));
    cfg[key].setme(val);
    cfg.commit();
    
    WvFile f(ininame, O_RDONLY);
    WvDynBuf buf;
    f.read(buf, 128*1024);
    WVPASSEQ(content, buf.getstr());
    ::unlink(ininame);
}


WVTEST_MAIN("writing")
{
    inicmp("/", "test",
	   "/ = test\n");
    
    inicmp("x/y", "z",
	   "\n[x]\ny = z\n");
    
    inicmp("x/y/z", "abc",
	   "\n[x]\ny/z = abc\n");
    
    inicmp("x\n/y/z", "abc",
	   "\n[{x\n}]\ny/z = abc\n");
    
    inicmp("/users/apenwarr", "{OBFU}scation!",
	   "\n[users]\napenwarr = {OBFU}scation!\n");
    
    inicmp("/users/ apenwarr", "pbc ",
	   "\n[users]\n{ apenwarr} = {pbc }\n");
    
    // FIXME: empty-string keys probably *should* be printed, but we don't,
    // for compatibility with WvConf.  This test makes sure it stays that
    // way (until we think of something better?)
    inicmp("/foo/blah", "",
	   "");
}

