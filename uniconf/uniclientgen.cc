/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniClientGen is a UniConfGen for retrieving data from the
 * UniConfDaemon.
 */
#include "uniclientgen.h"
#include "uniclientconn.h"
#include "unicache.h"
#include "wvtclstring.h"
#include "wvtcp.h"
#include "wvunixsocket.h"
#include "wvaddr.h"
#include "wvresolver.h"
#include "wvistreamlist.h"

#include "wvmoniker.h"

static UniConfGen *unixcreator(WvStringParm s, IObject *, void *)
{
    return new UniClientGen(new WvUnixConn(s));
}

static UniConfGen *tcpcreator(WvStringParm _s, IObject *, void *)
{
    WvString s(_s);
    char *cptr = s.edit();
    
    if (!strchr(cptr, ':')) // no default port
	s.append(":%s", DEFAULT_UNICONF_DAEMON_TCP_PORT);
    
    return new UniClientGen(new WvTCPConn(s));
}

// if 'obj' is a WvStream, build the uniconf connection around that;
// otherwise, create a new WvStream using 's' as the wvstream moniker.
static UniConfGen *wvstreamcreator(WvStringParm s, IObject *obj, void *)
{
    IWvStream *stream = NULL;
    if (obj)
	stream = mutate<IWvStream>(obj);
    if (!stream)
	stream = wvcreate<IWvStream>(s);
    return new UniClientGen(stream);
}

static WvMoniker<UniConfGen> unixreg("unix", unixcreator);
static WvMoniker<UniConfGen> tcpreg("tcp", tcpcreator);
static WvMoniker<UniConfGen> wvstreamreg("wvstream", wvstreamcreator);



/***** UniClientGen *****/

UniClientGen::UniClientGen(IWvStream *stream) :
    conn(NULL), cache(new UniCache()),
    streamid("UniClientGen to %s", *stream->src()),
    log(streamid), cmdinprogress(false), cmdsuccess(false)
{
    // FIXME:  This is required b/c some WvStreams (i.e. WvTCPConn) don't
    // actually try to finish connecting until in the first pre_select.
    conn = new UniClientConn(stream);
    conn->force_select(true, false, false);
    conn->select(15000);
    conn->setcallback(wvcallback(WvStreamCallback, *this,
        UniClientGen::conncallback), NULL);
    WvIStreamList::globallist.append(conn, false, streamid.edit());

    // FIXME: We currently register for notifications over all keys
    //        which places undue burden on the server.
    //        This could be alleviated if UniConfGen supplied an advisory
    //        addwatch() / delwatch() interface in addition to asking for
    //        notification about keys that are currently cached.
    addwatch(UniConfKey::EMPTY, UniConfDepth::INFINITE);
}


UniClientGen::~UniClientGen()
{
    conn->writecmd(UniClientConn::REQ_QUIT, "");
    conn->close();
    WvIStreamList::globallist.unlink(conn);
    delete conn;
    delete cache;
}


bool UniClientGen::isok()
{
    return (conn && conn->isok());
}


bool UniClientGen::refresh(const UniConfKey &key, UniConfDepth::Type depth)
{
    // flush the cache
    cache->mark_unknown_exist(key);
    return true;
}


bool UniClientGen::commit(const UniConfKey &key,
    UniConfDepth::Type depth)
{
    // we use write-through caching so nothing to do
    return true;
}


WvString UniClientGen::get(const UniConfKey &key)
{
    // check the cache
    WvString value;
    UniCache::TriState state = cache->query_exist(key, & value);
    if (state == UniCache::FALSE)
        return WvString::null;
    if (state == UniCache::TRUE && ! value.isnull())
        return value;
    
    // fetch the key
    hold_delta();
    prepare();
    conn->writecmd(UniClientConn::REQ_GET, wvtcl_escape(key));
    if (wait())
    {
        state = cache->query_exist(key, & value);
        if (state == UniCache::UNKNOWN)
            seterror("Protocol error: GET expected a value");
    }
    else
    {
        cache->mark_not_exist(key);
    }
    unhold_delta();
    return value;
}


bool UniClientGen::exists(const UniConfKey &key)
{
    // check the cache
    UniCache::TriState state = cache->query_exist(key, NULL);
    if (state == UniCache::UNKNOWN)
    {
        // fetch the key
        hold_delta();
        prepare();
        conn->writecmd(UniClientConn::REQ_GET, wvtcl_escape(key));
        if (wait())
        {
            state = cache->query_exist(key, NULL);
            if (state == UniCache::UNKNOWN)
                seterror("Protocol error: EXISTS expected a value");
        }
        else
        {
            cache->mark_not_exist(key);
        }
        unhold_delta();
    }
    return state == UniCache::TRUE;
}


bool UniClientGen::set(const UniConfKey &key, WvStringParm newvalue)
{
    // check the cache
    WvString value;
    UniCache::TriState state = cache->query_exist(key, & value);
    if (state == UniCache::TRUE && value == newvalue)
        return true;
    
    // change the key
    hold_delta();
    prepare();
    bool success;
    if (newvalue.isnull())
    {
        conn->writecmd(UniClientConn::REQ_REMOVE, wvtcl_escape(key));
        success = wait();
        if (success)
        {
            // FIXME: may remove this when notifications are finished
            cache->mark_not_exist(key);
        }
    }
    else
    {
        conn->writecmd(UniClientConn::REQ_SET,
            WvString("%s %s", wvtcl_escape(key), wvtcl_escape(newvalue)));
        success = wait();
        if (success)
        {
            // FIXME: may remove this when notifications are finished
            cache->mark_exist(key, newvalue);
        }
    }
    unhold_delta();
    return success;
}


