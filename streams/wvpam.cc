/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * A WvStream that authenticates with PAM before allowing any reading or
 * writing.  See wvpam.h.
 */
#include "wvlog.h"
#include "wvpam.h"
#include "wvautoconf.h"

// If PAM not installed at compile time, stub this out
#ifndef HAVE_SECURITY_PAM_APPL_H

WvPam::WvPam(WvStringParm _appname)
    : log("PAM Auth", WvLog::Info), appname(_appname)
{
    err.seterr("Compiled without PAM Support!\n");
}
         
          
WvPam::WvPam(WvStringParm _appname, WvStringParm rhost,
	     WvStringParm user, WvStringParm password) 
    : log("PAM Auth", WvLog::Info), appname(_appname)  
{
    err.seterr("Compiled without PAM Support!\n");
}


WvPam::~WvPam()
{
}

bool WvPam::authenticate(WvStringParm rhost, WvStringParm user, WvStringParm password)
{
    return false;
}

WvString WvPam::getuser() const
{
    return WvString::null;
}
 
 
void WvPam::getgroups(WvStringList &l) const
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
    WvString failmsg, user;
    WvStringList groups;

    WvPamData()
	: pamh(NULL), status(PAM_SUCCESS), user("")
	{ }
    
    WvPamData(WvStringParm _failmsg)
	: pamh(NULL), status(PAM_SUCCESS), failmsg(_failmsg)
	{ }
};


/** noconv: null PAM conversation function */
#if HAVE_BROKEN_PAM
int noconv(int num_msg, struct pam_message **msgm,
        struct pam_response **response, void *userdata)
#else
int noconv(int num_msg, const struct pam_message **msgm,
        struct pam_response **response, void *userdata)
#endif
{
    // if you need to ask things, it won't work
    return PAM_CONV_ERR;
}


// The password gets passed in from userdata, and we simply echo it back
// out in the response... *sigh* This is because pam expects this function
// to actually interact with the user, and get their password.
#if HAVE_BROKEN_PAM
static int passconv(int num_msg, struct pam_message **msgm,
        struct pam_response **response, void *userdata)
#else
static int passconv(int num_msg, const struct pam_message **msgm,
        struct pam_response **response, void *userdata)
#endif
{
    struct pam_response *password_echo;
    
    password_echo = (struct pam_response *)calloc(num_msg,
                                                  sizeof(struct pam_response));
    password_echo->resp = (char *)userdata;
    password_echo->resp_retcode = 0;
    
    *response = password_echo;
    
    return PAM_SUCCESS;
}

WvPam::WvPam(WvStringParm _appname)
    : log("PAM Auth", WvLog::Info), appname(_appname)
{
    init();
}
 

WvPam::WvPam(WvStringParm _appname, WvStringParm rhost,
             WvStringParm user, WvStringParm password)
    : log("PAM Auth", WvLog::Info), appname(_appname)
{
    if (init())
        authenticate(rhost, user, password);
}

WvPam::~WvPam()
{
    log(WvLog::Debug2, "Shutting down PAM Session for: %s\n", appname);
    if (d->status == PAM_SUCCESS)
        pam_close_session(d->pamh, 0);
    pam_end(d->pamh, d->status);
    d->groups.zap();
    delete d;       
}
 
 
bool WvPam::init()
{
    d = new WvPamData();
    log(WvLog::Debug2, "Starting up PAM Session for: %s\n", appname);
    err.seterr("Not yet authenticated...");
    
    struct pam_conv c;
    c.conv = noconv;  
    c.appdata_ptr = NULL;

    d->pamh = NULL;
    d->status = pam_start(appname, d->user, &c, &d->pamh);
    if (check_pam_status("pam_start")) return true;
    return false;
}

bool WvPam::authenticate(WvStringParm rhost, WvStringParm user, WvStringParm password)
{
    if (!!rhost)
    {
        d->status = pam_set_item(d->pamh, PAM_RHOST, rhost);
        if (!check_pam_status("rhost setup")) return false; 
    }

    if (!!user)
    {
        d->user = user;
        d->status = pam_set_item(d->pamh, PAM_USER, user);
        if (!check_pam_status("user setup")) return false;
    }

    if (!!password)
    {
        struct pam_conv c;
        c.conv = passconv;
        c.appdata_ptr = strdup(password);
        d->status = pam_set_item(d->pamh, PAM_CONV, &c);
        if (!check_pam_status("conversation setup")) return false;
        
        d->status = pam_set_item(d->pamh, PAM_AUTHTOK, password);
        if (!check_pam_status("password setup")) return false;   
    }

#if HAVE_BROKEN_PAM
    void *x = NULL;
#else
    const void *x = NULL;
#endif
    d->status = pam_get_item(d->pamh, PAM_USER, &x);
    if (!check_pam_status("get username")) return false;
    d->user = (const char *)x;

    log("Starting Authentication for %s@%s\n", d->user, rhost);

    d->status = pam_authenticate(d->pamh, PAM_DISALLOW_NULL_AUTHTOK | PAM_SILENT);
    if (!check_pam_status("authentication")) return false;

    d->status = pam_acct_mgmt(d->pamh, PAM_DISALLOW_NULL_AUTHTOK | PAM_SILENT);
    if (!check_pam_status("account management")) return false;
    
    d->status = pam_setcred(d->pamh, PAM_ESTABLISH_CRED);
    if (!check_pam_status("credentials")) return false;  

    d->status = pam_open_session(d->pamh, 0);
    if (!check_pam_status("session open")) return false;

    // Grab the current user name (now that we've authenticated)
    if (!d->user)
    {
        const void *x = NULL;
        d->status = pam_get_item(d->pamh, PAM_USER, &x);
        if (!check_pam_status("get username")) return false;
        d->user = (const char *)x;        
    }
    log("Session open as user '%s'\n", d->user);
    
    // If we made it here, we're clear of everything, and we can go
    // to a no error status.
    err.noerr();
    
    return true;
}


bool WvPam::check_pam_status(WvStringParm s)
{
    if (d->status == PAM_SUCCESS)
    {
        log(WvLog::Debug2, "PAM %s succeeded.\n", s);
        return true;
    }
    else
    {   
        WvString msg("PAM %s failed: %s\n", s, pam_strerror(d->pamh, d->status));
        log(WvLog::Info, msg);
        err.seterr(msg);
        d->user = WvString::null;
        d->groups.zap();
        return false;   
    }
}
 
 
WvString WvPam::getuser() const
{
    return d->user;
}
 
 
void WvPam::getgroups(WvStringList &l) const
{
    assert(l.isempty());

    // Cache after the first time...
    if (d->groups.isempty())
    {
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
     
    WvStringList::Iter i(d->groups);
    for (i.rewind(); i.next(); )
        l.append(new WvString(*i), true);
}






#endif // HAVE_SECURITY_PAM_APPL_H
