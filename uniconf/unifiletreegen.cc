/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A generator for .ini files.
 */
#include "wvtclstring.h"
#include "strutils.h"
#include "wvfile.h"
#include "wvmoniker.h"
#include "wvdiriter.h"
#include "unifiletreegen.h"


/***** Overridden members *****/

void UniFileTreeGen::map_directory()
{
    // Create a directory iterator with the specified recursion.
    WvDirIter i(directory, recursive);

    // Make sure our iterator is ok.  I.e. the directory exists, etc.
    if (!i.isok())
    {
        log(WvLog::Error, WvString("Error:  Directory %s did not exist!\n", directory));
    }

    // Go through all of the files now and load them.
    for (i.rewind(); i.next();)
    {
        log(WvLog::Error, WvString("Looking at:  %s!\n", i().name));
    }
}

