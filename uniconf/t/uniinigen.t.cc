#include "wvtest.h"
#include "wvfile.h"
#include "uniconfroot.h"


// static void inigen(WvStringParm content)

void inigen(WvStringParm content)
{
    ::unlink("tmp.ini");
    WvFile file("tmp.ini", O_WRONLY|O_TRUNC|O_CREAT);
    file.write(content);
    WVPASS(file.isok());
}


static int childcount(UniConf cfg)
{
    int count = 0;
    UniConf::Iter i(cfg);
    for (i.rewind(); i.next(); )
	count++;
    return count;
}


WVTEST_MAIN("commit-without-refresh")
{
    UniConfRoot cfg("ini:/dev/does-not-exist");
    cfg.commit();
    cfg.refresh();
    cfg.commit();
    WVFAIL(cfg.haschildren());
}


WVTEST_MAIN("parsing1")
{
    inigen("[S1]\n"
	   "a = b\n"
	   "[{S2}]  \n"
	   "c=d  \n"
	   "[{S\n3}]\n"
	   "e=f\n");
    UniConfRoot cfg("ini:tmp.ini");
    
    WVPASSEQ(cfg["S1/a"].getme(), "b");
    WVPASSEQ(cfg["S2/c"].getme(), "d");
    WVPASSEQ(cfg["S\n3/e"].getme(), "f");
    WVPASSEQ(childcount(cfg), 3);
}


WVTEST_MAIN("parsing2")
{
    inigen("[x]\n"
	   "[]\n"
	   "  a    =    b c   \n"
	   "  { a\n  b}  {c  }   = {  a\n  b2}  {c  }\n"
	   "apenwarr = {OBFU}scation!");
    UniConfRoot cfg("ini:tmp.ini");
    
    WVPASSEQ(cfg["a"].getme(), "b c");
    WVPASSEQ(cfg[" a\n  b}  {c  "].getme(), "  a\n  b2}  {c  ");
    WVPASSEQ(cfg["apenwarr"].getme(), "{OBFU}scation!");
}


WVTEST_MAIN("parsing3")
{
    inigen("/ = foo\n");
    UniConfRoot cfg("ini:tmp.ini");
    WVPASSEQ(cfg.getme(), "foo");
    WVFAIL(cfg.haschildren());
}
WVTEST_MAIN("Setting and getting (bug 6090)")
{
    UniConfRoot cfg("ini:tmp.ini");
    cfg["mrwise"].setme("{{bork!");
    
    WVPASSEQ(cfg["mrwise"].getme(), "{{bork!");

    cfg.commit();
    UniConfRoot cfg2("ini:tmp.ini");
//    WVPASSEQ(cfg2["mrwise"].getme(), "{{bork!");
}


static void inicmp(WvStringParm key, WvStringParm val, WvStringParm content)
{
    inigen("");
    UniConfRoot cfg("ini:tmp.ini");
    cfg[key].setme(val);
    cfg.commit();
    
    WvFile f("tmp.ini", O_RDONLY);
    WvDynBuf buf;
    f.read(buf, 128*1024);
    WVPASSEQ(content, buf.getstr());
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

