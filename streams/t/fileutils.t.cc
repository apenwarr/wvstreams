/* FIXME: horribly incomplete */
#include <sys/types.h>
#include <unistd.h>

#include "wvtest.h"
#include "wvfile.h"
#include "wvfileutils.h"

WVTEST_MAIN("wvchmod test")
{
    mode_t oldmode = 0600;
    mode_t newmode = 0744;

    WvString testdir("/tmp/wvchmod-test-%s", getpid());
    mkdir(testdir, 0775);

    WvString testfile("%s/file", testdir);
    WvFile f(testfile, O_CREAT | O_RDWR, oldmode);
    WVPASS(f.isok());
    f.close();

    // check that the file's initial perms are correct
    struct stat st;
    WVPASS(stat(testfile, &st) != -1);
    WVPASS((st.st_mode & 0777) == oldmode);

    // ensure that we can create the symlink
    WvString testlink("%s/link", testdir);
    WVPASS(symlink(testfile, testlink) != -1);

    // test that chmodding a symlink does not touch the file
    WVPASS(wvchmod(testlink, newmode) == -1);
    WVPASS(stat(testfile, &st) != -1);
    WVPASS((st.st_mode & 0777) == oldmode);

    // test that chmodding the file works
    WVPASS(wvchmod(testfile, newmode) != -1);
    WVPASS(stat(testfile, &st) != -1);
    WVPASS((st.st_mode & 0777) == newmode);

    unlink(testlink);
    unlink(testfile);
    rmdir(testdir);
}
