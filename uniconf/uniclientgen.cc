/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniClientGen is a UniConfGen for retrieving data from the
 * UniConfDaemon.
 */
#include "uniclientgen.h"
#include "uniclientconn.h"
#include "wvtclstring.h"
#include "wvtcp.h"
#include "wvunixsocket.h"
#include "wvaddr.h"
#include "wvresolver.h"
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
    
    return new UniClientGen(new WvTCPConn(s), _s);
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

UniClientGen::UniClientGen(IWvStream *stream, WvStringParm dst) :
    conn(NULL), log(WvString("UniClientGen to %s",
    dst.isnull() ? *stream->src() : WvString(dst))),
    cmdinprogress(false), cmdsuccess(false)
{
    conn = new UniClientConn(stream, dst);
    conn->setcallback(wvcallback(WvStreamCallback, *this,
        UniClientGen::conncallback), NULL);
}


UniClientGen::~UniClientGen()
{
    conn->writecmd(UniClientConn::REQ_QUIT, "");
    delete conn;
}


bool UniClientGen::isok()
{
    return (conn && conn->isok());
}


bool UniClientGen::refresh()
{
    // FIXME: This should make sure everything in the queue has been flushed
    return true;
}


WvString UniClientGen::get(const UniConfKey &key)
{
    WvString value;
    conn->writecmd(UniClientConn::REQ_GET, wvtcl_escape(key));

    if (do_select())
    {
        if (result_key == key)
            value = result;
        else
            seterror("Error: server sent wrong key pair.");
    }
    return value;
}


void UniClientGen::set(const UniConfKey &key, WvStringParm newvalue)
{
    set_queue.append(new WvString(key), true);

    if (newvalue.isnull())
        conn->writecmd(UniClientConn::REQ_REMOVE, wvtcl_escape(key));
    else
        conn->writecmd(UniClientConn::REQ_SET,
            WvString("%s %s", wvtcl_escape(key), wvtcl_escape(newvalue)));
}


bool UniClientGen::haschildren(const UniConfKey &key)
{
    conn->writecmd(UniClientConn::REQ_HASCHILDREN, wvtcl_escape(key));

    if (do_select())
    {
        if (result_key == key && result == "TRUE")
            return true;
    }

    return false;
}


UniClientGen::Iter *UniClientGen::iterator(const UniConfKey &key)
{
    result_list = new WvStringList();
    conn->writecmd(UniClientConn::REQ_SUBTREE, wvtcl_escape(key));

    if (do_select())
        return new RemoteKeyIter(result_list);

    delete result_list;
    return new UniConfGen::NullIter();
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
        
    UniClientConn::Command command = conn->readcmd();

    switch (command)
    {
        case UniClientConn::NONE:
            // do nothing
            break;

        case UniClientConn::REPLY_OK:
            cmdsuccess = true;
            cmdinprogress = false;
            break;

        case UniClientConn::REPLY_FAIL:
            result_key = WvString::null;
            cmdsuccess = false;
            cmdinprogress = false;
            break;

        case UniClientConn::REPLY_CHILD:
            {
                WvString key(wvtcl_getword(conn->payloadbuf, " "));
                WvString value(wvtcl_getword(conn->payloadbuf, " "));

                if (!key.isnull() && !value.isnull())
                {
                    result_key = key;
                    result = value;
                    cmdsuccess = true;
                }
                cmdinprogress = false;
                break;

            }

        case UniClientConn::REPLY_ONEVAL:
            {
                WvString key(wvtcl_getword(conn->payloadbuf, " "));
                WvString value(wvtcl_getword(conn->payloadbuf, " "));

                if (!key.isnull() && !value.isnull())
                {
                    result_key = key;
                    result = value;
                    cmdsuccess = true;
                }

                cmdinprogress = false;
                break;
            }

        case UniClientConn::PART_VALUE:
            {
                WvString key(wvtcl_getword(conn->payloadbuf, " "));
                WvString value(wvtcl_getword(conn->payloadbuf, " "));

                if (!key.isnull() && !value.isnull())
                {
                    if (result_list)
                        result_list->append(new WvString(key), true);
                }
                break;
            }

        case UniClientConn::EVENT_HELLO:
            {
                WvString server(wvtcl_getword(conn->payloadbuf, " "));

                if (server.isnull() || strncmp(server, "UniConf", 7))
                {
                    // wrong type of server!
                    log(WvLog::Error, "Connected to a non-UniConf serer!\n");

                    cmdinprogress = false;
                    cmdsuccess = false;
                    conn->close();
                }                    
                break;
            }

        case UniClientConn::EVENT_NOTICE:
            {
                WvString key(wvtcl_getword(conn->payloadbuf, " "));
                WvString value(wvtcl_getword(conn->payloadbuf, " "));
                delta(key, value);
            }   

        default:
            // discard unrecognized commands
            break;
    }
}


bool UniClientGen::do_select()
{
    cmdinprogress = true;
    cmdsuccess = false;

    while (conn->isok() && cmdinprogress)
    {
        conn->alarm(TIMEOUT);
        if (conn->select(-1))
            conn->callback();
    }
    conn->alarm(-1);

    if (!cmdsuccess)
        seterror("Error: server timed out on response.");

    return cmdsuccess;
}



/***** UniClientGen::RemoteKeyIter *****/

void UniClientGen::RemoteKeyIter::rewind()
{
    i.rewind();
}


bool UniClientGen::RemoteKeyIter::next()
{
    return i.next();
}


UniConfKey UniClientGen::RemoteKeyIter::key() const
{
    return UniConfKey(*i).last();
}
