/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvFileWatcher test.  Opens a file, /tmp/test.txt, and displays new
 * additions to this file as they occur.
 */
#include "wvlog.h"
#include "wvwatcher.h"

int main()
{
    WvLog log("watchtest", WvLog::Info);
    const WvString fname("/tmp/test.txt");
    WvFileWatcher f(fname, O_RDONLY | O_CREAT | O_TRUNC);
    char buf[1024];
    size_t len;
    
    log(WvLog::Notice, "Watching %s:\n", fname);
    
    while (f.isok())
    {
	if (f.select(100))
	{
	    len = f.read(buf, 1024);
	    log.write(buf, len);
	}
    }
    
    if (f.geterr())
	log("%s: %s\n", fname, strerror(f.geterr()));
    return 0;
}
