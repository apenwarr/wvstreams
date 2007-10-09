#include <map>
#include <wvtest.h>
#include <wvdiriter.h>
#include <wvfile.h>
#include <wvstring.h>
#include <wvfileutils.h>
#ifdef _WIN32
#include <direct.h>
#endif

static bool create_dir(WvStringParm dir, const WvStringList &entries)
{
    ::unlink(dir);
    if (wvmkdir(dir, 0700)) return false;

    WvStringList::Iter entry(entries);
    for (entry.rewind(); entry.next(); )
    {
        WvString name("%s/%s", dir, *entry);
        wvmkdir(getdirname(name), 0700);
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
    WvString dir = wvtmpfilename("wvtest-wvdiriter-");

    WvStringList entries;
    entries.split("file-one file-two .dot-file subdir/sub-file");
    
    WVPASS(create_dir(dir, entries));

    std::map<WvString, bool> found;

    WvDirIter di(dir, false);
    for (di.rewind(); di.next(); )
        found[di->relname] = true;

    WVFAIL(found.find(".") != found.end());
    WVFAIL(found.find("..") != found.end());
    WVPASS(found.find("file-one") != found.end());
    WVPASS(found.find("file-two") != found.end());
    WVPASS(found.find(".dot-file") != found.end());
    WVPASS(found.find("subdir") != found.end());
    WVFAIL(found.find("subdir/.") != found.end());
    WVFAIL(found.find("subdir/..") != found.end());
    WVFAIL(found.find("subdir/sub-file") != found.end());
    
    WVPASS(destroy_dir(dir));
}

WVTEST_MAIN("Recursive WvDirIter")
{
    WvString dir = wvtmpfilename("wvtest-wvdiriter-");

    WvStringList entries;
    entries.split("file-one file-two .dot-file subdir/sub-file");
    
    WVPASS(create_dir(dir, entries));

    std::map<WvString, bool> found;

    WvDirIter di(dir, true);
    for (di.rewind(); di.next(); )
        found[di->relname] = true;

    WVFAIL(found.find(".") != found.end());
    WVFAIL(found.find("..") != found.end());
    WVPASS(found.find("file-one") != found.end());
    WVPASS(found.find("file-two") != found.end());
    WVPASS(found.find(".dot-file") != found.end());
    WVPASS(found.find("subdir") != found.end());
    WVFAIL(found.find("subdir/.") != found.end());
    WVFAIL(found.find("subdir/..") != found.end());
    WVPASS(found.find("subdir/sub-file") != found.end());
    
    WVPASS(destroy_dir(dir));
}
