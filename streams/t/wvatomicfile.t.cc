/* FIXME: horribly incomplete */
#include "wvtest.h"
#include "wvatomicfile.h"

#ifdef MACOS
  #include <sys/stat.h>
#endif

WVTEST_MAIN("atomic file test")
{
    struct stat st;
    WvString filename("/tmp/testfile.%s", getpid());

    unlink(filename);

    WVPASS(lstat(filename, &st) == -1);
    
    WvAtomicFile g(filename, O_WRONLY|O_CREAT|O_TRUNC, 0660);
    WVPASS(g.iswritable());
    WVPASS(g.isok());
    WVPASS(lstat(filename, &st) == -1);

    g.print("Hello there");
    WVPASS(lstat(filename, &st) == -1);
    
    g.close();
    WVPASS(lstat(filename, &st) == 0);
    
    WvFile f(filename, O_RDONLY);
    WVPASS(f.isok());
    WVPASS(f.getline(-1) == WvString("Hello there"));
    
    unlink(filename);
}

WVTEST_MAIN("atomic file chmod")
{
    WvString filename("/tmp/testfile.%s", getpid());
    struct stat st;

    mode_t old_umask = umask(022);
    unlink(filename);

    WvAtomicFile g(filename, O_WRONLY|O_CREAT|O_TRUNC, 0660);
    WVPASS(g.isok());
    WVPASS(fstat(g.getfd(), &st) == 0 && (st.st_mode & 0777) == 0640);
    WVPASS(g.chmod(0666));
    WVPASS(fstat(g.getfd(), &st) == 0 && (st.st_mode & 0777) == 0666);
    g.close();
    WVFAIL(g.chmod(0600));
    
    WVPASS(lstat(filename, &st) == 0 && (st.st_mode & 0777) == 0666);

    umask(old_umask);    
    unlink(filename);
}

WVTEST_MAIN("atomic errors")
{
    WvString filename("/this/directory/had/better/not/exist/filename.txt");
    WvAtomicFile f(filename, O_WRONLY|O_CREAT|O_TRUNC, 0666);
    WVFAIL(f.isok());
    WVPASSEQ(f.geterr(), ENOENT);
    printf("error string: '%s'\n", f.errstr().cstr());
}

#if 0
// This only works as root...
WVTEST_MAIN("atomic file chown")
{
    WvString filename("/tmp/testfile.%s", getpid());
    struct stat st;

    WvAtomicFile g(filename, O_WRONLY|O_CREAT|O_TRUNC, 0666);
    WVPASS(g.isok());
    WVPASS(fstat(g.getfd(), &st) == 0
    	    && st.st_uid == getuid()
    	    && st.st_gid == getgid());
    WVPASS(g.chown(getuid()+1, getgid()+1));
    WVPASS(fstat(g.getfd(), &st) == 0
    	    && st.st_uid == getuid()+1
    	    && st.st_gid == getgid()+1);
    g.close();
    WVFAIL(g.chmod(0600));
    
    WVPASS(lstat(filename, &st) == 0
    	    && st.st_uid == getuid()+1
    	    && st.st_gid == getgid()+1);

    unlink(filename);
}
#endif
