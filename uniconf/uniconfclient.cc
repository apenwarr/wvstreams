/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * UniConfClient is a UniConfGen for retrieving data from the UniConfDaemon.
 *
 * see "uniconfclient.h"
 */

#include <uniconfclient.h>

UniConfClient::UniConfClient(UniConf *_top, WvStream *conn) : WvStreamClone(conn),
    top(_top), log("UniConfClient"), dict(5)
{
}

void UniConfClient::savesubtree(UniConf *tree, UniConfKey key)
{
    // Ok, check to see if *THIS* subtree is dirty.
    if (tree->dirty)
    {
        // Ok, we're dirty, send our information to the daemon.
        WvString data("set %s %s\n", wvtcl_escape(key), wvtcl_escape(*tree));
        print(data);
    }
    
    // No, we're not... what about our children.. do we have dirty children?
    if (tree->child_dirty)
    {
        // yes?  shame shame.. save them  clean them up.
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
    if (isok())
        // working.. yay, great, good.  Now, ship this subtree off to savesubtree
        savesubtree(top, "/");
    else
        log(WvLog::Error, "Connection was unuseable to save data.\n");
        // Not working.. damn.. log it and stop.
}

UniConf *UniConfClient::make_tree(UniConf *parent, const UniConfKey &key)
{
   // Now, do a get on the key from the daemon
    write(WvString("get %s\n", wvtcl_escape(key)));
    // Get the node which we're actually going to return...
    UniConf *toreturn = UniConfGen::make_tree(parent, key);
    // Now, we're actually waiting to get information back.
    toreturn->waiting = true;
    // Now wait for the response regarding this key.
    update(toreturn);
    return toreturn;
}

void UniConfClient::update(UniConf *&h)
{
    waitingdata *data = dict[(WvString)h->gen_full_key().printable()];
    if (!data && select(-1, true, false, false))
    {
        callback();
        data = dict[(WvString)h->gen_full_key().printable()];
    }
    
    // First check to see if we are in the dict..
    // if we are, then just return.  otherwise, do a select on the stream
    // and update our data appropriately.

    if (data) //&& data->key == h->gen_full_key())
    {
        h->set(data->value);
        //dict.remove(data);
        h->waiting = false;
    }

    // Mark this as not being dirty
    h->dirty = false;
}

void UniConfClient::execute()
{
    WvStreamClone::execute();

    int len = -1; 
    WvBuffer inc;
    while (len != 0)
    {
        char cptr[1024];
        len = read(cptr, 1023);
        cptr[len] = '\0';
        inc.put(cptr);
    }

    WvString *line = wvtcl_getword(inc, "\n");
    
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
            if (*cmd == "RETN")
            {
                WvString *value = wvtcl_getword(fromline);
                dict.add(new waitingdata(key->unique(), value->unique()), true);
                wvcon->print("Received:%s=%s.\n", *key, *value);
            }
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
        line = wvtcl_getword(inc, "\n");
    }
   
}
