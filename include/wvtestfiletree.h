/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * Manages a tree of test files.
 */
#ifndef __WVTESTFILETREE_H
#define __WVTESTFILETREE_H

#include "uniconf.h"
#include "wvlog.h"
#include "wvstringlist.h"

/** Creates or scans a tree of test files and keeps relevant info.
 * WvTestFileTree can be used to create a tree of abitrary size containing
 * random files and symlinks.  It maintains information (stat info and md5 sums)
 * in a UniConf object.  It can also scan an existing tree and record the info.
 */
class WvTestFileTree
{
public:
    WvTestFileTree(UniConf _files, WvStringList &_dirs, WvStringParm _basedir);
    ~WvTestFileTree() {}

    /** Create a set of directories, random-content files, and symlinks.
     * Creates a tree '_nesting' levels deep, with '_num_dirs' directories
     * at each level, with '_num_files' files of size '_size_of_file' bytes in
     * each directory.
     */
    bool create_test_files(unsigned int _num_dirs, unsigned int _num_files,
			   off_t _size_of_file, unsigned int _nesting);

    /** Creates a random file.
     * This is used by create_test_files() but can also be called on its own.
     * @return True if file was created successfully
     */
    bool create_random_file(WvStringParm filename, off_t size);

    /** Creates a symlink.
     * Used by create_test_files() but can be called on its own.  Essentially
     * a wrapper for ::symlink() that records info into 'files'.
     * @return True if symlink was created successfully
     */
    bool create_symlink(WvStringParm oldname, WvStringParm newname);

    /** Scans the file tree at 'basedir' and records info.
     */
    void scan_test_files();

    /** Records info of 'filename'.
     * Generally not needed externally (used by scan_test_files()).
     */
    bool record_fileinfo(WvStringParm filename, bool set_md5 = true);

    /** Calculates the MD5 sum of 'filename'.
     * Note that create_random_file doesn't use this, as it's more efficient
     * to calculate the MD5 sum as we create the file.
     */
    WvString get_md5_str(WvStringParm filename);

private:
    WvLog log;
    UniConf files;
    WvStringList &dirs;
    WvString basedir;

    /// The following 4 vars are used by create_test_files() only.
    unsigned int num_dirs;
    unsigned int num_files;
    off_t size_of_file;
    unsigned int nesting;

    bool fill_in_dir(WvStringParm dirname, WvStringParm dirnum,
		     unsigned int curnesting);
};

#endif // __WVTESTFILETREE_H
 

