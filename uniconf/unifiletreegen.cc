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
    basedir(_basedir), log(_basedir, WvLog::Info), moniker(_moniker)
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
            log(WvLog::Notice, WvString("%s doesn't exists!\n", unikey));
            WvString mountstring("%s:%s",moniker,filename);
            log(WvLog::Notice, WvString("Mounting with:  %s", mountstring));
            mount(unikey, mountstring, true);
        }
    }

//    UniMountTreeGen::refresh();
    return true;
}


