/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Globbing directory iterator.
 *
 */

#include "wvglobdiriter.h"
#include "wvglob.h"

WvGlobDirIter::WvGlobDirIter( WvStringParm dirname, WvStringParm glob_str,
		      bool _recurse, bool _skip_mounts, size_t sizeof_stat )
    : WvDirIter(dirname, _recurse, _skip_mounts, sizeof_stat), glob(NULL)
{
    if (!glob_str.isnull())
    {
        glob = new WvGlob(glob_str);
        if (!glob->isok())
        {
            delete glob;
            glob = NULL;
        }
    }
}

WvGlobDirIter::~WvGlobDirIter()
{
    if (glob) delete glob;
}

bool WvGlobDirIter::next()
{
    bool result;

    do
    {
        result = WvDirIter::next();
    } while (result && glob && !glob->match(ptr()->relname));

    return result;
}
