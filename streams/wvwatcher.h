/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997, 1998 Worldvisions Computer Technology, Inc.
 * 
 * The WvFileWatcher class provides support for files which sometimes
 * have data appended at the end.  The WvStream::select() function is
 * overloaded to provide support for this as best we can (ie., polling
 * every 100ms or so).
 * 
 * The file is rewound and reopened if its inode changes or its
 * length gets shorter, under the assumption that we will want to see the
 * entire contents of the new file.
 * 
 * IMPORTANT!! NOTE:
 * 
 * WvStreamList::select() cannot be used with this class, since the real
 * Unix select() call will not work properly with files.
 */
#ifndef __WVWATCHER_H
#define __WVWATCHER_H

#include "wvstream.h"
#include <sys/stat.h>

struct stat;

class WvFileWatcher : public WvFile
{
    WvString filename;
    int openmode;
    bool once_ok;
    struct stat last_st;
    off_t fpos;

protected:
    bool make_ok(bool retry);
    
public:
    WvFileWatcher(const char *_filename, int _mode);
    virtual bool isok() const;
    virtual size_t uread(void *buf, size_t size);
    virtual size_t uwrite(const void *buf, size_t size);
    virtual bool select(time_t msec_timeout,
			bool readable = true, bool writable = false,
			bool isexception = false);
};

#endif // __WVWATCHER_H
