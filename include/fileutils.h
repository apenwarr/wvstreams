/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Various little file functions...
 *
 */

#ifndef __FILEUTILS_H
#define __FILEUTILS_H

#include "wvstring.h"
#include "wvstringlist.h"


/**
 * Create a directory and any subdirectories required along the way. (Equivalent
 * to mkdir -p).
 *
 * The default permissions on created directories is 0700, but this can be
 * changed at will.
 */
bool mkdirp(WvStringParm _dir, int create_mode = 0700);


/**
 * Copy from src to dst preserving permissions and time stamp. This does not
 * preserve ownership, however.
 *
 * Two versions of this are provided. One for giving two filenames/paths, and
 * another for giving two starting directories and a relative path from there.
 */
bool fcopy(WvStringParm src, WvStringParm dst);
bool fcopy(WvStringParm srcdir, WvStringParm dstdir, WvStringParm relname);


/**
 * Check whether two files have the same date/time stamp. This can be used as a
 * quick check whether files are unchanged / the same, though obviously it
 * doesn't verify that they are indeed the same file.
 *
 * Two versions are provided, one for giving two files, and another for giving
 * two starting directories and a relative path from there.
 */
bool samedate(WvStringParm file1, WvStringParm file2);
bool samedate(WvStringParm dir1, WvStringParm dir2, WvStringParm relname);

/**
 * Runs fnmatch against everything in the patterns list.  We also interpret
 * .cvsignore-style '!' patterns, which makes us very fancy.
 */
bool wvfnmatch(WvStringList& patterns, WvStringParm name, int flags = 0);

#endif // __FILEUTILS_H
