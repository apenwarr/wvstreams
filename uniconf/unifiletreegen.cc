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

UniConfFileTreeGen::UniConfFileTreeGen(WvStringParm _basedir, WvStringParm _moniker) :
    basedir(_basedir), log(_basedir, WvLog::Info)//,root(NULL, UniConfKey::EMPTY, "")
{
    log(WvLog::Notice,
	"Creating a new FileTree based on '%s'.\n", basedir);
}

bool UniConfFileTreeGen::refresh()
{
    // Create an iterator to go through the basedirectory and
    // it's sub directories, to add all non-included files into the
    // UniConf repository.
    WvDirIter i(basedir, true);


    for (i.rewind(); i.next();)
    {
        WvString filename = i->fullname;
        WvString unikey = i->relname;

        log(WvLog::Debug1, WvString("Checking if %s exists already.\n", unikey));

        if (!exists(unikey))
        {
            log(WvLog::Debug2, WvString("%s doesn't exists!\n", unikey));
            mount(unikey, WvString("%s:%s", moniker, filename), true);
        }
/*        else
        {
            whichmount(UniConfKey(unikey), new UniConfKey(unikey))->refresh();
        }*/
    }

    UniMountTreeGen::refresh();
/*    hold_delta();
    bool result = true;

    UniConfGenList::Iter i(mounts->generators);
    for (i.rewind(); i.next();)
        result = result && i->refresh();

    unhold_delta();
    return result;*/
/*    WvDirIter dirit(basedir, true);
    WvString filename("%s%s", basedir, key);
    
    struct stat statbuf;
    bool exists = (0 == lstat(filename.cstr(), &statbuf));

    UniConfValueTree *node = root.find(key);
    if (!exists)
    {
        if (node != &root)
            delete node;
        return true;
    }
    node = maketree(key);
    node->zap();
    
    WvDirIter dirit(filename, true);
    UniConfKey dirkey(filename);
    for (dirit.rewind(); dirit.next(); )
    {
	log(WvLog::Debug2, ".");
        UniConfKey filekey(dirit->fullname);
        filekey = filekey.removefirst(dirkey.numsegments());
        maketree(filekey);
    }*/
    return true;
}

/*
WvString UniConfFileTreeGen::get(const UniConfKey &key)
{
    // check the cache
    UniConfValueTree *node = root.find(key);
    if (node && !node->value().isnull())
        return node->value();

    // read the file and extract the first non-black line
    WvString filename("%s%s", basedir, key);
    WvFile file(filename, O_RDONLY);
    
    char *line;
    for (;;)
    {
        line = NULL;
        if (!file.isok())
            break;
	line = file.getline(-1);
	if (!line)
            break;
	line = trim_string(line);
	if (line[0])
            break;
    }

    if (file.geterr())
    {
	log("Error reading %s: %s\n", filename, file.errstr());
        line = "";
    }

    if (!node)
        node = maketree(key);
    WvString value(line);
    value.unique();
    node->setvalue(value);
    file.close();
    return value;
}


bool UniConfFileTreeGen::exists(const UniConfKey &key)
{
    UniConfValueTree *node = root.find(key);
    if (!node)
    {
        refresh();
        node = root.find(key);
    }
    return node != NULL;
}


bool UniConfFileTreeGen::haschildren(const UniConfKey &key)
{
    UniConfValueTree *node = root.find(key);
    if (!node || !node->haschildren())
    {
        refresh(key, UniConfDepth::CHILDREN);
        node = root.find(key);
    }
    return node != NULL && node->haschildren();
}


UniConfFileTreeGen::Iter *UniConfFileTreeGen::iterator(const UniConfKey &key)
{
    if (haschildren(key))
    {
        UniConfValueTree *node = root.find(key);
        if (node)
            return new NodeIter(*node);
    }
    return new NullIter();
}


UniConfValueTree *UniConfFileTreeGen::maketree(const UniConfKey &key)
{
    // construct a node for the file with a null value
    UniConfValueTree *node = &root;
    UniConfKey::Iter it(key);
    it.rewind();
    while (it.next())
    {
        UniConfValueTree *prev = node;
        node = node->findchild(it());
        if (!node)
            node = new UniConfValueTree(prev, it(), WvString::null);
    }
    return node;
}

*/
