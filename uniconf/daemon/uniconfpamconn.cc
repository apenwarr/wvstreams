/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Manages a UniConf daemon session which is authenticated through PAM.
 */
#include "uniconfpamconn.h"
#include "unisecuregen.h"
#include "uniunwrapgen.h"
#include "uninullgen.h"
#include "wvpam.h"


UniConfPamConn::UniConfPamConn(WvStream *_s, const UniConf &_root,
			       UniPermGen *perms)
    : WvStreamClone(NULL)
{
    pam = new WvPamStream(_s, "uniconfd", WvString::null,
			  "FAIL {Not authorized.}\n");
    
    UniSecureGen *sec = new UniSecureGen(new UniUnwrapGen(_root), perms);
    
    // get the user and groups from PAM
    WvString user = pam->getuser();
    WvStringList groups;
    pam->getgroups(groups);
    
    sec->setcredentials(user, groups);
    newroot.mountgen(sec, false);
    setclone(new UniConfDaemonConn(pam, newroot));
}
