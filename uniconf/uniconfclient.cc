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

UniConfClientGen::UniConfClientGen(
    const UniConfLocation &_location,
    UniConf *_top, WvStream *stream, WvStreamList *l) :
    UniConfGen(_location),
    top(_top), log("UniConfClientGen"), waiting(13), list(l)
{
    // FIXME:  This is required b/c some WvStreams (i.e. WvTCPConn) don't
    // actually try to finish connecting until in the first pre_select.
    conn = new UniConfConn(stream);
    conn->select(15000, true, false, false);
    conn->setcallback(wvcallback(WvStreamCallback, *this, UniConfClientGen::execute), NULL);

    if (list)
        list->append(conn, false);

    waitforsubt = false;
    conn->alarm(15000);
}

UniConfClientGen::~UniConfClientGen()
{
    if (conn)
    {
        if (conn->isok())
            conn->write(WvString("%s\n", UniConfConn::UNICONF_QUIT));
	if (list)
	    list->unlink(conn);
        delete conn;
     }
}

bool UniConfClientGen::isok()
{
    return (conn && conn->isok());
}


// Saves the current subtree of the uniconf object.  
// Note:  If conn is not ok, or if conn is NULL, then we just return.
void UniConfClientGen::savesubtree(UniConf *tree, UniConfKey key)
{
    if (!conn || !conn->isok())
    {
        log(WvLog::Error, "Could not use connection to daemon.  Save aborted.\n");
        return;
    }

    // last save wins.
    if (tree->dirty)
    {
        WvString data("%s %s", UniConfConn::UNICONF_SET, wvtcl_escape(key));
        WvString value = tree->value();
        if (!!value)
            data.append(" %s", wvtcl_escape(value));
        data.append("\n");
        conn->print(data);
        tree->dirty = false;
    }
   
    // What about our children.. do we have dirty children?
    if (tree->child_dirty)
    {
        tree->child_dirty = false;
	UniConf::RecursiveIter it(*tree);

        for (it.rewind(); it.next();)
        {
            if (it->hasgen() && ! it->comparegen(this))
                it->save();

            else if (it->dirty || it->child_dirty)
            {
                savesubtree(it.ptr(), UniConfKey(key, it->key()));
            }
        }
    }
}

void UniConfClientGen::save()
{
    // Make sure we actually need to save anything...
    if (!top->dirty && !top->child_dirty)
        return;

    // check our connection...
    if (!conn || !conn->isok())
    {
        log(WvLog::Error, "Connection was unuseable, save aborted.\n");
        return;
    }
    
    if (conn->select(0, true, false, false))
        conn->callback();
        //execute();
    // working.. yay, great, good.  Now, ship the tree off to savesubtree
    savesubtree(top, UniConfKey::EMPTY);
}

UniConf *UniConfClientGen::make_tree(UniConf *parent, const UniConfKey &key)
{
    // Create the node which we're actually going to return...
    UniConf *toreturn = UniConfGen::make_tree(parent, key);
    return toreturn;
}

void UniConfClientGen::enumerate_subtrees(UniConf *conf, bool recursive)
{
    if (!conn || !conn->isok())
        return;
    if (conn->select(0, true, false, false))
        conn->callback(); 
    
    WvString cmd("%s %s\n",
        recursive ? UniConfConn::UNICONF_RECURSIVESUBTREE : 
            UniConfConn::UNICONF_SUBTREE,
        wvtcl_escape(conf->full_key(top)));
    conn->print(cmd);

    waitforsubt = true;

    while (waitforsubt && conn->isok())
    {
        if (conn->select(500, true, false, false))
        {
            conn->callback();
        }
    }
}

void UniConfClientGen::pre_get(UniConf *&h)
{
    WvString lookfor("%s", h->gen_full_key());
    if (conn && conn->isok())
        conn->print("%s %s\n", UniConfConn::UNICONF_GET,
            wvtcl_escape(lookfor));
}

