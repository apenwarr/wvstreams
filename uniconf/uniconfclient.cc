/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * UniConfClient is a UniConfGen for retrieving data from the UniConfDaemon.
 *
 * see "uniconfclient.h"
 */

#include <uniconfclient.h>

UniConfClient::UniConfClient(UniConf *_top, WvStream *stream, WvStreamList *l, bool automount) :
    top(_top), log("UniConfClient"), dict(5), list(l)
{
    // FIXME:  This is required b/c some WvStreams (i.e. WvTCPConn) don't
    // actually try to finish connecting until in the first pre_select.
    conn = new UniConfConn(stream);
    conn->select(15000, true, false, false);
    conn->setcallback(wvcallback(WvStreamCallback, *this, UniConfClient::execute), NULL);

    list->append(conn, true);

    waitforsubt = false;
    if (automount)
        top->mount(this);
}

UniConfClient::~UniConfClient()
{
    if (conn)
    {
        if (conn->isok())
            conn->write(WvString("%s\n", UniConfConn::UNICONF_QUIT));
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
        WvString data("%s %s %s\n", UniConfConn::UNICONF_SET, wvtcl_escape(key), wvtcl_escape(*tree));
        conn->print(data);
        tree->dirty = false;
    }
    
    // What about our children.. do we have dirty children?
    if (tree->child_dirty)
    {
        tree->child_dirty = false;
	UniConf::Iter i(*tree);

        for (i.rewind(); i.next();)
        {
            if (i->hasgen() && !i->comparegen(this))//i->generator && this != i->generator)
                continue;

            if (i->dirty || i->child_dirty)
            {
                UniConfKey key2(key);
                key2.append(&i->name, false);
                savesubtree(i.ptr(), key2);
            }
        }
    }
    // done.
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
    conn->alarm(5000);

    while (waitforsubt && conn->isok())
    {
        if (conn->select(500, true, false, false))
        {
            conn->callback();
        }

        if (conn->alarm_was_ticking)
        {
            waitforsubt = false;
        }
    }
    conn->alarm(-1);
}

void UniConfClient::update(UniConf *&h)
{
    WvString lookfor(h->full_key(top));

    if (conn->select(0,true,false,false))
        conn->callback();

    waitingdata *data = dict[lookfor];

    if (!data && (h->waiting || (h->obsolete && !h->dirty)))
    {
        if (conn && conn->isok())
            conn->print(WvString("%s %s\n", UniConfConn::UNICONF_GET, wvtcl_escape(lookfor)));
        else
        {
            h->waiting = false;
            return;
        }
    }

    // If we're waiting, we KNOW the value is coming if we don't have it yet.
    if (h->waiting && !data && conn->isok())
    {
        conn->select(-1, true, false, false);

        conn->callback();
        data = dict[lookfor];
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

void UniConfClient::executereturn(WvString &key, WvConstStringBuffer &fromline)
{
    WvString value = wvtcl_getword(fromline);
    waitingdata *data = dict[key];
    if (data == NULL)
    {
        dict.add(new waitingdata(key.unique(), value.unique()),
                true);
    }
    else
    {
        data->value = value.unique();
    }

    UniConf *temp = &top->get(key);
    if (!temp->dirty)
    {
        temp->obsolete = true;
        temp->notify = true;
        for (UniConf *par = temp->parent; par != NULL; par = par->parent)
        {
            par->child_notify = true;
        }
    }
}

void UniConfClient::executeforget(WvString &key)
{
    dict.remove(dict[key]);
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

void UniConfClient::executesubtree(WvString &key, WvConstStringBuffer &fromline)
{
    waitforsubt = false;
    while (fromline.used() > 0)
    {
        WvString pair = wvtcl_getword(fromline);
        WvDynamicBuffer temp;
        temp.putstr(pair);
        WvString newkey = wvtcl_getword(temp);
        WvString newval = wvtcl_getword(temp);
        dict.add(new waitingdata(newkey.unique(),
                    newval.unique()), true);
        UniConf *narf = &top->get(key);
        narf = &narf->get(newkey);
    }
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
            WvString key = wvtcl_getword(fromline);
            if (cmd.isnull() || key.isnull())
                break;
           
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
}


