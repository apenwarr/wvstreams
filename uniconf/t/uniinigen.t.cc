#include "uniinigen.h"
#include "wvfile.h"
#include "wvfileutils.h"
#include "uniconfroot.h"
#ifdef _WIN32
#include <sys/stat.h>
#endif
#include "uniwatch.h"
#include "wvsystem.h"
#include "wvtest.h"
#include "uniconfgen-sanitytest.h"

#ifdef _WIN32
static inline int mkdir(const char *x, int y) { return mkdir(x); }
#endif

// Returns the filename where the content was written.  This file must be
// deleted when no longer needed.  Returns WvString::null if unable to create
// the file.
WvString inigen(WvStringParm content)
{
    WvString ininame = wvtmpfilename("inigen_test.ini");
    WvFile file;
    WVPASS(file.open(ininame, O_CREAT|O_RDWR));
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


WVTEST_MAIN("UniIniGen Sanity Test")
{
    WvString inifile("/tmp/inigen-test-%s.ini", getpid());
    UniIniGen *gen = new UniIniGen(inifile);
    UniConfGenSanityTester::sanity_test(gen, WvString("ini:%s", inifile));
    WVRELEASE(gen);
    unlink(inifile);
}

WVTEST_MAIN("commit-without-refresh")
{

    UniConfRoot cfg("ini:/dev/does-not-exist");
    cfg.commit();
    cfg.refresh();
    cfg.commit();
    WVFAIL(cfg.haschildren());
}

#ifndef _WIN32
WVTEST_MAIN("ini file permissions")
{
    system("rm -f perm.ini");
    system("touch perm.ini");
    UniConfRoot cfg("ini:perm.ini");
    cfg["foo"].setme("bar");
    mode_t oldmask = umask(02);
    cfg.commit();
    umask(oldmask);
    
    struct stat statbuf;
    WVPASS(stat("perm.ini", &statbuf) == 0);
    WVPASSEQ(statbuf.st_mode, 0100664); //file and permissions 0664

    system("rm -f perm.ini");
}
#endif

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


WVTEST_MAIN("parsing4")
{
    // first line should be dropped to try to recover, since the entire
    // file is invalid tcl-encoding.
    WvString ininame = inigen("{ broken\n/ = boo\n");
    UniConfRoot cfg(WvString("ini:%s", ininame));
    WVPASSEQ(cfg.getme(), "boo");
    WVFAIL(cfg.haschildren());
}


WVTEST_MAIN("Setting and getting (bug 6090)")
{
    WvString ininame = inigen("");
    UniConfRoot cfg(WvString("ini:%s", ininame));

    cfg["mrwise"].setme("{{bork!");
    cfg.commit();
    WVPASSEQ(cfg["mrwise"].getme(), "{{bork!");

    UniConfRoot cfg2(WvString("ini:%s", ininame));
    WVPASSEQ(cfg2["mrwise"].getme(), "{{bork!");

    ::unlink(ininame);
}

// This is probably covered by sanity tests now.  OTOH, more tests is more
// better.
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

static void count_cb(int *i, const UniConf &cfg, const UniConfKey &key)
{
    (*i)++;
}


WVTEST_MAIN("ini callbacks")
{
    int i = 0;
    WvString ininame = inigen("a/b/c/1 = 11\n"
			      "a/b/c/2 = 22\n");
    UniConfRoot cfg(WvString("ini:%s", ininame));
    UniWatch w(cfg, wv::bind(&count_cb, &i, _1, _2), true);

    WVPASSEQ(i, 0);
    cfg.refresh();
    WVPASSEQ(i, 0);
    
    {
	WvFile f(ininame, O_WRONLY|O_TRUNC);
	f.print("a/b/c/1 = 111\n"
		"a/b/c/2 = 222\n");
	cfg.refresh();
	WVPASSEQ(i, 2);
	
	f.print("a/b/c/3 = 333\n");
	cfg.refresh();
	WVPASSEQ(i, 3);
	
	f.print("\n");
	cfg.refresh();
	WVPASSEQ(i, 3);
    }
    
    cfg.xset("x", "y");
    WVPASSEQ(i, 4);
    cfg.commit();
    WVPASSEQ(i, 4);
    cfg.refresh();
    WVPASSEQ(i, 4);
    
    ::unlink(ininame);
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


static ino_t inode_of(const char *fname)
{
    struct stat st;
    if (stat(fname, &st) != 0)
	return 0;
    else
	return st.st_ino;
}


static off_t size_of(const char *fname)
{
    struct stat st;
    if (stat(fname, &st) != 0)
	return 0;
    else
	return st.st_size;
}


WVTEST_MAIN("atomic updates")
{
    WvString dirname("atomic-update-dir.tmp"), fname("%s/test.ini", dirname);
    chmod(dirname, 0700);
    WvSystem("rm", "-rf", dirname);
    WVPASS(!mkdir(dirname, 0700)); // honours umask
    WVPASS(!chmod(dirname, 0700)); // doesn't include umask
    
    // the directory is writable, so we can safely do atomic file replacement.
    // That means the file inode number will be *different* after doing
    // a second commit().
    UniConfRoot ini(WvString("ini:%s", fname));
    ini.xset("useless key", "useless value");
    ini.commit();
    ino_t inode1 = inode_of(fname);
    off_t size1 = size_of(fname);
    WVFAILEQ(inode1, 0);
    WVFAILEQ(size1, 0);
    ini.xset("1", "2");
    ini.commit();
    ino_t inode2 = inode_of(fname);
    off_t size2 = size_of(fname);
    WVFAILEQ(inode2, 0);
    WVFAILEQ(inode1, inode2);
    WVPASS(size2 > size1);
    
    // now let's make the directory non-writable.  The inifile inside the
    // directory is still writable, which means we can update it, but not
    // atomically.  Therefore its inode number *won't* change.
    WVPASS(!chmod(dirname, 0500));
    ini.xset("3", "4");
    ini.commit();
    ino_t inode3 = inode_of(fname);
    off_t size3 = size_of(fname);
    WVFAILEQ(inode3, 0);
    WVPASSEQ(inode2, inode3);
    WVPASS(size3 > size2);
    
    // clean up
    chmod(dirname, 0700);
    WvSystem("rm", "-rf", dirname);
}


WVTEST_MAIN("do not refresh if not requested")
{
    WvString ininame = inigen("[foo]\n"
			      "bar = baz\n");
    UniConfRoot cfg(WvString("ini:%s", ininame), false);

    WVFAIL(cfg["foo/bar"].exists());
    WVFAILEQ(cfg["foo/bar"].getme(), "baz");
    WVPASSEQ(childcount(cfg), 0);

    ::unlink(ininame);
}

