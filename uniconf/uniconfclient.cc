/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * UniConfClient is a UniConfGen for retrieving data from the UniConfDaemon.
 *
 * see "uniconfclient.h"
 */

#include <uniconfclient.h>

UniConfClient::UniConfClient(UniConf *_top, WvStream *stream, WvStreamList *l) :
    top(_top), log("UniConfClient"), dict(5), list(l)
{
    // FIXME:  This is required b/c some WvStreams (i.e. WvTCPConn) don't
    // actually try to finish connecting until in the first pre_select.
    conn = new UniConfConn(stream);
    conn->select(15000, true, false, false);
    conn->setcallback(wvcallback(WvStreamCallback, *this, UniConfClient::execute), NULL);

    if (list)
        list->append(conn, false);

    waitforsubt = false;
    conn->alarm(15000);
}

UniConfClient::~UniConfClient()
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

bool UniConfClient::isok()
{
    return (conn && conn->isok());
}


// Saves the current subtree of the uniconf object.  
// Note:  If conn is not ok, or if conn is NULL, then we just return.
void UniConfClient::savesubtree(UniConf *tree, UniConfKey key)
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
        if (!!*tree)
            data.append(" %s", wvtcl_escape(*tree));
        data.append("\n");
        conn->print(data);
        tree->dirty = false;
    }
   
    // What about our children.. do we have dirty children?
    if (tree->child_dirty)
    {
        tree->child_dirty = false;
	UniConf::RecursiveIter i(*tree);

        for (i.rewind(); i._next();)
        {
            if (i->hasgen() && !i->comparegen(this))
                i->save();

            else if (i->dirty || i->child_dirty)
            {
                UniConfKey key2(key);
                key2.append(&i->name, false);
                savesubtree(i.ptr(), key2);
            }
        }
    }
}

void UniConfClient::save()
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
    savesubtree(top, "/");
}

UniConf *UniConfClient::make_tree(UniConf *parent, const UniConfKey &key)
{
    // Create the node which we're actually going to return...
    UniConf *toreturn = UniConfGen::make_tree(parent, key);
    return toreturn;
}

void UniConfClient::enumerate_subtrees(UniConf *conf, bool recursive)
{
    if (!conn || !conn->isok())
        return;
    
    WvString cmd("%s ", recursive ? UniConfConn::UNICONF_RECURSIVESUBTREE : 
            UniConfConn::UNICONF_SUBTREE);

    if (conn->select(0, true, false, false))
        conn->callback(); 

    if (conf != top)
        cmd.append(wvtcl_escape(conf->full_key(top)));
    else
        cmd.append("/");
    
    cmd.append("\n");
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

void UniConfClient::pre_get(UniConf *&h)
{
    WvString lookfor("%s", h->gen_full_key());
    if (conn && conn->isok())
        conn->print("%s %s\n", UniConfConn::UNICONF_GET, wvtcl_escape(lookfor));
}

void UniConfClient::update_all()
{
    if (conn->select(0,true,false,false))
        conn->callback();

    //wvcon->print("There are %s items waiting.\n", dict.count());
    waitingdataDict::Iter i(dict);
    for (i.rewind(); i.next();)
    {
        UniConf *toupdate = top->find(i->key);
        //wvcon->print("About to update:  %s\n", i->key);
        if (toupdate && !toupdate->dirty)
            update(toupdate);
    }
    dict.zap();
    //wvcon->print("There are %s items after the zap.\n", dict.count());
}

void UniConfClient::update(UniConf *&h)
{
    assert(h != NULL);

    if ( h->full_key(top).printable().isnull() )
        return;

    WvString lookfor("%s",h->full_key(top));

    if (conn->select(0,true,false,false))
        conn->callback();

    waitingdata *data = dict[lookfor];
    
    if (!data)
    {
        pre_get(h);
        while (!data && conn->isok())
        {
            if (conn->select(0, true, false, false))
                conn->callback();
            data = dict[lookfor];
        }

        if (!conn->isok())
            h->waiting = false;
    }

    if (data) 
    {
        // If we are here, we will not longer be waiting nor will our data be
        // obsolete.
        h->set(data->value.unique());
        dict.remove(data);
        h->waiting = false;
        h->obsolete = false;
    }
    
    h->dirty = false;
}

void UniConfClient::executereturn(UniConfKey &key, WvConstStringBuffer &fromline)
{
    WvString value = wvtcl_getword(fromline);
    waitingdata *data = dict[key.printable()];
    if (data == NULL)
        dict.add(new waitingdata(key.printable(), value.unique()),true);
    else
        data->value = value.unique();

    UniConf *temp = top->find(key);
    if (temp && !temp->dirty)
    {
        temp->obsolete = true;
        temp->notify = true;
        for (UniConf *par = temp->parent; par != NULL; par = par->parent)
        {
            par->child_notify = true;
        }
    }
}

void UniConfClient::executeforget(UniConfKey &key)
{
    dict.remove(dict[key.printable()]);
    UniConf *obs = &(*top)[key];
    if (obs)
    {
        obs->obsolete = true;
        UniConf *par = obs->parent;
        while (par)
        {
            par->child_obsolete = true;
            par = par->parent;
        }
    }
}

void UniConfClient::executesubtree(UniConfKey &key, WvConstStringBuffer &fromline)
{
    waitforsubt = false;
    time_t ticks_left = conn->alarm_remaining();
    conn->alarm(-1);
    while (fromline.used() > 0)
    {
        WvString pair = wvtcl_getword(fromline);
        WvDynamicBuffer temp;
        temp.putstr(pair);
        WvString newkey = wvtcl_getword(temp);
        WvString newval = wvtcl_getword(temp);
        dict.add(new waitingdata(newkey.unique(),
                    newval.unique()), true);
        UniConf *narf = top->find(key);
        if (narf)
            narf = narf->find_make(newkey);
    }
    conn->alarm(ticks_left);
}

void UniConfClient::executeok(WvConstStringBuffer &fromline)
{
/*    log(WvLog::Debug3,"Command %s with key %s was executed successfully.\n",
                        cmd, key);*/
    fromline.zap();
}

void UniConfClient::executefail(WvConstStringBuffer &fromline)
{
//    log(WvLog::Debug3,"Command %s with key %s failed.\n", cmd, key);
    fromline.zap();
}

void UniConfClient::execute(WvStream &stream, void *userdata)
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
        if (dict.count())
            update_all();
        stream.alarm(15000);
    }
}


