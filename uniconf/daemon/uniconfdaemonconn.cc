
#include "uniconfdaemonconn.h"
#include "uniconfdaemon.h"
#include "uniconfiter.h"
#include "wvtclstring.h"


UniConfDaemonConn::UniConfDaemonConn(WvStream *_s, UniConfDaemon *_source)
    : UniConfConn(_s), 
      log(WvString("UniConf to %s", *_s->src()), WvLog::Debug)
{
    print("HELLO UniConf Server ready\n");
    source = _source;
}


UniConfDaemonConn::~UniConfDaemonConn()
{
    WvStringList::Iter i(keys);
    for (i.rewind(); i.next();)
    {
        WvString key = *i;
        if (!key) key = "/";
        del_callback(key, this);
    }
}

void UniConfDaemonConn::keychanged(void *userdata, UniConf &conf)
{
    // All the following is irrelevant if we have a null pointer, so check
    // it first.
    if (!userdata)
        return;
    
    UniConfDaemonConn *s = (UniConfDaemonConn *)userdata;
    WvString keyname(conf.gen_full_key()); 
//    log(WvLog::Debug2, "Got a callback for key %s.\n", keyname);

    WvString response("%s %s %s\n", UNICONF_RETURN, wvtcl_escape(keyname),
            !!source->mainconf.get(keyname) ? wvtcl_escape(source->mainconf.get(keyname)) :
            WvString());
    if (s->isok())
        s->print(response);
}

/* Functions to look after UniEvents callback setting / removing */

void UniConfDaemonConn::update_callbacks(WvString key, WvStream *s, bool one_shot)
{
    del_callback(key, s);
    add_callback(key, one_shot, s);
}

void UniConfDaemonConn::del_callback(WvString key, WvStream *s)
{
//    log("About to delete callbacks for %s.\n", key);
    source->events.del(wvcallback(UniConfCallback, *s,
                UniConfDaemonConn::keychanged), s, key);

}

void UniConfDaemonConn::add_callback(WvString key, bool one_shot, WvStream *s)
{
//    log("About to add callbacks for %s.\n", key);
    source->events.add(wvcallback(UniConfCallback, *s, 
                UniConfDaemonConn::keychanged), s, key, one_shot);

    // Now track what keys I know of.
//    log("Now adding key:%s, to the list of keys.\n",key);
    keys.append(new WvString(key), true);
}

/* End of functions to look after UniEvents callback setting / removing */

void UniConfDaemonConn::dook(const WvString cmd, const WvString key, WvStream *s)
{
    if (s->isok())
        s->print("%s %s %s\n", UNICONF_OK, cmd, key); 
}

void UniConfDaemonConn::doget(WvString key, WvStream *s)
{
    dook(UNICONF_GET, key, s);
    WvString response("%s %s ", UNICONF_RETURN, wvtcl_escape(key));
    if (!!source->mainconf.get(key))
        response.append("%s\n",wvtcl_escape(source->mainconf.get(key)));
    else
        response.append("\\0\n");

    if (s->isok())
    {
        s->print(response);

        // Ensure no duplication of events.
        update_callbacks(key, s);
    }
}

void UniConfDaemonConn::dosubtree(WvString key, WvStream *s)
{
    UniConf *nerf = &source->mainconf[key];
    WvString send("%s %s ", UNICONF_SUBTREE_RETURN, wvtcl_escape(key));
    
    dook(UNICONF_SUBTREE, key, s);
    
    if (nerf)
    {
        UniConf::Iter i(*nerf);
        for (i.rewind(); i.next();)
        {
            send.append("{%s %s} ", wvtcl_escape(i->name),
                    wvtcl_escape(*i));
            
            update_callbacks(key, s);
       }
    }
   
    send.append("\n");
    if (this->isok())
        print(send);
}

void UniConfDaemonConn::dorecursivesubtree(WvString key, WvStream *s)
{
    UniConf *nerf = &source->mainconf[key];
    WvString send("%s %s ", UNICONF_SUBTREE_RETURN, wvtcl_escape(key));
    
    dook(UNICONF_RECURSIVESUBTREE, key, s);
    
    if (nerf)
    {
        UniConf::RecursiveIter i(*nerf);
        for (i.rewind(); i.next();)
        {
            send.append("{%s %s} ", wvtcl_escape(i->full_key(nerf)),
                    wvtcl_escape(*i));
            
            update_callbacks(key, s);
        }
    }
    send.append("\n");
    if (this->isok())
        print(send);
}

void UniConfDaemonConn::doset(WvString key, WvConstStringBuffer &fromline, WvStream *s)
{
    WvString newvalue = wvtcl_getword(fromline);
    source->mainconf[key] = wvtcl_unescape(newvalue);
    source->keymodified = true;
    dook(UNICONF_SET, key, s);
}

void UniConfDaemonConn::registerforchange(WvString key)
{
    dook(UNICONF_REGISTER, key, this);
    update_callbacks(key, this);
}

void UniConfDaemonConn::execute()
{
    WvString line, cmd;
    
    UniConfConn::execute();
    fillbuffer();

    while (!(line = gettclline()).isnull())
    {
        WvConstStringBuffer fromline(line);

//	log(WvLog::Debug5, "Got command: '%s'\n", line);

        while (!(cmd = wvtcl_getword(fromline)).isnull())
        {
            // check the command
	    if (cmd == UNICONF_HELP)//"help")
	    {
                if (this->isok())
	    	    print("OK I know how to: help, get, subt, quit\n");
		return;	    
	    }
            if (cmd == UNICONF_QUIT)//"quit")
            {
                dook(cmd, "<null>", this);
                close();
                return;
            }
            WvString key = wvtcl_getword(fromline);
            if (key.isnull())
                break;

            if (cmd == UNICONF_GET)//"get") // return the specified value
            {
                doget(key, this);
            }
            else if (cmd == UNICONF_SUBTREE)//"subt") // return the subtree(s) of this key
            {
                dosubtree(key, this);
            }
            else if (cmd == UNICONF_RECURSIVESUBTREE)//"rsub")
            {
                dorecursivesubtree(key, this);
            }
            else if (cmd == UNICONF_SET) // set the specified value
            {
                doset(key, fromline, this);
            }
            else if (cmd == UNICONF_REGISTER)
            {
                registerforchange(key);
            }
        }
    }
}
