/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * UniConfClient is a UniConfGen for retrieving data from the UniConfDaemon.
 *
 * see "uniconfclient.h"
 */

#include <uniconfclient.h>

UniConfClient::UniConfClient(UniConf *_top, UniConfConnFactory *_fctry) :
    top(_top), fctry(_fctry), log("UniConfClient"), dict(5), references(0)
{
    conn = fctry->open();
    waitforsubt = false;
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
    WvString newkey(key);
    UniConf *par = parent;
    while (par)
    {
        if (par->name != "")
            newkey = WvString("%s/%s", par->name, newkey);
        par = par->parent;
    }
    conn->print(WvString("get %s\n", wvtcl_escape(newkey)));
    // Get the node which we're actually going to return...
    UniConf *toreturn = UniConfGen::make_tree(parent, key);
    // Now wait for the response regarding this key.
    if (toreturn->waiting)
        update(toreturn);
    return toreturn;
}

void UniConfClient::enumerate_subtrees(const UniConfKey &key)
{
    if (conn->select(0, true, false, false))
        execute();
    conn->print(WvString("subt %s\n", wvtcl_escape(key)));
    waitforsubt = true;
    while (waitforsubt)
    {
        if (conn->select(0, true, false, false))
            execute();
    }
}

void UniConfClient::update(UniConf *&h)
{
    waitingdata *data = dict[(WvString)h->gen_full_key()];
    if (conn->select(0,true, false, false) 
    || (h->waiting && !data && conn->select(0, true, false, false)))
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
        dict.remove(data);
    }

    h->waiting = false;
    h->obsolete = false;
    h->dirty = false;
}

bool UniConfClient::deleteable()
{
    references--;
    return 0 == references;
}

void UniConfClient::execute()
{
    conn->fillbuffer();

    WvString *line = conn->gettclline();
    
    if (!line) return;

    WvDynamicBuffer fromline;
    fromline.putstr(*line);
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
            else if (*cmd == "SUBT")  // This is so inefficient it scares me.
            {
                waitforsubt = false;
                while (fromline.used() > 0)
                {
                    WvString *pair = wvtcl_getword(fromline);
                    WvDynamicBuffer temp;
                    temp.put(*pair, pair->len());
                    WvString *newkey = wvtcl_getword(temp);
                    WvString *newval = wvtcl_getword(temp);
                    if (!newval) newval = new WvString;
                    dict.add(new waitingdata(newkey->unique(), newval->unique()), false);
                    UniConf *narf = &top->get(*key);
                    narf = &narf->get(*newkey);
                    narf->generator = this;
                }
            }

            cmd = wvtcl_getword(fromline);
            key = wvtcl_getword(fromline);
        }
        line = conn->gettclline();
    }
   
}
