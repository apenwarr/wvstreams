/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 2002 Net Integration Technologies, Inc.
 * 
 * UniSecureGen is a UniConfGen for checking permissions before allowing
 * access to a UniConf tree.  See unisecuregen.h and unipermgen.h.
 */
#include "unisecuregen.h"
#include "wvmoniker.h"
#include "wvstringlist.h"
#include "wvtclstring.h"
#include "wvlog.h"
#include "wvbuf.h"
#include "wvlinkerhack.h"

WV_LINK(UniSecureGen);


static IUniConfGen *creator(WvStringParm s)
{
    return new UniSecureGen(s);
}

static WvMoniker<IUniConfGen> reg("perm", creator);


UniSecureGen::UniSecureGen(WvStringParm moniker, UniPermGen *_perms)
    : UniFilterGen(NULL)
{
    WvString mainmon(moniker), permmon;

    if (!_perms)
    {
	WvConstInPlaceBuf buf(moniker, moniker.len());
	permmon = wvtcl_getword(buf);
	mainmon = wvtcl_getword(buf);
    
	IUniConfGen *_perms = wvcreate<IUniConfGen>(permmon);
	assert(_perms);
	perms = new UniPermGen(_perms);
	perms->refresh();
    }
    
    IUniConfGen *main = wvcreate<IUniConfGen>(mainmon);
    setinner(main);
}


UniSecureGen::UniSecureGen(IUniConfGen *_gen, UniPermGen *_perms)
    : UniFilterGen(_gen)
{
    assert(_perms);
    perms = _perms;
    perms->refresh();
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


bool UniSecureGen::refresh()
{
    perms->refresh();
    return UniFilterGen::refresh();
}


void UniSecureGen::commit()
{
    perms->commit();
    UniFilterGen::commit();
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


class _UniSecureIter : public UniConfGen::Iter
{
    UniFilterGen::Iter *it;
    UniSecureGen *gen;
    UniConfKey subpath;

public:
    _UniSecureIter(UniFilterGen::Iter *_it, UniSecureGen *_gen, UniConfKey _subpath) :
        it(_it),
        gen(_gen),
        subpath(_subpath)
        { }
    virtual ~_UniSecureIter()
        { delete it; }

    virtual void rewind() 
        { it->rewind(); }

    virtual bool next()
        { return it->next(); }

    virtual UniConfKey key() const 
        { return it->key(); } // if we've come this far, this is ok

    virtual WvString value() const
        {
            UniConfKey realkey = it->key();
            realkey.prepend(subpath);
            return gen->get(realkey);
        }               
};


UniConfGen::Iter *UniSecureGen::iterator(const UniConfKey &key)
{
    // we don't check the permissions on keys returned by the iterator, but
    // that's okay: since this iterator is non-recursive, and we've checked
    // permissions on the parent key, we know we're allowed to at least read
    // the *names* of all child keys (even if the value itself is unreadable)
    if (findperm(key, UniPermGen::EXEC))
        return new _UniSecureIter(UniFilterGen::iterator(key), this, key); 

    return NULL;
}


UniConfGen::Iter *UniSecureGen::recursiveiterator(const UniConfKey &key)
{
    // FIXME: this needs to check permissions on *every* key, not just the
    // top one, so we'll cheat: use the default UniConfGen recursiveiterator
    // instead, which just calls the non-recursive iterator recursively.
    // This can be bad for performance, but not in any of the situations
    // we currently need. (ie. security is usually done on the server side,
    // but it's the client-to-server connection that needs a fast recursive
    // iterator, so it'll be fine.)
    if (findperm(key, UniPermGen::EXEC))
	return UniConfGen::recursiveiterator(key);

    return NULL;
}


void UniSecureGen::gencallback(const UniConfKey &key, WvStringParm value)
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
