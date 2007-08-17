#include "wvtest.h"
#include "wvfile.h"
#include "wvfileutils.h"

WVTEST_MAIN("basics")
{
    WvString filename = wvtmpfilename("wvfile-test");
    
    {
	WvFile f(filename, O_WRONLY | O_CREAT);
	WVPASS(f.isok());
	if (!f.isok())
	    printf("file error code: '%s'\n", f.errstr().cstr());
	WVFAIL(f.isreadable());
	WVPASS(f.iswritable());
	
	f.print("line1\n");
	f.print("line2");
    }
    
    {
	WvFile f(filename, O_RDONLY);
	WVPASS(f.isok());
	
	WVPASS(f.isreadable());
	WVFAIL(f.iswritable());
	
	char buf[1024];
	size_t len = f.read(buf, sizeof(buf)-1);
	WVPASSEQ(len, 11);
	buf[11] = 0;
	WVPASSEQ(buf, "line1\nline2");
    }
    
    {
	WvFile f(filename, O_RDONLY);
	WVPASS(f.isok());
	WVPASSEQ(f.getline(), "line1");
	WVPASSEQ(f.getline(), "line2");
	WVPASSEQ(f.getline(), NULL);
    }
    
    unlink(filename);
}


WVTEST_MAIN("no crlf mangling")
{
    WvString filename = wvtmpfilename("wvfile-test");
    WvString s("a\rb\nc\r\nd\n\r");
    
    {
	FILE *f = fopen(filename, "wb");
	if (f)
	{
	    fwrite(s, strlen(s), 1, f);
	    fclose(f);
	}
    }
    
    {
	WvFile f(filename, O_RDONLY);
	char buf[1024];
	size_t len = f.read(buf, sizeof(buf)-1);
	buf[len] = 0;
	WVPASSEQ(strlen(buf), strlen(s));
	WVPASSEQ(buf, s);
    }
    
//    unlink(filename);
}
