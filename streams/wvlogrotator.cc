#include "wvlogrotator.h"

#include "wvdiriter.h"
#include "wvfork.h"
#include "wvstrutils.h"
#include "wvtimeutils.h"
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_MINS_IN_DAY (24*60)
#define NUM_SECS_IN_DAY (60*NUM_MINS_IN_DAY)

WvLogRotator::WvLogRotator(WvStringParm _filenames, int _keep_for)
    : WvDailyEvent(1), keep_for(_keep_for)
{
    filenames.split(_filenames, " ");
}

void WvLogRotator::execute()
{
    WvDailyEvent::execute();
    
    WvStringList::Iter i(filenames);
    for (i.rewind(); i.next(); )
    {
        // move the current log
        WvString filename = i();
        time_t timenow = wvtime().tv_sec;
        struct tm *tmstamp = localtime(&timenow);
        char buf[20];
        strftime(buf, 20, "%Y-%m-%d", tmstamp);
    
        int num = 0;
        struct stat statbuf;
        // Get the next filename
        WvString fullname("%s.%s", filename, buf);
        while (stat(fullname, &statbuf) != -1)
            fullname = WvString("%s.%s.%s", filename, buf, num++);

        rename(filename, fullname);
            
        // remove expired logs
        WvString base = getfilename(filename);
        pid_t forky = wvfork();
        if (!forky)
        {
            // In child
            if (!wvfork())
            {
                // Child will look for old logs and purge them
                WvDirIter i(getdirname(filename), false);
                i.rewind();
                while (i.next() && keep_for)
                {
                    // if it begins with the base name
                    if (!strncmp(i.ptr()->name, base, strlen(base)))
                        // and it's older than 'keep_for' days
                        if (i.ptr()->st_mtime < 
                                wvtime().tv_sec - keep_for*NUM_SECS_IN_DAY)
                        {
                            // delete it
                            unlink(i.ptr()->fullname);
                        }
                }
                _exit(0);
            }
            _exit(0);
        }
        int status;
        waitpid(forky, &status, 0);
    }
}
