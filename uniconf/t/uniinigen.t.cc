#include "wvtest.h"
#include "wvfile.h"
#include "uniconfroot.h"


static void inigen(WvStringParm content)
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
    
    WVPASSEQ(cfg["S1/a"].get(), "b");
    WVPASSEQ(cfg["S2/c"].get(), "d");
    WVPASSEQ(cfg["S\n3/e"].get(), "f");
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
    
    WVPASSEQ(cfg["a"].get(), "b c");
    WVPASSEQ(cfg[" a\n  b}  {c  "].get(), "  a\n  b2}  {c  ");
    WVPASSEQ(cfg["apenwarr"].get(), "{OBFU}scation!");
}


WVTEST_MAIN("parsing3")
{
    inigen("/ = foo\n");
    UniConfRoot cfg("ini:tmp.ini");
    WVPASSEQ(cfg.get(), "foo");
    WVFAIL(cfg.haschildren());
}


static void inicmp(WvStringParm key, WvStringParm val, WvStringParm content)
{
    inigen("");
    UniConfRoot cfg("ini:tmp.ini");
    cfg[key].set(val);
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
	   "\n[x/y]\nz = abc\n");
    
    inicmp("x\n/y/z", "abc",
	   "\n[{x\n/y}]\nz = abc\n");
    
    inicmp("/users/apenwarr", "{OBFU}scation!",
	   "\n[users]\napenwarr = {OBFU}scation!\n");
    
    inicmp("/users/ apenwarr", "pbc ",
	   "\n[users]\n{ apenwarr} = {pbc }\n");
}

