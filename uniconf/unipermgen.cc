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
    IUniConfGen *gen = wvcreate<IUniConfGen>(moniker);
    assert(gen && "Moniker doesn't get us a generator!");
    setinner(new UniDefGen(gen));
}


void UniPermGen::setowner(const UniConfKey &path, WvStringParm owner)
{
    inner()->set(WvString("%s/owner", path), owner);
}


WvString UniPermGen::getowner(const UniConfKey &path)
{
    WvString owner = inner()->get(WvString("%s/owner", path));
    if (owner.isnull() && !path.isempty())
        owner = getowner(path.removelast());
    return owner;
}


void UniPermGen::setgroup(const UniConfKey &path, WvStringParm group)
{
    inner()->set(WvString("%s/group", path), group);
}


WvString UniPermGen::getgroup(const UniConfKey &path)
{
    WvString group = inner()->get(WvString("%s/group", path));
    if (group.isnull() && !path.isempty())
        group = getgroup(path.removelast());
    return group;
}


void UniPermGen::setperm(const UniConfKey &path, Level level, Type type, bool val)
{
    inner()->set(WvString("%s/%s-%s", path, level2str(level), type2str(type)), val);
}


bool UniPermGen::getperm(const UniConfKey &path, const Credentials &cred, Type type)
{
    WvString owner = getowner(path);
    WvString group = getgroup(path);

    Level level;
    if (cred.user == owner) level = USER;
    else if (cred.groups[group]) level = GROUP;
    else level = WORLD;

    bool perm = getoneperm(path, level, type);
    return perm;
}


bool UniPermGen::getoneperm(const UniConfKey &path, Level level, Type type)
{
    int val = str2int(inner()->get(WvString("%s/%s-%s", path, level2str(level),
                type2str(type))), -1);
    if (val == -1)
    {
        if (path.isempty())
        {
            switch (type)
            {
                case READ: return true;
                case WRITE: return false;
                case EXEC: return true; 
                default: assert(false && "Something in the Type enum wasn't covered");
            }
        }
        else
            return getoneperm(path.removelast(), level, type);
    }
    return val;
}


void UniPermGen::chmod(const UniConfKey &path, unsigned int user, unsigned int group,
        unsigned int world)
{
    static const int r = 4;
    static const int w = 2;
    static const int x = 1;

    setperm(path, USER, READ, (user & r));
    setperm(path, USER, WRITE, (user & w));
    setperm(path, USER, EXEC, (user & x));

    setperm(path, GROUP, READ, (group & r));
    setperm(path, GROUP, WRITE, (group & w));
    setperm(path, GROUP, EXEC, (group & x));

    setperm(path, WORLD, READ, (world & r));
    setperm(path, WORLD, WRITE, (world & w));
    setperm(path, WORLD, EXEC, (world & x));
}


void UniPermGen::chmod(const UniConfKey &path, unsigned int mode)
{
    unsigned int user =  (mode & 0700) >> 6;
    unsigned int group = (mode & 0070) >> 3;
    unsigned int world = (mode & 0007);
    
    chmod(path, user, group, world);
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
