/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Manages a UniConf daemon session which is authenticated through PAM.
 */
#include "uniconfpamconn.h"
#include "unisecuregen.h"
#include "wvpam.h"


unsigned int WvHash(const UniConfGen *u)
{
    return WvHash((int) u);
}


/***** UniConfPamConn *****/


SecureGenDict UniConfPamConn::securegens(7);


UniConfPamConn::UniConfPamConn(WvStream *_s, const UniConf &_root) :
    UniConfDaemonConn(new WvPamStream(_s, "uniconfdaemon", WvString::null,
            "FAIL { Not authorized }"), _root)
{
}


void UniConfPamConn::addcallback()
{
    root.add_callback(UniConfCallback(this,
            &UniConfPamConn::deltacallback), true);
}


void UniConfPamConn::delcallback()
{
    root.del_callback(UniConfCallback(this,
            &UniConfPamConn::deltacallback), true);
}


void UniConfPamConn::do_get(const UniConfKey &key)
{
    updatecred(root[key]);
    UniConfDaemonConn::do_get(key);
}


void UniConfPamConn::do_set(const UniConfKey &key, WvStringParm value)
{
    updatecred(root[key]);
    UniConfDaemonConn::do_set(key, value);
}


void UniConfPamConn::do_remove(const UniConfKey &key)
{
    updatecred(root[key]);
    UniConfDaemonConn::do_remove(key);
}


void UniConfPamConn::do_subtree(const UniConfKey &key)
{
    updatecred(root[key]);
    UniConfDaemonConn::do_subtree(key);
}


void UniConfPamConn::do_haschildren(const UniConfKey &key)
{
    updatecred(root[key]);
    UniConfDaemonConn::do_haschildren(key);
}


void UniConfPamConn::deltacallback(const UniConf &cfg, const UniConfKey &key)
{
    updatecred(cfg[key]);
    UniConfDaemonConn::deltacallback(cfg, key);

    // FIXME: looks like if there's no permission to read, pamconn will tell
    // the client that it's been deleted instead of just staying silent.
}


void UniConfPamConn::updatecred(const UniConf &key)
{
    // get the user and groups from PAM
    WvPamStream *pam = static_cast<WvPamStream *>(cloned);
    WvString user = pam->getuser();
    WvStringList groups;
    pam->getgroups(groups);
    
    // if this isn't a UniSecureGen, don't need to authenticate
    SecureGen *sec = securegens[key.whichmount(NULL)];
    if (sec && sec->data) sec->data->setcredentials(user, groups);
}
