/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */
 
/** \file
 * UniConfClientGen is a UniConfGen for retrieving data from the
 * UniConfDaemon.
 */
#include "uniconfclient.h"
#include "wvtclstring.h"
#include "wvtcp.h"
#include "wvunixsocket.h"
#include "wvaddr.h"
#include "wvresolver.h"


/***** UniConfClientGen *****/

UniConfClientGen::UniConfClientGen(const UniConfLocation &location,
    WvStream *stream) :
    xlocation(location), conn(NULL),
    log("UniConfClientGen"), waiting(13),
    inprogress(false), success(false)
{
    // FIXME:  This is required b/c some WvStreams (i.e. WvTCPConn) don't
    // actually try to finish connecting until in the first pre_select.
    conn = new UniConfConn(stream);
    conn->force_select(true, false, false);
    conn->select(15000);
    conn->setcallback(wvcallback(WvStreamCallback, *this,
        UniConfClientGen::execute), NULL);
}


UniConfClientGen::~UniConfClientGen()
{
    if (conn)
    {
        if (conn->isok())
            conn->write(WvString("%s\n", UniConfConn::UNICONF_QUIT));
        delete conn;
    }
}


UniConfLocation UniConfClientGen::location() const
{
    return xlocation;
}


bool UniConfClientGen::isok()
{
    return (conn && conn->isok());
}


void UniConfClientGen::attach(WvStreamList *streamlist)
{
    streamlist->append(conn, false, "uniconf client conn");
}


void UniConfClientGen::detach(WvStreamList *streamlist)
{
    streamlist->unlink(conn);
}


bool UniConfClientGen::refresh(const UniConfKey &key,
    UniConf::Depth depth)
{
    if (! isok())
    {
        log(WvLog::Error, "Connection died; refresh aborted.\n");
        return false;
    }
    return true;
}


bool UniConfClientGen::commit(const UniConfKey &key,
    UniConf::Depth depth)
{
    if (! isok())
    {
        log(WvLog::Error, "Connection died; commit aborted.\n");
        return false;
    }
    return true;
}


WvString UniConfClientGen::get(const UniConfKey &key)
{
    if (! isok())
        return WvString::null;

    waiting.zap();
    conn->print("%s %s\n", UniConfConn::UNICONF_GET,
        wvtcl_escape(key));

    bool success = wait();
    WvString result;
    if (success)
    {
        UniConfPair *data = waiting[key];
        assert(data != NULL);
        result = data->value();
    }
    waiting.zap();
    return result;
}


bool UniConfClientGen::set(const UniConfKey &key, WvStringParm value)
{
    if (! isok())
        return false;
        
    // FIXME: this clearly does not distinguish between removing
    //        a key and setting it to an empty value
    WvString command("%s %s", UniConfConn::UNICONF_SET,
        wvtcl_escape(key));
    if (! value.isnull())
        command.append(" %s", wvtcl_escape(value));
    conn->print("%s\n", command);

    bool success = wait(true /*justneedok*/);
    return success;
}


bool UniConfClientGen::zap(const UniConfKey &key)
{
    if (! isok())
        return false;

    // FIXME: need a more efficient way to do this
    bool success = true;
    Iter *it = iterator(key);
    for (it->rewind(); isok() && it->next(); )
    {
        success = remove(UniConfKey(key, it->key())) && success;
    }
    delete it;
    return success;
}


bool UniConfClientGen::haschildren(const UniConfKey &key)
{
    if (! isok())
        return false;

    // FIXME: need a more efficient way to do this
    Iter *it = iterator(key);
    it->rewind();
    bool result = it->next();
    delete it;
    return result;
}


UniConfClientGen::Iter *UniConfClientGen::iterator(const UniConfKey &key)
{
    if (! isok())
        return new NullIter();

    // FIXME: need a more efficient way to do this (incrementally?)
    waiting.zap();
    conn->print("%s %s\n", UniConfConn::UNICONF_SUBTREE,
        wvtcl_escape(key));

    bool success = wait();
    if (! success || waiting.isempty())
        return new NullIter();

    WvStringList *keylist = new WvStringList();
    UniConfPairDict::Iter it(waiting);
    int trimsegments = key.numsegments();
    for (it.rewind(); it.next(); )
    {
        UniConfKey subkey(it->key().removefirst(trimsegments));
        keylist->append(new WvString(subkey), true);
    }
    return new RemoteKeyIter(keylist);
}


void UniConfClientGen::executereturn(UniConfKey &key,
    WvBuffer &fromline)
{
    WvString value = wvtcl_getword(fromline);
    UniConfPair *data = waiting[key];
    if (data == NULL)
    {
        data = new UniConfPair(key, value);
        waiting.add(data, true);
    }
    else
        data->setvalue(value);
    inprogress = false;
    success = true;
}


