#include <map>
#include <wvtest.h>
#include <wvglobdiriter.h>
#include <wvfile.h>
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

WVTEST_MAIN("Non-recursive WvGlobDirIter")
{
    WvString dir("/tmp/wvtest-wvdiriter-%s", getpid());

    WvStringList entries;
    entries.split("file-one file-two .dot-file subdir/sub-file");
    
    WVPASS(create_dir(dir, entries));

    std::map<WvString, bool> found;

    WvGlobDirIter di(dir, "file-*", false);
    for (di.rewind(); di.next(); )
        found[di->relname] = true;

    WVFAIL(found.find(".") != found.end());
    WVFAIL(found.find("..") != found.end());
    WVPASS(found.find("file-one") != found.end());
    WVPASS(found.find("file-two") != found.end());
    WVFAIL(found.find(".dot-file") != found.end());
    WVFAIL(found.find("subdir") != found.end());
    WVFAIL(found.find("subdir/.") != found.end());
    WVFAIL(found.find("subdir/..") != found.end());
    WVFAIL(found.find("subdir/sub-file") != found.end());
    
    WVPASS(destroy_dir(dir));
}

WVTEST_MAIN("Recursive WvGlobDirIter")
{
    WvString dir("/tmp/wvtest-wvdiriter-%s", getpid());

    WvStringList entries;
    entries.split("file-one file-two .dot-file subdir/sub-file");
    
    WVPASS(create_dir(dir, entries));

    std::map<WvString, bool> found;

    WvGlobDirIter di(dir, "subdir/*-file", true);
    for (di.rewind(); di.next(); )
        found[di->relname] = true;

    WVFAIL(found.find(".") != found.end());
    WVFAIL(found.find("..") != found.end());
    WVFAIL(found.find("file-one") != found.end());
    WVFAIL(found.find("file-two") != found.end());
    WVFAIL(found.find(".dot-file") != found.end());
    WVFAIL(found.find("subdir") != found.end());
    WVFAIL(found.find("subdir/.") != found.end());
    WVFAIL(found.find("subdir/..") != found.end());
    WVPASS(found.find("subdir/sub-file") != found.end());
    
    WVPASS(destroy_dir(dir));
}

