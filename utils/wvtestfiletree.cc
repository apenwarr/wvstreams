/* Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * Manages a tree of test files.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include "strutils.h"
#include "wvacl.h"
#include "wvbuf.h"
#include "wvdigest.h"
#include "wvdiriter.h"
#include "wvfile.h"
#include "wvtestfiletree.h"

WvTestFileTree::WvTestFileTree(UniConf _files, WvStringList &_dirs,
			       WvStringParm _basedir)
    : log("TestFileTree", WvLog::Debug2), files(_files), dirs(_dirs),
      basedir(_basedir), num_dirs(0), num_files(0)
{
}


bool WvTestFileTree::create_test_files(unsigned int _num_dirs,
				       unsigned int _num_files,
				       off_t _size_of_file,
				       unsigned int _nesting)
{
    num_dirs = _num_dirs;
    num_files = _num_files;
    size_of_file = _size_of_file;
    nesting = _nesting;

    bool created_files = true;
    printf("creating %d levels of %d dirs with %d files of size %llu each.\n",
	   nesting, num_dirs, num_files, size_of_file);
    dirs.zap();
    for (size_t i = 0; i < num_dirs; i++)
    {
      	WvString dirname("%s/dir%s", basedir, i);
	mkdir(dirname, 0777);
	dirs.append(new WvString(dirname), true);
	if (!record_fileinfo(dirname))
	{
	    created_files = false;
	    log(WvLog::Error, "Couldn't retrieve file info for %s.\n",
		dirname);
	}
	if (!fill_in_dir(dirname, i, 0))
	{
	    created_files = false;
	    log(WvLog::Error, "Couldn't create files in %s.\n", dirname);
	}
    }

    files.commit();

    return created_files;
}


bool WvTestFileTree::create_random_file(WvStringParm filename, off_t size)
{
    WvDynBuf buf;
    UniConf info(files[filename]);
    WvFile out(filename, O_WRONLY | O_CREAT | O_TRUNC);
    WvFile in("/dev/urandom", O_RDONLY);
    if (!out.isok() || !in.isok())
	return false;

    WvMD5Digest md5;
    WvDynBuf md5sum;
    off_t res, num_to_read, count = size;
    while (count > 0)
    {
	num_to_read = count > 1024 ? 1024 : count;
	res = in.read(buf, num_to_read);
	if (res)
	{
	    out.write(buf, res);
	    count -= res;
	    buf.unget(res);
	    md5.encode(buf, md5sum);
	}
    }
    md5.finish(md5sum);

    char md5string[md5.digestsize() * 2 + 1];
    hexify(md5string, md5sum.get(md5.digestsize()), md5.digestsize());

    info["wvstats/md5sum"].set(md5string);

    return (record_fileinfo(filename, false));
}


bool WvTestFileTree::create_symlink(WvStringParm oldname, WvStringParm newname)
{
    if (symlink(oldname, newname) != 0)
	return false;

    return (record_fileinfo(newname, false));
}


void WvTestFileTree::scan_test_files()
{
    printf("scanning directory %s.\n", basedir.cstr());
    dirs.zap();
    WvDirIter di(basedir, false);
    for (di.rewind(); di.next(); )
	dirs.append(new WvString(di().fullname), true);

    WvDirIter dir(basedir);
    for (dir.rewind(); dir.next(); )
	record_fileinfo(dir().fullname);
}


bool WvTestFileTree::record_fileinfo(WvStringParm filename, bool set_md5)
{
//    printf("recording fileinfo for %s\n", filename.cstr());
    struct stat st;
    if (lstat(filename, &st) != 0)
	return false;
    UniConf info(files[filename]);
    info["wvstats/mode"].setint(st.st_mode);
    info["wvstats/uid"].setint(st.st_uid);
    info["wvstats/gid"].setint(st.st_gid);
    info["wvstats/rdev"].setint(st.st_rdev);
    info["wvstats/mtime"].setint(st.st_mtime);
    info["wvstats/ctime"].setint(st.st_ctime);
    info["wvstats/acl"].set(get_acl_short_form(filename));

    if (S_ISDIR(st.st_mode))
	info["wvstats/size"].setint(0);
    else if (S_ISLNK(st.st_mode))
    {
	info["wvstats/size"].setint(st.st_size);
	char *linkto = new char[st.st_size+1];
	linkto[st.st_size] = 0;
	if (readlink(filename, linkto, st.st_size))
	{
//	    printf("symlink to %s\n", linkto);
	    info["wvstats/linkto"].set(linkto);
	    delete[] linkto;
	}
    }
    else if (S_ISREG(st.st_mode))
    {
	info["wvstats/size"].setint(st.st_size);
	if (set_md5)
	    info["wvstats/md5sum"].set(get_md5_str(filename));
    }
    else
    {
	printf("skipping weird file %s.\n", filename.cstr());
	return false;
    }
    
    return true;
}


WvString WvTestFileTree::get_md5_str(WvStringParm filename)
{
    WvFile f(filename, O_RDONLY);
    if (!f.isok())
	return WvString();

    WvMD5Digest md5;
    WvDynBuf buf, mymd5sum;
    size_t res;
    while (f.isok())
    {
	res = f.read(buf, 1024);
	if (res)
	    md5.encode(buf, mymd5sum);
    }
    md5.finish(mymd5sum);

    char md5string[md5.digestsize() * 2 + 1];
    hexify(md5string, mymd5sum.get(md5.digestsize()), md5.digestsize());

    return md5string;
}


bool WvTestFileTree::fill_in_dir(WvStringParm dirname, WvStringParm dirnum,
				 unsigned int curnesting)
{
    bool created_files = true;
    int last_file = 0;
    for (unsigned int j = 0; j < num_files; j++)
    {
	// create files and symlinks
	if (j % 3 == 1)
	{
	    if (!create_symlink(WvString("%s/file%s.%s", dirname, dirnum,
					 last_file),
				WvString("%s/file%s.%s", dirname, dirnum, j)))
		created_files = false;
	}
	else
	{
	    if (!create_random_file(WvString("%s/file%s.%s", dirname, dirnum,
					     j), size_of_file))
		created_files = false;
	    else
		last_file = j;
	}
    }

    if (curnesting < nesting - 1)
    {
	curnesting++;
	for (unsigned int i = 0; i < num_dirs; i++)
	{
	    WvString newdirnum("%s.%s", dirnum, i);
	    WvString newdirname("%s/dir%s", dirname, newdirnum);
	    mkdir(newdirname, 0777);
	    if (!record_fileinfo(newdirname))
	    {
		created_files = false;
		log(WvLog::Error, "Couldn't retrieve file info for %s.\n",
		    dirname);
	    }		
	    
	    if (!fill_in_dir(newdirname, newdirnum, curnesting))
	    {
		created_files = false;
		log(WvLog::Error, "Couldn't create files in %s.\n", dirname);
	    }
	}
    }
    
    return created_files;
}
