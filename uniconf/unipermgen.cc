/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * UniPermGen is a UniConfGen for holding Unix-style permissions
 *
 */

#include "unipermgen.h"
#include "wvmoniker.h"
#include "wvstringlist.h"
#include "wvtclstring.h"
#include "wvlog.h"

UniPermGen::UniPermGen(UniConfGen *_gen) :
    UniFilterGen(_gen)
{
}


UniPermGen::UniPermGen(WvStringParm moniker) :
    UniFilterGen(NULL)
{
    UniConfGen *gen = wvcreate<UniConfGen>(moniker);
    assert(gen && "Moniker doesn't get us a generator!");
    setinner(gen);
}


void UniPermGen::setowner(const UniConfKey &path, WvStringParm owner)
{
    Perms p;
    parse(inner()->get(path), p);
    p.owner = owner;
    inner()->set(path, format(p));
}


WvString UniPermGen::getowner(const UniConfKey &path)
{
    Perms p;
    parse(inner()->get(path), p);
    return p.owner;
}


void UniPermGen::setgroup(const UniConfKey &path, WvStringParm group)
{
    Perms p;
    parse(inner()->get(path), p);
    p.group = group;
    inner()->set(path, format(p));
}


WvString UniPermGen::getgroup(const UniConfKey &path)
{
    Perms p;
    parse(inner()->get(path), p);
    return p.group;
}


void UniPermGen::setperm(const UniConfKey &path, Level level, Type type, bool val)
{
    Perms p;
    parse(inner()->get(path), p);
    p.mode[level][type] = val;
    inner()->set(path, format(p));
}


bool UniPermGen::getperm(const UniConfKey &path, const Credentials &cred, Type type)
{
    Perms p;
    parse(inner()->get(path), p);

    if (p.mode[WORLD][type]) return true;
    if (cred.groups[p.group] && p.mode[GROUP][type]) return true;
    if (cred.user == p.owner && p.mode[USER][type]) return true;
    return false;
}


bool UniPermGen::defaultperm(Type type)
{
    switch (type)
    {
    case READ: return true;
    case WRITE: return false;
    case EXEC: return true;
    }
    assert(false && "Something in the Type enum wasn't covered");
    return true;
}


void UniPermGen::chmod(const UniConfKey &path, int user, int group, int world)
{
    static const int r = 4;
    static const int w = 2;
    static const int x = 1;

    Perms p;
    parse(inner()->get(path), p);
    p.mode[USER][READ] = (user & r);
    p.mode[USER][WRITE] = (user & w);
    p.mode[USER][EXEC] = (user & x);
    p.mode[GROUP][READ] = (group & r);
    p.mode[GROUP][WRITE] = (group & w);
    p.mode[GROUP][EXEC] = (group & x);
    p.mode[WORLD][READ] = (world & r);
    p.mode[WORLD][WRITE] = (world & w);
    p.mode[WORLD][EXEC] = (world & x);
    inner()->set(path, format(p));
}


void UniPermGen::chmod(const UniConfKey &path, int mode)
{
    chmod(path, mode & 0700, mode & 0070, mode & 0007);
}


WvString checknull(WvStringParm s)
{
    if (!s) return WvString::null;
    else return s;
}


void UniPermGen::parse(WvStringParm str, Perms &p)
{
    if (!!str)
    {
        WvStringList l;
        wvtcl_decode(l, str);
        p.owner = checknull(l.popstr());
        p.group = checknull(l.popstr());
        for (int i = USER; i <= WORLD; i++)
            for (int j = READ; j <= EXEC; j++)
                p.mode[i][j] = l.popstr().num();
    }
    else
    {
        p.owner = WvString::null;
        p.group = WvString::null;
        for (int i = USER; i <= WORLD; i++)
            for (int j = READ; j <= EXEC; j++)
                p.mode[i][j] = defaultperm(static_cast<Type>(j));
    }
}


WvString *nullconvert(WvStringParm s)
{
    if (!s) return new WvString("");
    return new WvString(s);
}


WvString UniPermGen::format(const Perms &p)
{
    WvStringList l;
    l.append(nullconvert(p.owner), true);
    l.append(nullconvert(p.group), true);
    for (int i = USER; i <= WORLD; i++)
        for (int j = READ; j <= EXEC; j++)
            l.append(new WvString("%s", p.mode[i][j]), true);
    return wvtcl_encode(l);
}
