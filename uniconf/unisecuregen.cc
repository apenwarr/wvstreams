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
    IUniConfGen *_gen = wvcreate<IUniConfGen>(moniker);
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


void UniSecureGen::setcredentials(WvStringParm user, const WvStringList &groups)
{
    cred.user = user;
    cred.groups.zap();
    WvStringList::Iter i(groups);
    for (i.rewind(); i.next(); )
        cred.groups.add(new WvString(*i), true);
}


WvString UniSecureGen::get(const UniConfKey &key)
{
    if (findperm(key, UniPermGen::READ))
    {
        WvString val = UniFilterGen::get(key);
        return val;
    }
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
    if (!drilldown(key))
        return false;
    else
        return perms->getperm(key, cred, type);
}


bool UniSecureGen::drilldown(const UniConfKey &key)
{
    UniConfKey check;
    UniConfKey left = key;

    while (!left.isempty())
    {
        // check the exec perm
        if (!perms->getperm(check, cred, UniPermGen::EXEC))
            return false;

        // move the first segment of left to check
        // note that when left is empty, we exit the loop before checking the
        // last segment.  That's on purpose: the last segment is the 'file'
        // and we only need to check the 'directories'
        check.append(left.first());
        left = left.removefirst();
    }
    return true;
}
