#include <wvtest.h>
#include <wvdiriter.h>
#include <wvfile.h>
#include <wvhashtable.h>
#include <wvstring.h>

static bool create_dir(WvStringParm dir, const WvStringList &entries)
{
    if (mkdir(dir, 0700)) return false;

    WvStringList::Iter entry(entries);
    for (entry.rewind(); entry.next(); )
    {
        WvString name("%s/%s", dir, *entry);
        mkdir(getdirname(name), 0700);
        WvFile(name, O_CREAT | O_EXCL, 0600).print("wvtest");
    }

    return true;
}

static bool destroy_dir(WvStringParm dir)
{
    system(WvString("rm -rf %s", dir));

    return true;
}

WVTEST_MAIN("Non-recursive WvDirIter")
{
    WvString dir("/tmp/wvtest-wvdiriter-%s", getpid());

    WvStringList entries;
    entries.split("file-one file-two .dot-file subdir/sub-file");
    
    WVPASS(create_dir(dir, entries));

    WvMap<WvString, bool> found(8);

    WvDirIter di(dir, false);
    for (di.rewind(); di.next(); )
        found.set(di->relname, true);

    WVFAIL(found.exists("."));
    WVFAIL(found.exists(".."));
    WVPASS(found.exists("file-one"));
    WVPASS(found.exists("file-two"));
    WVPASS(found.exists(".dot-file"));
    WVPASS(found.exists("subdir"));
    WVFAIL(found.exists("subdir/."));
    WVFAIL(found.exists("subdir/.."));
    WVFAIL(found.exists("subdir/sub-file"));
    
    WVPASS(destroy_dir(dir));
}

WVTEST_MAIN("Recursive WvDirIter")
{
    WvString dir("/tmp/wvtest-wvdiriter-%s", getpid());

    WvStringList entries;
    entries.split("file-one file-two .dot-file subdir/sub-file");
    
    WVPASS(create_dir(dir, entries));

    WvMap<WvString, bool> found(8);

    WvDirIter di(dir, true);
    for (di.rewind(); di.next(); )
        found.set(di->relname, true);

    WVFAIL(found.exists("."));
    WVFAIL(found.exists(".."));
    WVPASS(found.exists("file-one"));
    WVPASS(found.exists("file-two"));
    WVPASS(found.exists(".dot-file"));
    WVPASS(found.exists("subdir"));
    WVFAIL(found.exists("subdir/."));
    WVFAIL(found.exists("subdir/.."));
    WVPASS(found.exists("subdir/sub-file"));
    
    WVPASS(destroy_dir(dir));
}
