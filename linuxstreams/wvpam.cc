/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * A WvStream that authenticates with PAM before allowing any reading or
 * writing.  If WvStreams is compiled without PAM, it just fails.
 *
 * For now, this only works for PAM modules that don't require any user
 * interaction.  (Most notably Ssoya.)
 */

#include "wvlog.h"
#include "wvpam.h"
#include "wvautoconf.h"

// If PAM not installed at compile time, stub this out
#ifndef HAVE_SECURITY_PAM_APPL_H

WvPamStream::WvPamStream(WvStream *cloned, WvStringParm name, WvStringParm
        success, WvStringParm fail) :
    WvStreamClone(cloned)
{
    WvLog log("WvPamStream", WvLog::Warning);
    log("Compiled without PAM support\n");
    if (cloned && !!fail)
        cloned->write(fail.cstr(), fail.len());
}

WvPamStream::~WvPamStream()
{
}

bool WvPamStream::isok() const
{
    return false;
}

bool WvPamStream::check_pam_status(WvStringParm step)
{
    return false;
}

WvString WvPamStream::getuser() const
{
    return WvString::null;
}

void WvPamStream::getgroups(WvStringList &l) const
{
}

#else   // HAVE_SECURITY_PAM_APPL_H

#include <security/pam_appl.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include "wvaddr.h"


class WvPamData
{
public:
    pam_handle_t *pamh;
    int status;
    WvStringParm failmsg;
    WvString user;
    WvStringList groups;

    WvPamData(WvStringParm _failmsg) :
        pamh(NULL), status(PAM_SUCCESS), failmsg(_failmsg)
    { }
};


/** noconv: null PAM conversation function */
int noconv(int num_msg, const struct pam_message **msgm,
        struct pam_response **response, void *userdata)
{
    // if you need to ask things, it won't work
    return PAM_CONV_ERR;
}


// FIXME: pam calls could block.  Should fork off a subproc for this.  On the
// other hand, the stream's blocked anyway until pam comes back, so do we
// really care?
WvPamStream::WvPamStream(WvStream *cloned, WvStringParm name,
        WvStringParm successmsg, WvStringParm failmsg) :
    WvStreamClone(cloned),
    d(new WvPamData(failmsg))
{
    // create the conv structure
    struct pam_conv c;
    c.conv = noconv;
    c.appdata_ptr = NULL;

    // find the user
    struct passwd *pw = getpwuid(getuid());
    assert(pw);
    d->user = pw->pw_name;

    // find the host and port
    WvString rhost(*src());
 
    // authenticate through PAM
    d->status = pam_start(name, d->user, &c, &d->pamh);
    if (!check_pam_status("startup")) return;

    d->status = pam_set_item(d->pamh, PAM_RHOST, rhost);
    if (!check_pam_status("environment setup")) return;

    d->status = pam_authenticate(d->pamh, PAM_DISALLOW_NULL_AUTHTOK);
    if (!check_pam_status("authentication")) return;

    d->status = pam_setcred(d->pamh, PAM_ESTABLISH_CRED);
    if (!check_pam_status("credentials")) return;

    d->status = pam_open_session(d->pamh, 0);
    if (!check_pam_status("session open")) return;

    // write the success message if necessary
    if (cloned && !!successmsg) cloned->write(successmsg.cstr(), successmsg.len());
    
    // get the groups
    setgrent();
    struct group *gr;
    while ((gr = getgrent()))
    {
        for (char **i = gr->gr_mem; *i != NULL; i++)
        {
            if (strcmp(*i, d->user))
            {
                d->groups.append(new WvString(gr->gr_name), true);
                break;
            }
        }
    }
    endgrent();
}


WvPamStream::~WvPamStream()
{
    if (d->status == PAM_SUCCESS)
        pam_close_session(d->pamh, 0);
    pam_end(d->pamh, d->status);
}


bool WvPamStream::isok() const
{
    return (d->status == PAM_SUCCESS && WvStreamClone::isok());
}


bool WvPamStream::check_pam_status(WvStringParm s)
{
    WvLog log("WvPamStream", WvLog::Debug2);
    if (d->status == PAM_SUCCESS)
    {
        log("PAM %s succeeded\n", s);
        return true;
    }
    else
    {
        log("PAM %s FAILED: %s\n", s, d->status);
        if (cloned && !!d->failmsg) cloned->write(d->failmsg.cstr(), d->failmsg.len());
        d->user = WvString::null;
        d->groups.zap();
        return false;
    }
}


WvString WvPamStream::getuser() const
{
    return d->user;
}


void WvPamStream::getgroups(WvStringList &l) const
{
    assert(l.isempty());
    WvStringList::Iter i(d->groups);
    for (i.rewind(); i.next(); )
        l.append(new WvString(*i), true);
}


#endif // HAVE_SECURITY_PAM_APPL_H
