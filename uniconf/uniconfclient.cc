/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * UniConfClient is a UniConfGen for retrieving data from the UniConfDaemon.
 *
 * see "uniconfclient.h"
 */

#include <uniconfclient.h>

UniConfClient::UniConfClient(UniConf *_top, UniConfConnFactory *_fctry/*WvStream *conn*/) : /*UniConfConn(conn),*/
    top(_top), fctry(_fctry), log("UniConfClient"), dict(5)
{
    conn = fctry->open();
}

UniConfClient::~UniConfClient()
{
    conn->close();
    delete conn;
}

void UniConfClient::savesubtree(UniConf *tree, UniConfKey key)
{
    if (tree->dirty && !tree->obsolete)
    {
        WvString data("set %s %s\n", wvtcl_escape(key), wvtcl_escape(*tree));
        conn->print(data);
    }
    
    // What about our children.. do we have dirty children?
    if (tree->child_dirty)
    {
	UniConf::Iter i(*tree);

        for (i.rewind(); i.next();)
        {
            if (i->generator && this != i->generator)
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
    if (!conn->isok())
    {
        log(WvLog::Debug2, "Connection was unuseable.  Creating another.\n");
        conn = fctry->open();
        if (!conn->isok()) // we're borked
        {
            log(WvLog::Error, "Unable to create new connection.  Save aborted.\n");
            return;
        }
    }
    
    if (conn->select(0, true, false, false))
        execute();
    // working.. yay, great, good.  Now, ship this subtree off to savesubtree
    savesubtree(top, "/");
}

UniConf *UniConfClient::make_tree(UniConf *parent, const UniConfKey &key)
{
   // Now, do a get on the key from the daemon
    conn->print(WvString("get %s\n", wvtcl_escape(key)));
    // Get the node which we're actually going to return...
    UniConf *toreturn = UniConfGen::make_tree(parent, key);
    // Now wait for the response regarding this key.
    if (toreturn->waiting)
        update(toreturn);
    return toreturn;
}

void UniConfClient::update_tree()
{
    if (conn->select(0, true, false, false))
        conn->callback();
}

void UniConfClient::update(UniConf *&h)
{
    waitingdata *data = dict[(WvString)h->gen_full_key()];
    if (conn->select(0,true, false, false) 
    || (h->waiting && !data && conn->select(-1, true, false, false)))
    {
        //conn->callback();
        execute();
        data = dict[(WvString)h->gen_full_key()];
    }
    
    if (data) 
    {
        // If we are here, we will not longer be waiting nor will our data be
        // obsolete.
        h->set(data->value.unique());
        h->waiting = false;
        h->obsolete = false;
        dict.remove(data);
    }

    h->dirty = false;
}


void UniConfClient::execute()
{
    conn->fillbuffer();

    WvString *line = conn->gettclline();
    
    if (!line) return;

    WvBuffer fromline;
    fromline.put(*line);
    WvString *cmd = NULL;
    WvString *key = NULL;
    while (line)
    {
        cmd = wvtcl_getword(fromline);
        key = wvtcl_getword(fromline);
        while (cmd && key)
        {
            // Value from a get is incoming
            if (*cmd == "RETN") 
            {
                WvString *value = wvtcl_getword(fromline);
                if (!value)
                {
                    value = new WvString();
                }
                dict.add(new waitingdata(key->unique(), value->unique()), true);
            }
            // A set has happened on a key we requested.
            else if (*cmd == "FGET") 
            {
                dict.remove(dict[*key]);
                UniConf *obs = &(*top)[*key];
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

            cmd = wvtcl_getword(fromline);
            key = wvtcl_getword(fromline);
        }
        line = conn->gettclline();
    }
   
}
