/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniConfClientGen is a UniConfGen for retrieving data from the
 * UniConfDaemon.
 */
#include "uniconfclient.h"
#include "wvtclstring.h"
#include "wvtcp.h"
#include "wvunixsocket.h"
#include "wvaddr.h"
#include "wvresolver.h"
#include "wvistreamlist.h"

#include "wvmoniker.h"

static UniConfGen *unixcreator(WvStringParm s, IObject *, void *)
{
    return new UniConfClientGen(new WvUnixConn(s));
}

static UniConfGen *tcpcreator(WvStringParm _s, IObject *, void *)
{
    WvString s(_s);
    char *cptr = s.edit();
    
    if (!strchr(cptr, ':')) // no default port
	s.append(":%s", DEFAULT_UNICONF_DAEMON_TCP_PORT);
    
    return new UniConfClientGen(new WvTCPConn(s));
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
    return new UniConfClientGen(stream);
}

static WvMoniker<UniConfGen> unixreg("unix", unixcreator);
static WvMoniker<UniConfGen> tcpreg("tcp", tcpcreator);
static WvMoniker<UniConfGen> wvstreamreg("wvstream", wvstreamcreator);


UniConfClientGen::UniConfClientGen(IWvStream *stream) :
    conn(NULL), values(71),
    streamid("UniConfClientGen to %s", *stream->src()),
    log(streamid), cmdinprogress(false), cmdsuccess(false)
{
    // FIXME:  This is required b/c some WvStreams (i.e. WvTCPConn) don't
    // actually try to finish connecting until in the first pre_select.
    conn = new UniConfConn(stream);
    conn->force_select(true, false, false);
    conn->select(15000);
    conn->setcallback(wvcallback(WvStreamCallback, *this,
        UniConfClientGen::conncallback), NULL);
    WvIStreamList::globallist.append(conn, false, streamid.edit());
}


UniConfClientGen::~UniConfClientGen()
{
    conn->writecmd(UniConfConn::REQ_QUIT, "");
    conn->close();
    WvIStreamList::globallist.unlink(conn);
    delete conn;
}


bool UniConfClientGen::isok()
{
    return (conn && conn->isok());
}


bool UniConfClientGen::refresh(const UniConfKey &key, UniConfDepth::Type depth)
{
    // TODO: no caching so nothing to be done here right now
    if (! isok())
    {
        log(WvLog::Error, "Connection died; refresh aborted.\n");
        return false;
    }
    return true;
}


bool UniConfClientGen::commit(const UniConfKey &key,
    UniConfDepth::Type depth)
{
    // TODO: no caching so nothing to be done here right now
    if (! isok())
    {
        log(WvLog::Error, "Connection died; commit aborted.\n");
        return false;
    }
    return true;
}


WvString UniConfClientGen::get(const UniConfKey &key)
{
    prepare();
    conn->writecmd(UniConfConn::REQ_GET, wvtcl_escape(key));
    bool success = wait();
    if (success)
    {
        UniConfPair *v = values[key];
        if (v)
            return v->value();
    }
    return WvString::null;
}


bool UniConfClientGen::set(const UniConfKey &key, WvStringParm value)
{
    prepare();
    if (value.isnull())
        conn->writecmd(UniConfConn::REQ_REMOVE, wvtcl_escape(key));
    else
        conn->writecmd(UniConfConn::REQ_SET,
            WvString("%s %s", wvtcl_escape(key), wvtcl_escape(value)));
    return wait();
}


bool UniConfClientGen::zap(const UniConfKey &key)
{
    prepare();
    conn->writecmd(UniConfConn::REQ_ZAP, wvtcl_escape(key));
    return wait();
}


bool UniConfClientGen::haschildren(const UniConfKey &key)
{
    // FIXME: need a more efficient way to do this
    Iter *it = iterator(key);
    it->rewind();
    bool result = it->next();
    delete it;
    return result;
}


UniConfClientGen::Iter *UniConfClientGen::iterator(const UniConfKey &key)
{
    prepare();
    conn->writecmd(UniConfConn::REQ_SUBTREE, wvtcl_escape(key));
    if (! wait() || values.isempty())
        return new NullIter();

    // FIXME: eventually we should do this incrementally
    WvStringList *keylist = new WvStringList();
    UniConfPairDict::Iter it(values);
    int trimsegments = key.numsegments();
    for (it.rewind(); it.next(); )
    {
        UniConfKey subkey(it->key().removefirst(trimsegments));
        keylist->append(new WvString(subkey), true);
    }
    return new RemoteKeyIter(keylist);
}


void UniConfClientGen::conncallback(WvStream &stream, void *userdata)
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
        UniConfConn::Command command = conn->readcmd();
        if (command == UniConfConn::NONE)
            break;
        didone = true;

        switch (command)
        {
            case UniConfConn::REPLY_OK:
                cmdsuccess = true;
                cmdinprogress = false;
                break;
                
            case UniConfConn::REPLY_FAIL:
                cmdsuccess = false;
                cmdinprogress = false;
                break;

            case UniConfConn::PART_VALUE:
            {
                WvString key(wvtcl_getword(conn->payloadbuf, " "));
                WvString value(wvtcl_getword(conn->payloadbuf, " "));
                if (key.isnull() || value.isnull())
                    break; // malformed message!
                
                UniConfPair *v = values[key];
                if (v)
                    v->setvalue(value);
                else
                    values.add(new UniConfPair(key, value), true);
                break;
            }

            case UniConfConn::PART_TEXT:
                // don't care about text messages
                break;

            case UniConfConn::EVENT_HELLO:
                // discard server information
                break;

            case UniConfConn::EVENT_CHANGED:
            {
                WvString key(wvtcl_getword(conn->payloadbuf, " "));
                WvString depthstr(wvtcl_getword(conn->payloadbuf, " "));
                if (key.isnull() || depthstr.isnull())
                    break; // malformed message!
                
                UniConfDepth::Type depth = UniConfDepth::fromname(depthstr);
                if (depth == -1)
                    depth = UniConfDepth::INFINITE; // fwd compatibility
                delta(key, depth);
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


void UniConfClientGen::prepare()
{
    values.zap();
    cmdinprogress = true;
    cmdsuccess = false;
}


bool UniConfClientGen::wait()
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