void UniConfClientGen::executeforget(UniConfKey &key)
{
    // FIXME: not currently used because we don't cache
}


void UniConfClientGen::executesubtree(UniConfKey &key,
    WvBuffer &fromline)
{
    time_t ticks_left = conn->alarm_remaining();
    conn->alarm(-1);
    while (fromline.used() > 0)
    {
        WvString pair = wvtcl_getword(fromline);
        WvConstStringBuffer temp(pair);
        
        UniConfKey newkey(wvtcl_getword(temp));
        WvString newval(wvtcl_getword(temp));
        
        waiting.add(new UniConfPair(newkey, newval), true);
    }
    conn->alarm(ticks_left);
    inprogress = false;
    success = true;
}


void UniConfClientGen::executeok(WvBuffer &fromline)
{
    fromline.zap();
    if (justneedok)
    {
        inprogress = false;
        success = true;
    }
}


void UniConfClientGen::executefail(WvBuffer &fromline)
{
    fromline.zap();
    inprogress = false;
    success = false;
}


void UniConfClientGen::execute(WvStream &stream, void *userdata)
{
    UniConfConn *s = (UniConfConn *) &stream;
    s->fillbuffer();
    for (;;)
    {
        WvString line = s->gettclline();
        if (line.isnull())
            break;
        WvConstStringBuffer fromline(line);
        for (;;)
        {
            WvString cmd = wvtcl_getword(fromline);
            WvString k = wvtcl_getword(fromline);
//            wvcon->print("Got:  %s with key %s.\n", cmd, k);
            if (cmd.isnull() || k.isnull())
                break;

            UniConfKey key(k);

           
            // Value from a get is incoming
            if (cmd == UniConfConn::UNICONF_RETURN)
            {
                executereturn(key, fromline);
            }
            // This will be for when an entire subtree has become outdated
            // but for now is used when:  
            //
            // A set has happened on a key we requested.
            else if (cmd == UniConfConn::UNICONF_FORGET)
            {
                executeforget(key);
            }
            else if (cmd == UniConfConn::UNICONF_SUBTREE_RETURN)
            {
                executesubtree(key, fromline);
            }
            else if (cmd == UniConfConn::UNICONF_OK)
            {
                executeok(fromline);
            }
            else if (cmd == UniConfConn::UNICONF_FAIL)
            {
                executefail(fromline);
            }
        }
    }
    if (stream.alarm_remaining() <= 0 && stream.alarm_was_ticking)
    {
        // command response took too long!
        inprogress = false;
        conn->close();
    }
}


bool UniConfClientGen::wait(bool _justneedok)
{
    conn->alarm(15000);
    inprogress = true;
    success = false;
    justneedok = _justneedok;
    while (inprogress)
    {
        if (conn->select(-1))
            conn->callback();
    }
    return success;
}



/***** UniConfClientGen::RemoteKeyIter *****/

UniConfClientGen::RemoteKeyIter::RemoteKeyIter(WvStringList *list) :
    xlist(list), xit(*xlist)
{
}


UniConfClientGen::RemoteKeyIter::~RemoteKeyIter()
{
    delete xlist;
}


UniConfClientGen::RemoteKeyIter *UniConfClientGen::
    RemoteKeyIter::clone() const
{
    // FIXME: this iterator is just a hack anyways
    assert(false || ! "not implemented");
    return NULL;
}


void UniConfClientGen::RemoteKeyIter::rewind()
{
    xit.rewind();
}


bool UniConfClientGen::RemoteKeyIter::next()
{
    return xit.next();
}


UniConfKey UniConfClientGen::RemoteKeyIter::key() const
{
    return UniConfKey(*xit);
}



/***** UniConfClientGenFactory *****/

UniConfGen *UniConfClientGenFactory::newgen(
    const UniConfLocation &location)
{
    if (location.proto() == "unix")
    {
        WvUnixAddr addr(location.payload());
        return new UniConfClientGen(location, new WvUnixConn(addr));
    }
    else
    {
        WvStringList hostport;
        hostport.split(location.payload(), ":");
        WvString hostname("localhost");
        int port = DEFAULT_UNICONF_DAEMON_TCP_PORT;
        WvStringList::Iter it(hostport);
        it.rewind();
        if (it.next())
        {
            hostname = it();
            if (it.next())
                port = it().num();
        }
        const WvIPAddr *hostaddr;
        WvResolver resolver;
        // FIXME: timeout does not really belong here!
        if (resolver.findaddr(500, hostname, & hostaddr) > 0)
        {
            WvIPPortAddr addr(*hostaddr, port);
            return new UniConfClientGen(location, new WvTCPConn(addr));
        }
        return NULL;
    }
}
