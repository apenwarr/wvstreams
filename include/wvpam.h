/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * A WvStream that authenticates with PAM.  If WvStreams is compiled without
 * PAM, it just fails.  Note that if you don't check isok, you can still read
 * and write to the stream - in particular, anything written in the
 * constructor will go through before authentication begins.
 *
 * For now, this only works for PAM modules that don't require any user
 * interaction.
 */
#ifndef __WVPAM_H
#define __WVPAM_H

#include "wvstreamclone.h"
#include "wvstringlist.h"

class WvPamData;

class WvPamStream : public WvStreamClone
{
    WvPamData *d;

public:
    
    /**
     * Require PAM authentication for the cloned stream.  name is the PAM
     * service name.  success and fail are optional messages to write to the
     * cloned stream on success or failure.
     */
    WvPamStream(WvStream *cloned, WvStringParm name, WvStringParm success =
            WvString::null, WvStringParm fail = WvString::null);
    virtual ~WvPamStream();

    /** Goes not ok if authentication fails */
    virtual bool isok() const;

    /** Return the user */
    WvString getuser() const;

    /** Return the list of groups */
    void getgroups(WvStringList &l) const;

private:

    /**
     * Log the result of the last PAM step, based on the pam_status flag,and
     * write a failure message to the cloned stream on error.  step is the
     * name to use in the log message.  Returns true if the last step
     * succeeded, false if it failed.
     */
    bool check_pam_status(WvStringParm step);
};

#endif // __WVPAM_H
