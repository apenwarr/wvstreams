/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Manages a UniConf daemon session, first authenticating with PAM.
 */

#include "uniconfpamconn.h"

#ifdef HAVE_SECURITY_PAM_APPL_H

#include <sys/types.h>
#include <pwd.h>

#include "wvtclstring.h"
#include "wvaddr.h"

const int PAM_START_STEP = 1;
const int PAM_AUTH_STEP = 2;
const int PAM_ACCT_STEP = 3;
const int PAM_SETCRED_STEP = 4;
const int PAM_SESSION_STEP = 5;


const char *UniConfPamConn::PAM_SERVICE_NAME = "uniconfdaemon";


UniConfPamConn::UniConfPamConn(WvStream *s, const UniConf &root) :
    UniConfDaemonConn(s, root),
    pamh(NULL), pam_status(PAM_SUCCESS), session_exists(false)
{
}


UniConfPamConn::~UniConfPamConn()
{
    if (session_exists)
        pam_close_session(pamh, 0);
    pam_end(pamh, pam_status);
}


void UniConfPamConn::startup()
{
    // create the conv structure
    struct pam_conv c;
    c.conv = UniConfPamConn::noconv;
    c.appdata_ptr = NULL;

    // find the user
    struct passwd *pw = getpwuid(getuid());
    assert(pw);
    
    // find the host and port
    WvString rhost(*src());
    
    pam_status = pam_start(PAM_SERVICE_NAME, pw->pw_name, &c, &pamh);
    if (!check_pam_status("startup")) return;

    pam_status = pam_set_item(pamh, PAM_RHOST, rhost);
    if (!check_pam_status("environment setup")) return;

    pam_status = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
    if (!check_pam_status("authentication")) return;

    pam_status = pam_setcred(pamh, PAM_ESTABLISH_CRED);
    if (!check_pam_status("credentials")) return;
        
    pam_status = pam_open_session(pamh, 0);
    if (!check_pam_status("session open")) return;

    session_exists = true;

    UniConfDaemonConn::startup();
}


bool UniConfPamConn::check_pam_status(WvStringParm s)
{
    if (pam_status == PAM_SUCCESS)
    {
        log(WvLog::Debug2, "PAM %s succeeded\n", s);
        return true;
    }
    else
    {
        log(WvLog::Debug2, "PAM %s FAILED: %s\n", s, pam_status);
        writefail(wvtcl_escape("Authorization failed"));
        return false;
    }
}


bool UniConfPamConn::isok() const
{
    return ((pam_status == PAM_SUCCESS || pam_status == PAM_INCOMPLETE)
            && UniConfDaemonConn::isok());
}


int UniConfPamConn::noconv(int num_msg, const struct pam_message **msgm,
        struct pam_response **response, void *userdata)
{
    // if you need to ask things, it won't work
    return PAM_CONV_ERR;
}


void UniConfPamConn::do_get(const UniConfKey &key)
{
    if (session_exists)
        UniConfDaemonConn::do_get(key);
    else
        writefail("Not authenticated");
}


void UniConfPamConn::do_set(const UniConfKey &key, WvStringParm value)
{
    if (session_exists)
        UniConfDaemonConn::do_set(key, value);
    // don't write fail because this doesn't expect a response
}

void UniConfPamConn::do_remove(const UniConfKey &key)
{
    if (session_exists)
        UniConfDaemonConn::do_remove(key);
    // don't write fail because this doesn't expect a response
}


void UniConfPamConn::do_subtree(const UniConfKey &key)
{
    if (session_exists)
        UniConfDaemonConn::do_subtree(key);
    else
        writefail("Not authenticated");
}


void UniConfPamConn::do_haschildren(const UniConfKey &key)
{
    if (session_exists)
        UniConfDaemonConn::do_haschildren(key);
    else
        writefail("Not authenticated");
}

#endif // HAVE_SECURITY_PAM_APPL_H
