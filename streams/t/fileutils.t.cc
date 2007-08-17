/* FIXME: horribly incomplete */
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include "wvtest.h"
#include "wvfile.h"
#include "wvfileutils.h"

#ifndef _WIN32 // file permissions are pointless in win32
WVTEST_MAIN("wvchmod test")
{
    mode_t oldmode = 0600;
    mode_t newmode = 0744;
    mode_t diroldmode = 0700;
    mode_t dirnewmode = 0744;

    WvString basedir("/tmp/wvchmod-test-%s", getpid());
    WVPASS(wvmkdir(basedir, 0775) != -1);

    WvString testfile("%s/file", basedir);
    WvFile f(testfile, O_CREAT | O_RDWR, oldmode);
    WVPASS(f.isok());
    f.close();

    // check that the file's initial perms are correct
    struct stat st;
    WVPASS(stat(testfile, &st) != -1);
    WVPASSEQ((st.st_mode & 0777), oldmode);

    // ensure that we can create the symlink
    WvString testlink("%s/link", basedir);
    WVPASS(symlink(testfile, testlink) != -1);

    // test that chmodding a symlink does not touch the file
    WVPASS(wvchmod(testlink, newmode) == -1);
    WVPASS(stat(testfile, &st) != -1);
    WVPASSEQ((st.st_mode & 0777), oldmode);
    unlink(testlink);

    // test that chmodding the file works
    WVPASS(wvchmod(testfile, newmode) != -1);
    WVPASS(stat(testfile, &st) != -1);
    WVPASSEQ((st.st_mode & 0777), newmode);

    // add a test dir and make sure perms are good
    WvString testdir("%s/dir", basedir);
    WVPASS(wvmkdir(testdir, diroldmode) != -1);
    WVPASS(stat(testdir, &st) != -1);
    WVPASSEQ((st.st_mode & 0777), diroldmode);

    WvString testdlink("%s/dlink", basedir);
    WVPASS(symlink(testdir, testdlink) != -1);

    // test that chmodding a symlink does not touch the dir
    WVPASS(wvchmod(testdlink, dirnewmode) == -1);
    WVPASS(stat(testdir, &st) != -1);
    WVPASSEQ((st.st_mode & 0777), diroldmode);
    unlink(testdlink);

    // test that chmodding the dir works
    WVPASS(wvchmod(testdir, dirnewmode) != -1);
    WVPASS(stat(testdir, &st) != -1);
    WVPASSEQ((st.st_mode & 0777), dirnewmode);

    unlink(testfile);
    rmdir(testdir);
    rmdir(basedir);
}
#endif // !_WIN32
