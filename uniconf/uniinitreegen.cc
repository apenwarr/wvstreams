/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A generator for .ini files.
 */
#include "uniinitreegen.h"
#include "uniinigen.h"
#include "unitempgen.h"
#include "wvtclstring.h"
#include "strutils.h"
#include "wvfile.h"
#include "wvmoniker.h"

static UniConfGen *creator(WvStringParm s, IObject *, void *)
{
    WvStringList params;
    params.split(s, ":");
    WvString *dir = params.first();
    WvString *rec = params.last();
    return new UniIniTreeGen(*dir, (*rec == "true" ? true : false));
}

static WvMoniker<UniConfGen> reg("initree", creator);

const WvString UniIniTreeGen::moniker = "ini";
/**
 * Creates a generator which can load/modify/save a .ini file.
 * "filename" is the local path of the .ini file
 */
UniIniTreeGen::UniIniTreeGen(WvStringParm directory, bool recursive) :
    UniFileTreeGen(directory, moniker, recursive)
{
}

UniIniTreeGen::~UniIniTreeGen()
{
}

/***** Overridden members *****/
/*
bool UniIniTreeGen::commit(const UniConfKey &key, UniConfDepth::Type depth)
{
}

bool UniIniTreeGen::refresh(const UniConfKey &key, UniConfDepth::Type depth)
{
}*/