void UniConfClientGen::update_all()
{
    if (conn->select(0,true,false,false))
        conn->callback();

    //wvcon->print("There are %s items waiting.\n", dict.count());
    UniConfPairDict::Iter it(waiting);
    for (it.rewind(); it.next();)
    {
        UniConf *toupdate = top->find(it->key());
        //wvcon->print("About to update:  %s\n", i->key);
        if (toupdate && !toupdate->dirty)
            update(toupdate);
    }
    waiting.zap();
    //wvcon->print("There are %s items after the zap.\n", dict.count());
}

void UniConfClientGen::update(UniConf *&h)
{
    assert(h != NULL);

    if ( h->full_key(top).printable().isnull() )
        return;

    if (conn->select(0,true,false,false))
        conn->callback();

    UniConfKey lookfor(h->full_key(top));
    UniConfPair *data = waiting[lookfor];
    
    if (!data)
    {
        pre_get(h);
        while (!data && conn->isok())
        {
            if (conn->select(0, true, false, false))
                conn->callback();
            data = waiting[lookfor];
        }

        if (!conn->isok())
            h->waiting = false;
    }

    if (data) 
    {
        // If we are here, we will not longer be waiting nor will our data be
        // obsolete.
        h->setvalue(data->value());
        waiting.remove(data);
        h->waiting = false;
        h->obsolete = false;
    }
    
    h->dirty = false;
}

void UniConfClientGen::executereturn(UniConfKey &key, WvConstStringBuffer &fromline)
{
    WvString value = wvtcl_getword(fromline);
    UniConfPair *data = waiting[key];
    if (data == NULL)
        waiting.add(new UniConfPair(key, value), true);
    else
        data->setvalue(value);

    UniConf *temp = top->find(key);
    if (temp && !temp->dirty)
    {
        temp->obsolete = true;
        temp->notify = true;
        for (UniConf *par = temp->parent(); par != NULL;
            par = par->parent())
        {
            par->child_notify = true;
        }
    }
}

void UniConfClientGen::executeforget(UniConfKey &key)
{
    UniConfPair *data = waiting[key];
    if (data)
        waiting.remove(data);
    UniConf *obs = top->find(key);
    if (obs)
    {
        obs->obsolete = true;
        UniConf *par = obs->parent();
        while (par)
        {
            par->child_obsolete = true;
            par = par->parent();
        }
    }
}

void UniConfClientGen::executesubtree(UniConfKey &key, WvConstStringBuffer &fromline)
{
    waitforsubt = false;
    time_t ticks_left = conn->alarm_remaining();
    conn->alarm(-1);
    while (fromline.used() > 0)
    {
        WvString pair = wvtcl_getword(fromline);
        WvDynamicBuffer temp;
        temp.putstr(pair);
        UniConfKey newkey(wvtcl_getword(temp));
        WvString newval = wvtcl_getword(temp);
        waiting.add(new UniConfPair(newkey, newval), true);
        UniConf *narf = top->find(key);
        if (narf)
            narf = narf->findormake(newkey);
    }
    conn->alarm(ticks_left);
}

void UniConfClientGen::executeok(WvConstStringBuffer &fromline)
{
/*    log(WvLog::Debug3,"Command %s with key %s was executed successfully.\n",
                        cmd, key);*/
    fromline.zap();
}

void UniConfClientGen::executefail(WvConstStringBuffer &fromline)
{
//    log(WvLog::Debug3,"Command %s with key %s failed.\n", cmd, key);
    fromline.zap();
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
        if (waiting.count())
            update_all();
        stream.alarm(15000);
    }
}



/***** UniConfClientGenFactory *****/

UniConfGen *UniConfClientGenFactory::newgen(
    const UniConfLocation &location, UniConf *top)
{
    if (location.proto() == "unix")
    {
        WvUnixAddr addr(location.payload());
        return new UniConfClientGen(location, top,
            new WvUnixConn(addr), NULL);
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
            return new UniConfClientGen(location, top,
                new WvTCPConn(addr), NULL);
        }
        return NULL;
    }
}
