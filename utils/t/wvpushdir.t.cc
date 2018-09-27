#include "wvtest.h"
#include "wvpushdir.h"
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

WVTEST_MAIN("pushdir exists")
{
    WvString dir("/tmp/wvpushdir-%s", getpid());
    mkdir(dir, 0775);

    WvPushDir newpushdir(dir);

    WVPASS(newpushdir.isok());

    char pwd[1024] = "";
    getcwd(pwd, sizeof(pwd));
    WVPASSEQ(pwd, dir);

    unlink(dir);
}

WVTEST_MAIN("pushdir does NOT exist")
{
    WvString dir("/tmp/wvpushdir-%s", getpid() + 32767);

    WvPushDir newpushdir(dir);

    WVFAIL(newpushdir.isok());
}

WVTEST_MAIN("pushdir is a file")
{
    WvString dir("/tmp/wvpushdir-%s", getpid());
    mkdir(dir, 0775);
    system(WvString("touch %s/tmpfile", dir)); 

    WvPushDir newpushdir(WvString("%s/tmpfile", dir));

    WVFAIL(newpushdir.isok());

    unlink(WvString("%s/tmpfile", dir));
    unlink(dir);
}

WVTEST_MAIN("rmdir calls fail")
{
    WvString dir("/tmp/wvpushdir-%s", getpid());
    mkdir(dir, 0775);

    WvPushDir newpushdir(dir);
    WVFAIL(rmdir(dir));

    WVPASS(newpushdir.isok());
}

#ifndef _WIN32
WVTEST_MAIN("pushdir is allocated on the STACK only")
{
    pid_t child = fork(); 

    if (child == 0)
    {
	(void)new WvPushDir("anyfile");

        // should never be reached, if it does, it should return good so
        //  that we'll fail when we check for a fail
        _exit(0); 
    }
    else if (child < 0)
    {
       WVFAIL("Fork failed"); 
       return;
    }

    int status;
    waitpid(child, &status, 0);
    WVPASSEQ(WTERMSIG(status), 6); // sig_abrt
}
#endif
