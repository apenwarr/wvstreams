#include "unifilesystemgen.h"
#include "uniconfkey.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

// Don't make this too big; we put the whole thing on the stack.
#define BYTES_PER_READ 2048

UniFileSystemGen::UniFileSystemGen(WvStringParm _dir, WvStringParm _file,
				   mode_t _mode)
    : dir(_dir), file(_file), mode(_mode)
{
    assert(0 && "The UniFileSystemGen is unfinished; please do not use it.");
}

WvString UniFileSystemGen::get(const UniConfKey &key)
{
    WvString path(dir);
    UniConfKey mykey(UniConfKey(file.cstr()), key);
    while (!mykey.isempty())
    {
	WvString seg(mykey.pop().printable());
	if (seg == "." || seg == "..")
	    return WvString();

	path.append("/%s", seg);
    }

    struct stat buf;
    if (stat(path.cstr(), &buf) == -1)
	// Fail.
	return WvString();

    if (S_ISREG(buf.st_mode))
    {
	// We actually just keep reading until end of file instead of, say,
	// reading an amount of bytes equal to the length of the file, because
	// the UniFileSystemGen was created to expose some /proc/sys files to
	// UniConf, which are magical and claim to have length of zero.
	char tmpbuf[BYTES_PER_READ+1];
	WvString ret;
	int fd = open(path.cstr(), O_RDONLY);	
	while (1)
	// Yes, really read the entire contents of a file into a WvString.
	{
            // Read the file for a bit.
	    int res = ::read(fd, tmpbuf, BYTES_PER_READ);
	    
	    // Break on end of file or error
            if (res <= 0)
		break;
	    
	    // Put everything we read into our string
	    tmpbuf[res] = '\0';
	    ret.append(tmpbuf);
	}
	close(fd);
	return ret;
    }

    return WvString::empty;
}

void UniFileSystemGen::set(const UniConfKey &key, WvStringParm value)
{
    WvString path(dir);
    UniConfKey mykey(UniConfKey(file.cstr()), key);
    while (mykey.numsegments() > 1)
    {
	WvString seg(mykey.pop().printable());
	if (seg == "." || seg == "..")
	    return;

	path.append("/%s", seg);
	struct stat buf;
	if (stat(path.cstr(), &buf) == -1)
	{
	    if (errno != ENOENT || value.isnull())
		// Fail or finish early, as appropriate.
		return;
	    else
		goto do_mkdir;
	}
	else if (S_ISREG(buf.st_mode))
	{
	    if (buf.st_size != 0 || value.isnull())
		// Fail or finish early, as appropriate.
		return;
	    else
		goto do_unlink;
	}
	else if (!S_ISDIR(buf.st_mode))
	    goto do_unlink;
	else
	    continue;

    do_unlink:
	if (unlink(path.cstr()) == -1)
	    // Fail.
	    return;

    do_mkdir:
	if (mkdir(path.cstr(), mode) == -1)
	    // Fail.
	    return;
    }

    WvString seg(mykey.printable());
    if (seg == "." || seg == "..")
        return;

    path.append("/%s", seg);
    struct stat buf;
    if (stat(path.cstr(), &buf) == -1)
    {
	if (errno != ENOENT)
            // Fail.
	    return;
    }
    else if (value.isnull())
    {
	if (S_ISDIR(buf.st_mode))
	{
	    system(WvString("rm -Rf %s", path).cstr());
	    return;
	}
	else
	{
	    unlink(path.cstr());
	    return;
	}
    }
    else if (!value && !S_ISREG(buf.st_mode))
    {
	return;
    }
    else
    {
	if (S_ISDIR(buf.st_mode))
	{
	    if (rmdir(path.cstr()) == -1) return;
        }
	else if (!S_ISREG(buf.st_mode))
	{
	    if (unlink(path.cstr()) == -1) return;
	}
    }

    // Write.
    int remaining = value.len();
    const char *where = value.cstr();
    int fd = creat(path.cstr(), mode);
    for (int res = 0; remaining > 0; where += res, remaining -= res)
    {
	res = ::write(fd, where, remaining);
	
	// Break on end of file or error
	if (res <= 0)
	    break;
    }
    close(fd);
    
    return;
}

class UniFileSystemGenIter : public UniConfGen::Iter
{
public:
    UniFileSystemGenIter(UniFileSystemGen *_that, DIR *_dir,
			 const UniConfKey &_rel)
	: that(_that), dir(_dir), rel(_rel)
    {
    }

    ~UniFileSystemGenIter()
    {
	closedir(dir);
    }

    void rewind()
    {
	rewinddir(dir);
    }

    bool next()
    {
	struct dirent *entry = readdir(dir);
	if (entry)
	{
	    WvString entryname(entry->d_name);
	    if (entryname == "." || entryname == "..")
		return next();
	    else
	    {
                name = UniConfKey(entryname);
	        return true;
	    }
	}
	else
	    return false;
    }

    UniConfKey key() const
    {
	return UniConfKey(rel, name);
    }

    WvString value() const
    {
	return that->get(key());
    }

private:
    UniFileSystemGen *that;
    DIR *dir;
    UniConfKey rel;
    UniConfKey name;
};

UniConfGen::Iter *UniFileSystemGen::iterator(const UniConfKey &key)
{
    WvString path(dir);
    UniConfKey mykey(UniConfKey(file.cstr()), key);
    while (!mykey.isempty())
    {
	WvString seg(mykey.pop().printable());
	if (seg == "." || seg == "..")
	    return NULL;

	path.append("/%s", seg);
    }

    DIR *dh = opendir(path.cstr());
    if (dh == NULL)
	return NULL;
    else
	return new UniFileSystemGenIter(this, dh, key);
}