bool UniClientGen::zap(const UniConfKey &key)
{
    // check the cache
    UniCache::TriState state = cache->query_children(key, NULL);
    if (state == UniCache::FALSE)
        return false;
    
    // zap the children
    hold_delta();
    prepare();
    conn->writecmd(UniClientConn::REQ_ZAP, wvtcl_escape(key));
    bool success = wait();
    if (success)
    {
        // FIXME: may remove this when notifications are finished
        cache->mark_no_children(key);
    }
    unhold_delta();
    return success;
}


bool UniClientGen::haschildren(const UniConfKey &key)
{
    // check the cache
    UniCache::TriState state = cache->query_children(key, NULL);
    if (state == UniCache::UNKNOWN)
    {
        // fetch the list
        cache->mark_unknown_children(key); // FIXME: flushing too much here!
        hold_delta();
        prepare();
        conn->writecmd(UniClientConn::REQ_SUBTREE, wvtcl_escape(key));
        if (wait())
        {
            cache->mark_exist(key);
            cache->mark_known_children(key);
            state = cache->query_children(key, NULL);
            if (state == UniCache::UNKNOWN)
                seterror("Internal error: HASCHILDREN expected data from cache");
        }
        else
        {
            cache->mark_not_exist(key);
        }
        unhold_delta();
    }
    return state == UniCache::TRUE;
}


UniClientGen::Iter *UniClientGen::iterator(const UniConfKey &key)
{
    // check the cache
    WvStringList *keylist = new WvStringList();
    UniCache::TriState state = cache->query_children(key, keylist);
    if (state == UniCache::UNKNOWN ||
        state == UniCache::TRUE && keylist->isempty())
    {
        // fetch the list
        cache->mark_unknown_children(key); // FIXME: flushing too much here!
        prepare();
        hold_delta();
        conn->writecmd(UniClientConn::REQ_SUBTREE, wvtcl_escape(key));
        if (wait())
        {
            cache->mark_exist(key);
            cache->mark_known_children(key);
            state = cache->query_children(key, keylist);
            if (state == UniCache::UNKNOWN)
                seterror("Internal error: ITERATOR expected data from cache");
        }
        else
        {
            cache->mark_not_exist(key);
        }
        unhold_delta();
    }
    return new RemoteKeyIter(keylist);
}
    

bool UniClientGen::addwatch(const UniConfKey &key, UniConfDepth::Type depth)
{
    hold_delta();
    prepare();
    conn->writecmd(UniClientConn::REQ_ADDWATCH, WvString("%s %s",
        wvtcl_escape(key), UniConfDepth::nameof(depth)));
    bool success = wait();
    unhold_delta();
    return success;
}


bool UniClientGen::delwatch(const UniConfKey &key, UniConfDepth::Type depth)
{
    hold_delta();
    prepare();
    conn->writecmd(UniClientConn::REQ_DELWATCH, WvString("%s %s",
        wvtcl_escape(key), UniConfDepth::nameof(depth)));
    bool success = wait();
    unhold_delta();
    return success;
}


void UniClientGen::conncallback(WvStream &stream, void *userdata)
{
    if (conn->alarm_was_ticking)
    {
        // command response took too long!
        log(WvLog::Error, "Command timeout; connection closed.\n");
        cmdinprogress = false;
        cmdsuccess = false;
        conn->close();
        return;
    }
    bool didone = false;
    for (;;)
    {
        UniClientConn::Command command = conn->readcmd();
        if (command == UniClientConn::NONE)
            break;
        didone = true;

        switch (command)
        {
            case UniClientConn::REPLY_OK:
                cmdsuccess = true;
                cmdinprogress = false;
                break;
                
            case UniClientConn::REPLY_FAIL:
                cmdsuccess = false;
                cmdinprogress = false;
                break;

            case UniClientConn::PART_VALUE:
            {
                WvString key(wvtcl_getword(conn->payloadbuf, " "));
                WvString value(wvtcl_getword(conn->payloadbuf, " "));
                if (key.isnull() || value.isnull())
                    break; // malformed message!
                
                cache->mark_exist(key, value);
                break;
            }

            case UniClientConn::PART_TEXT:
                // don't care about text messages
                break;

            case UniClientConn::EVENT_HELLO:
                // discard server information
                break;

            case UniClientConn::EVENT_FORGET:
            {
                WvString key(wvtcl_getword(conn->payloadbuf, " "));
                if (key.isnull())
                    break; // malformed message!
                cache->mark_change(key, UniConfDepth::INFINITE);
                delta(key);
                break;
            }

            default:
                // discard unrecognized commands
                break;
        }
    }
    if (didone)
        conn->alarm(TIMEOUT);
}


void UniClientGen::prepare()
{
    cmdinprogress = true;
    cmdsuccess = false;
}


bool UniClientGen::wait()
{
    conn->alarm(TIMEOUT);
    while (conn->isok() && cmdinprogress)
    {
        if (conn->select(-1))
            conn->callback();
    }
    conn->alarm(-1);
    return cmdsuccess;
}



/***** UniClientGen::RemoteKeyIter *****/

UniClientGen::RemoteKeyIter::RemoteKeyIter(WvStringList *list) :
    xlist(list), xit(*xlist)
{
}


UniClientGen::RemoteKeyIter::~RemoteKeyIter()
{
    delete xlist;
}


UniClientGen::RemoteKeyIter *UniClientGen::
    RemoteKeyIter::clone() const
{
    // FIXME: this iterator is just a hack anyways
    assert(false || ! "not implemented");
    return NULL;
}


void UniClientGen::RemoteKeyIter::rewind()
{
    xit.rewind();
}


bool UniClientGen::RemoteKeyIter::next()
{
    return xit.next();
}


UniConfKey UniClientGen::RemoteKeyIter::key() const
{
    return UniConfKey(*xit);
}
