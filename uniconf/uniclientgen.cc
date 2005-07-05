/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * UniClientGen is a UniConfGen for retrieving data from the
 * UniConfDaemon.
 */
#include "wvfile.h"
#include "uniclientgen.h"
#include "wvtclstring.h"
#include "wvtcp.h"
#include "wvaddr.h"
#include "wvresolver.h"
#include "wvmoniker.h"
#include "wvsslstream.h"
#include "unilistiter.h"
#include "wvlinkerhack.h"

WV_LINK(UniClientGen);


#ifndef _WIN32
#include "wvunixsocket.h"
static IUniConfGen *unixcreator(WvStringParm s, IObject *, void *)
{
    return new UniClientGen(new WvUnixConn(s));
}
static WvMoniker<IUniConfGen> unixreg("unix", unixcreator);
#endif


static IUniConfGen *tcpcreator(WvStringParm _s, IObject *, void *)
{
    WvString s(_s);
    char *cptr = s.edit();
    
    if (!strchr(cptr, ':')) // no default port
	s.append(":%s", DEFAULT_UNICONF_DAEMON_TCP_PORT);
    
    return new UniClientGen(new WvTCPConn(s), _s);
}


static IUniConfGen *sslcreator(WvStringParm _s, IObject *, void *)
{
    WvString s(_s);
    char *cptr = s.edit();
    
    if (!strchr(cptr, ':')) // no default port
	s.append(":%s", DEFAULT_UNICONF_DAEMON_SSL_PORT);
    
    return new UniClientGen(new WvSSLStream(new WvTCPConn(s), NULL), _s);
}


// if 'obj' is a WvStream, build the uniconf connection around that;
// otherwise, create a new WvStream using 's' as the wvstream moniker.
static IUniConfGen *wvstreamcreator(WvStringParm s, IObject *obj, void *)
{
    IWvStream *stream = NULL;
    if (obj)
	stream = mutate<IWvStream>(obj);
    if (!stream)
	stream = wvcreate<IWvStream>(s);
    return new UniClientGen(stream);
}

#ifdef WITH_SLP
#include "wvslp.h"

// FIXME: Only gets the first
static IUniConfGen *slpcreator(WvStringParm s, IObject *obj, void *)
{
    WvStringList serverlist;
    
    if (slp_get_servs("uniconf.niti", serverlist))
    {
	WvString server = serverlist.popstr();
	printf("Creating connection to: %s\n", server.cstr());
	return new UniClientGen(new WvTCPConn(server), s);
    }
    else
        return NULL;
}

static WvMoniker<IUniConfGen> slpreg("slp", slpcreator);
#endif

static WvMoniker<IUniConfGen> tcpreg("tcp", tcpcreator);
static WvMoniker<IUniConfGen> sslreg("ssl", sslcreator);
static WvMoniker<IUniConfGen> wvstreamreg("wvstream", wvstreamcreator);




/***** UniClientGen *****/

UniClientGen::UniClientGen(IWvStream *stream, WvStringParm dst) 
    : log(WvString("UniClientGen to %s",
		   dst.isnull() && stream->src() 
		   ? *stream->src() : WvString(dst)))
{
    cmdinprogress = cmdsuccess = false;
    result_list = NULL;

    conn = new UniClientConn(stream, dst);
    conn->setcallback(WvStreamCallback(this,
        &UniClientGen::conncallback), NULL);
    WvIStreamList::globallist.append(conn, false, "uniclientconn-via-gen");
}


UniClientGen::~UniClientGen()
{
    if (isok())
	conn->writecmd(UniClientConn::REQ_QUIT, "");
    WvIStreamList::globallist.unlink(conn);
    WVRELEASE(conn);
}


bool UniClientGen::isok()
{
    return (conn && conn->isok());
}


bool UniClientGen::refresh()
{
    conn->writecmd(UniClientConn::REQ_REFRESH);
    return do_select();
}

void UniClientGen::flush_buffers()
{
    // this ensures that all keys pending notifications are dealt with
    while (conn->isok() && conn->isreadable())
        conn->callback();
}

void UniClientGen::commit()
{
    conn->writecmd(UniClientConn::REQ_COMMIT);
    do_select();
}

WvString UniClientGen::get(const UniConfKey &key)
{
    WvString value;
    conn->writecmd(UniClientConn::REQ_GET, wvtcl_escape(key));

    if (do_select())
    {
        if (result_key == key)
            value = result;
//        else
//            seterror("Error: server sent wrong key pair.");
    }
    return value;
}


void UniClientGen::set(const UniConfKey &key, WvStringParm newvalue)
{
    //set_queue.append(new WvString(key), true);
    hold_delta();

    if (newvalue.isnull())
        conn->writecmd(UniClientConn::REQ_REMOVE, wvtcl_escape(key));
    else
        conn->writecmd(UniClientConn::REQ_SET,
            WvString("%s %s", wvtcl_escape(key), wvtcl_escape(newvalue)));

    unhold_delta();
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


UniClientGen::Iter *UniClientGen::do_iterator(const UniConfKey &key,
					      bool recursive)
{
    assert(!result_list);
    result_list = new UniListIter(this);
    conn->writecmd(UniClientConn::REQ_SUBTREE,
		   WvString("%s %s", wvtcl_escape(key), WvString(recursive)));

    if (do_select())
    {
	ListIter *it = result_list;
	result_list = NULL;
	return it;
    }
    else
    {
	delete result_list;
	result_list = NULL;
	return NULL;
    }
}


UniClientGen::Iter *UniClientGen::iterator(const UniConfKey &key)
{
    return do_iterator(key, false);
}
    

UniClientGen::Iter *UniClientGen::recursiveiterator(const UniConfKey &key)
{
    return do_iterator(key, true);
}
    

void UniClientGen::conncallback(WvStream &stream, void *userdata)
{
    if (conn->alarm_was_ticking)
    {
        // command response took too long!
        log(WvLog::Warning, "Command timeout; connection closed.\n");
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
			result_list->add(key, value);
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


// FIXME: horribly horribly evil!!
bool UniClientGen::do_select()
{
    hold_delta();
    
    cmdinprogress = true;
    cmdsuccess = false;

    conn->alarm(TIMEOUT);
    while (conn->isok() && cmdinprogress)
    {
	// We would really like to run the "real" wvstreams globallist
	// select loop here, but we can't because we may already be inside
	// someone else's callback or something.  So we'll wait on *only* this
	// connection.
        if (conn->select(-1, true, false))
        {
            conn->callback();
            conn->alarm(TIMEOUT);
        }
    }
    conn->alarm(-1);

//    if (!cmdsuccess)
//        seterror("Error: server timed out on response.");

    unhold_delta();
    
    return cmdsuccess;
}
