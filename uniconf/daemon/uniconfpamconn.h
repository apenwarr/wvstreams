/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Manages a UniConf daemon session, first authenticating with PAM.
 */
#ifndef __UNICONFPAMCONN_H
#define __UNICONFPAMCONN_H

#include "wvautoconf.h"
#include "uniconfdaemonconn.h"

#ifdef HAVE_SECURITY_PAM_APPL_H

#include <security/pam_appl.h>

class UniConfPamConn : public UniConfDaemonConn 
{
    pam_handle_t *pamh;
    int pam_status;
    bool session_exists;

public:
    UniConfPamConn(WvStream *s, const UniConf &root);
    virtual ~UniConfPamConn();

    /** Do the PAM negotiation */
    virtual void startup();

    /** The service name is "uniconfdaemon" */
    static const char *PAM_SERVICE_NAME;

    /** Goes not ok if authentication fails */
    virtual bool isok() const;

    /** PAM user communication function - always fails */
    static int noconv(int num_msg, const struct pam_message **msgm,
        struct pam_response **response, void *userdata);

protected:

    /**
     * Log the result of the last PAM step, based on the pam_status flag,and
     * write FAIL to the client conn on error.  step is the name to use in the
     * log message.  Returns true if the last step succeeded, false if it
     * failed.
     */
    bool check_pam_status(WvStringParm step);

    /** Override request handlers to require authentication */
    virtual void do_get(const UniConfKey &key);
    virtual void do_set(const UniConfKey &key, WvStringParm value);
    virtual void do_remove(const UniConfKey &key);
    virtual void do_subtree(const UniConfKey &key);
    virtual void do_haschildren(const UniConfKey &key);
};

#else

typedef UniConfDaemonConn UniConfPamConn;

#endif // HAVE_SECURITY_PAM_APPL_H

#endif // __UNICONFPAMCONN_H
