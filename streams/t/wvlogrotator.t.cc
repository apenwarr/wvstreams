#include "wvdiriter.h"
#include "wvfile.h"
#include "wvlogrotator.h"
#include "wvtest.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>

#define WORKDIR "/tmp"
#define NUM_MINS_IN_DAY (24*60)
#define NUM_SECS_IN_DAY (60*NUM_MINS_IN_DAY)

void touch_file(WvStringParm file)
{
    WvFile f(file, O_CREAT | O_TRUNC);
}

class WvLogRotatorTest
{
public:
    // useful when debugging rare failures to do with rotator forking
    bool nonefailed;
    WvLogRotator *rotator;
    WvString workdir;
    WvStringList lognames;

    WvLogRotatorTest(WvStringParm _lognames, int keep_for)
        : nonefailed(true)
    {
        lognames.split(_lognames, " ");
        WvStringList::Iter j(lognames);
        j.rewind(); j.next();
        
        // find a directory not in use, try using the first log name given
        workdir = WvString("%s/%sXXXXXX", WORKDIR, j());
        
        int fd;
        while ((fd = mkstemp(workdir.edit())) == (-1));
        close(fd);
        unlink(workdir);
        system(WvString("mkdir %s", workdir));
        // fprintf(stderr, "using dir %s\n", workdir.cstr());
        
        WvStringList::Iter i(lognames);
        for (i.rewind(); i.next(); )
        {
            // label our logname as being inside our new directory
            i() = WvString("%s/%s", workdir, i());
            touch_file(i());
        }

        rotator = new WvLogRotator(lognames.join(" "), keep_for);
    }
    
    ~WvLogRotatorTest()
    {
        WVRELEASE(rotator);
        WvDirIter i(workdir);
        for (i.rewind(); i.next(); )
        {
            unlink(i->fullname);
        }
        rmdir(workdir);
        if (!nonefailed)
            sleep(10);
    }

    void execute()
    {
        rotator->execute();
    }
    
    int get_keep_for()
    {
        return rotator->keep_for;
    }
    
    WvString create_log(int days_old = 0)
    {
        WvStringList filenames;
        WvStringList::Iter i(lognames);
        for (i.rewind(); i.next(); )
        {
            time_t timenow = wvtime().tv_sec;
            time_t timethen = timenow -= NUM_SECS_IN_DAY * days_old;
            struct tm *tmstamp = localtime(&timenow);
            char buf[20];
            strftime(buf, 20, "%Y-%m-%d", tmstamp);
            WvString filename = WvString("%s.%s", i(), buf);
            touch_file(filename);
            struct utimbuf tbuf;
            tbuf.actime = timenow;
            tbuf.modtime = timethen;
            utime(filename, &tbuf);
            filenames.append(filename);
        }
    
        return filenames.join(" ");
    }
};

bool files_exist(WvStringList &list)
{
    struct stat statbuf;
    WvStringList::Iter i(list);
    for (i.rewind(); i.next(); )
    {
        if (stat(i(), &statbuf) == -1)
            return false;
    }
    return true;
}

void run_test(WvStringParm lognames)
{
    // keep logs for 1 day
    WvLogRotatorTest test(lognames, 2);

    //existing logname files should be moved
    test.execute();
    WVFAIL(files_exist(test.lognames));
    
    // create a few logs that are too old and one that's not
    WvStringList result, result1, result2, result3;
    result.split(test.create_log(1));
    result1.split(test.create_log(3));
    result2.split(test.create_log(4));
    result3.split(test.create_log(5));
    test.execute();
    // wait for rotator's delete fork to finish up
    sleep(1);
    test.nonefailed = WVPASS(files_exist(result));
    test.nonefailed = !WVFAIL(files_exist(result1));
    test.nonefailed = !(files_exist(result2));
    test.nonefailed = !(files_exist(result3));
}

WVTEST_MAIN("basics")
{
    WvLogRotatorTest test("log", 2);
    test.rotator->set_keep_for(5);
    WVPASS(test.get_keep_for() == 5);
    test.rotator->set_keep_for(0);
    WVPASS(test.get_keep_for() == 0);
    test.rotator->set_keep_for(-50);
    WVPASS(test.get_keep_for() == 0);
}

WVTEST_MAIN("old logs get removed, good ones don't")
{
    run_test("rotatorlog");
}

WVTEST_MAIN("lists of files to take care of")
{
    run_test("rlog1 rlog2 rlog3 rlog4");
}
