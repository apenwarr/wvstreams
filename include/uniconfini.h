/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * A generator for .ini files.
 */
#ifndef __UNICONFINI_H
#define __UNICONFINI_H

#include "uniconfgen.h"
#include "uniconftemp.h"
#include "wvlog.h"

/**
 * Loads and saves ".ini"-style files similar to those used by
 * Windows, but adapted to represent keys and values using Tcl
 * style lists.
 * <p>
 * To mount, use the moniker prefix "ini://" followed by the
 * path of the .ini file.
 * </p>
 */
class UniConfIniFileGen : public UniConfTempGen
{
    WvString filename;
    WvLog log;
    
public:
    UniConfIniFileGen(WvStringParm _filename);
    virtual ~UniConfIniFileGen();
    
    /***** Overridden members *****/

    virtual UniConfLocation location() const;
    virtual bool commit(const UniConfKey &key, UniConf::Depth depth);
    virtual bool refresh(const UniConfKey &key, UniConf::Depth depth);

private:
    void save(WvStream &file, UniConfTree &parent);
};


/**
 * A factory for UniConfIniFileGen instances.
 */
class UniConfIniFileGenFactory : public UniConfGenFactory
{
public:
    virtual UniConfIniFileGen *newgen(const UniConfLocation &location);
};


#endif // __UNICONFINI_H
