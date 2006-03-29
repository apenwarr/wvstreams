/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Manages a UniConf daemon session which is authenticated through PAM.
 */
#include "uniconfpamconn.h"
#include "unisecuregen.h"
#include "unipermgen.h"
#include "uniunwrapgen.h"
#include "uniconfdaemonconn.h"
#include "uninullgen.h"
#include "wvpam.h"
#include "wvaddr.h"

UniConfPamConn::UniConfPamConn(WvStream *_s, const UniConf &_root,
			       UniPermGen *perms)
    : WvStreamClone(NULL)
{
    WvPam pam("uniconfd");
    WvString rhost(*(WvIPAddr *)_s->src());
    if (pam.authenticate(rhost, "", WvString::null))
    {
	UniSecureGen *sec = new UniSecureGen(new UniUnwrapGen(_root), perms);
    
	// get the user and groups from PAM
	WvString user = pam.getuser();
	WvStringList groups;
	pam.getgroups(groups);
    
	sec->setcredentials(user, groups);
	newroot.mountgen(sec, false);
	setclone(new UniConfDaemonConn(_s, newroot));
    }
    else
    {
	_s->print("FAIL {Not Authorized}\n");
	_s->flush_then_close(1000);
    }
}
