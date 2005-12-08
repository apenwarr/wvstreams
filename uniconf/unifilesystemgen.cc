#include "unifilesystemgen.h"
#include "wvfile.h"
#include "wvdiriter.h"
#include "wvfileutils.h"
#include "wvmoniker.h"
#include "wvlinkerhack.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

WV_LINK(UniFileSystemGen);


static IUniConfGen *creator(WvStringParm s)
{
    return new UniFileSystemGen(s, 0777);
}

WvMoniker<IUniConfGen> UniFileSystemGenMoniker("fs", creator);


UniFileSystemGen::UniFileSystemGen(WvStringParm _dir, mode_t _mode)
    : dir(_dir), mode(_mode)
{
}


static bool key_safe(const UniConfKey &key)
{
    UniConfKey::Iter i(key);
    for (i.rewind(); i.next(); )
    {
	if (*i == "." || *i == ".." || *i == "")
	    return false; // unsafe key segments
    }
    
    // otherwise a safe filename
    return true;
}


WvString UniFileSystemGen::get(const UniConfKey &key)
{
    WvString null;
    
    if (!key_safe(key))
	return null;
    
    WvString path("%s/%s", dir, key);
    
    // WARNING: this code depends on the ability to open() a directory
    // as long as we don't read it, because we want to fstat() it after.
    WvFile file(path, O_RDONLY);
    if (!file.isok())
	return null; // unreadable; pretend it doesn't exist
    
    struct stat st;
    if (fstat(file.getrfd(), &st) < 0)
	return null; // openable but can't stat?  That's odd.

    if (S_ISREG(st.st_mode))
    {
	WvDynBuf buf;
	while (file.isok())
	    file.read(buf, 4096);
	if (file.geterr())
	    return null;
	else
	    return buf.getstr();
    }
    else
	return ""; // exists, but pretend it's an empty file
}


void UniFileSystemGen::set(const UniConfKey &key, WvStringParm value)
{
    if (!key_safe(key))
	return;
    
    WvString base("%s/%s", dir, key.removelast(1));
    WvString path("%s/%s", dir, key);
    
    mkdirp(base, mode);
    
    if (value.isnull())
	rm_rf(path);
    else
    {
	WvFile file(path, O_WRONLY|O_CREAT|O_TRUNC, mode & 0666);
	file.write(value);
    }
}


void UniFileSystemGen::setv(const UniConfPairList &pairs)
{
    setv_naive(pairs);
}


class UniFileSystemGenIter : public UniConfGen::Iter
{
private:
    UniFileSystemGen *gen;
    WvDirIter i;
    UniConfKey rel;
    
public:
    UniFileSystemGenIter(UniFileSystemGen *_gen, WvStringParm path,
			 const UniConfKey &_rel)
	: gen(_gen), i(path, false), rel(_rel)
	{ }

    ~UniFileSystemGenIter()
        { }

    void rewind()
        { i.rewind(); }

    bool next() 
        { return i.next(); }

    UniConfKey key() const
        { return i->relname; }

    WvString value() const
        { return gen->get(WvString("%s/%s", rel, i->relname)); }
};


UniConfGen::Iter *UniFileSystemGen::iterator(const UniConfKey &key)
{
    if (!key_safe(key))
	return NULL;
    
    return new UniFileSystemGenIter(this, WvString("%s/%s", dir, key), key);
}
