/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * UniSecureGen is a UniConfGen for checking security on data 
 *
 */

#include "unisecuregen.h"
#include "wvmoniker.h"
#include "wvstringlist.h"
#include "wvtclstring.h"
#include "wvlog.h"


UniSecureGen::UniSecureGen(UniConfGen *_gen, UniPermGen *_perms) :
    UniFilterGen(_gen),
    perms(_perms)
{
}


UniSecureGen::UniSecureGen(WvStringParm moniker, UniPermGen *_perms) :
    UniFilterGen(NULL)
{
    UniConfGen *_gen = wvcreate<UniConfGen>(moniker);
    assert(_gen && "Moniker doesn't get us a generator!");
    setinner(_gen);
    perms = _perms;
}


void UniSecureGen::setcredentials(const UniPermGen::Credentials &_cred)
{
    cred.user = _cred.user;
    cred.groups.zap();
    WvStringTable::Iter i(_cred.groups);
    for (i.rewind(); i.next(); )
        cred.groups.add(new WvString(*i), true);
}


WvString UniSecureGen::get(const UniConfKey &key)
{
    if (findperm(key, UniPermGen::READ))
        return UniFilterGen::get(key);
    return WvString::null;
}


bool UniSecureGen::exists(const UniConfKey &key)
{
    if (findperm(key.removelast(), UniPermGen::EXEC))
        return UniFilterGen::exists(key);
    return false;
}


void UniSecureGen::set(const UniConfKey &key, WvStringParm value)
{
    if (findperm(key, UniPermGen::WRITE))
        UniFilterGen::set(key, value);
}


bool UniSecureGen::haschildren(const UniConfKey &key)
{
    if (findperm(key, UniPermGen::EXEC))
        return UniFilterGen::haschildren(key);
    return false;
}


UniConfGen::Iter *UniSecureGen::iterator(const UniConfKey &key)
{
    if (findperm(key, UniPermGen::EXEC))
        return UniFilterGen::iterator(key);
    return new UniConfGen::NullIter();
}


void UniSecureGen::gencallback(const UniConfKey &key, WvStringParm value,
        void *userdata)
{
    if (findperm(key, UniPermGen::READ))
        delta(key, value);
}


bool UniSecureGen::findperm(const UniConfKey &key, UniPermGen::Type type)
{
    if (!drilldown(key.removelast()))
        return false;
    else
        return perms->getperm(key, cred, type);
}


bool UniSecureGen::drilldown(const UniConfKey &key)
{
    UniConfKey::Iter i(key);
    for (i.rewind(); i.next(); )
    {
        bool ok = perms->getperm(*i, cred, UniPermGen::EXEC);
        wvcon->print("Checking path %s: %s\n", *i, ok ? "ok" : "no");
        if (!ok)
            return false;
    }
    return true;
}
