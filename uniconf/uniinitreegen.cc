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
    return new UniIniTreeGen(s);
}

static WvMoniker<UniConfGen> reg("initree", creator);

const WvString UniIniTreeGen::moniker = "ini";
/**
 * Creates a generator which can load/modify/save a .ini file.
 * "filename" is the local path of the .ini file
 */
UniIniTreeGen::UniIniTreeGen(WvStringParm directory) :
    UniConfFileTreeGen(directory, moniker)
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
