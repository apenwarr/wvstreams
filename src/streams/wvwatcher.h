/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */ 
#ifndef __WVWATCHER_H
#define __WVWATCHER_H

#include "wvfile.h"
#include <sys/stat.h>

struct stat;

/**
 * The WvFileWatcher class provides support for files which sometimes
 * have data appended at the end.  It only polls as often as your select()
 * delay, so be careful!
 * 
 * The file is rewound and reopened if its inode changes or its
 * length gets shorter, under the assumption that we will want to see the
 * entire contents of the new file.
 */
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
    virtual bool pre_select(SelectInfo &si);
};

#endif // __WVWATCHER_H
