/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * UniPermGen is a UniConfGen for holding Unix-style permissions
 *
 */

#include "unipermgen.h"
#include "unidefgen.h"

#include "wvmoniker.h"
#include "wvstringlist.h"
#include "wvtclstring.h"


UniPermGen::UniPermGen(UniConfGen *_gen) :
    UniFilterGen(new UniDefGen(_gen))
{
}


UniPermGen::UniPermGen(WvStringParm moniker) :
    UniFilterGen(NULL)
{
    UniConfGen *gen = wvcreate<UniConfGen>(moniker);
    assert(gen && "Moniker doesn't get us a generator!");
    setinner(new UniDefGen(gen));
}


void UniPermGen::setowner(const UniConfKey &path, WvStringParm owner)
{
    inner()->set(WvString("%s/owner", path), owner);
}


WvString UniPermGen::getowner(const UniConfKey &path)
{
    return inner()->get(WvString("%s/owner", path));
}


void UniPermGen::setgroup(const UniConfKey &path, WvStringParm group)
{
    inner()->set(WvString("%s/group", path), group);
}


WvString UniPermGen::getgroup(const UniConfKey &path)
{
    return inner()->get(WvString("%s/group", path));
}


void UniPermGen::setperm(const UniConfKey &path, Level level, Type type, bool val)
{
    inner()->set(WvString("%s/%s-%s", path, level2str(level), type2str(type)), val);
}


bool UniPermGen::getperm(const UniConfKey &path, const Credentials &cred, Type type)
{
    WvString owner = inner()->get(WvString("%s/owner", path));
    WvString group = inner()->get(WvString("%s/group", path));

    bool def;
    switch (type)
    {
    case READ: def = true; break;
    case WRITE: def = false; break;
    case EXEC: def = true; break;
    default: assert(false && "Something in the Type enum wasn't covered"); break;
    }

    bool w = str2int(inner()->get(WvString("%s/world-%s", path, type2str(type))), def);
    bool g = str2int(inner()->get(WvString("%s/group-%s", path, type2str(type))), def);
    bool u = str2int(inner()->get(WvString("%s/user-%s", path, type2str(type))), def);

    return (w || (g && cred.groups[group]) || (u && cred.user == owner));
}


void UniPermGen::chmod(const UniConfKey &path, int user, int group, int world)
{
    static const int r = 4;
    static const int w = 2;
    static const int x = 1;

    inner()->set(WvString("%s/user-read", path), (user & r));
    inner()->set(WvString("%s/user-write", path), (user & w));
    inner()->set(WvString("%s/user-exec", path), (user & x));

    inner()->set(WvString("%s/group-read", path), (group & r));
    inner()->set(WvString("%s/group-write", path), (group & w));
    inner()->set(WvString("%s/group-exec", path), (group & x));

    inner()->set(WvString("%s/world-read", path), (world & r));
    inner()->set(WvString("%s/world-write", path), (world & w));
    inner()->set(WvString("%s/world-exec", path), (world & x));
}


void UniPermGen::chmod(const UniConfKey &path, int mode)
{
    chmod(path, mode & 0700, mode & 0070, mode & 0007);
}


WvString UniPermGen::level2str(Level level)
{
    switch (level)
    {
    case USER: return "user";
    case GROUP: return "group";
    case WORLD: return "world";
    }
    assert(false && "Something in the Level enum wasn't covered");
    return WvString::null;
}


WvString UniPermGen::type2str(Type type)
{
    switch (type)
    {
    case READ: return "read";
    case WRITE: return "write";
    case EXEC: return "exec";
    }
    assert(false && "Something in the Type enum wasn't covered");
    return WvString::null;
}
