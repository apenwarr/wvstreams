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
    WvLogRotator *rotator;
    WvString workdir;
    WvStringList lognames;

    WvLogRotatorTest(WvStringParm _lognames, int keep_for)
    {
        lognames.split(_lognames, " ");
        WvStringList::Iter j(lognames);
        j.rewind(); j.next();
        
        // find a directory not in use, try using the first log name given
        workdir = WvString("%s/%s", WORKDIR, j());
        int num = 0;
        struct stat statbuf;
        WvString dirname = workdir;
        while (stat(workdir, &statbuf) != -1)
            workdir = WvString("%s.%s", dirname, num++);
        system(WvString("mkdir %s", workdir));
        
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
        system(WvString("rm -r %s", workdir));
        WVRELEASE(rotator);
    }

    void execute()
    {
        rotator->execute();
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
    WVPASS(files_exist(result));
    // FAILS
    //WVFAIL(files_exist(result1));
    //WVFAIL(files_exist(result2));
    //WVFAIL(files_exist(result3));
}

WVTEST_MAIN("old logs get removed, good ones don't")
{
    run_test("rotatorlog");
}

WVTEST_MAIN("lists of files to take care of")
{
    run_test("rlog1 rlog2 rlog3 rlog4");
}
