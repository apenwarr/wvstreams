/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A generator for .ini files.
 */
#ifndef __UNICONFFILETREEGEN_H
#define __UNICONFFILETREEGEN_H

#include "uniconfgen.h"
#include "unitempgen.h"
#include "wvlog.h"

/**
 * Loads and saves configuration files of the specified type to and
 * from the specified directory.  If the generator is told to
 * do a recursive search of directories, all files in the specified
 * directory as well as ALL sub-directories will be stored.
 *  
 */
class UniFileTreeGen : public UniTempGen
{
    WvString directory, moniker;
    bool recursive;
    WvLog log;
    
public:
    /**
     * Creates a generator which can load/modify/save configuration
     * files from the specified directory using the specified
     * moniker.
     */
    UniFileTreeGen(WvStringParm dir, WvStringParm genmoniker, bool rec)
        : directory(dir), moniker(genmoniker), recursive(rec), log(directory)
    { }

    virtual ~UniFileTreeGen() { }
    
private:
    void map_directory();
};


#endif // __UNICONFFILETREEGEN_H
