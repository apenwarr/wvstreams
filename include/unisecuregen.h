/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 */

#ifndef __UNISECUREGEN_H
#define __UNISECUREGEN_H

#include "unifiltergen.h"
#include "unipermgen.h"
#include "wvstring.h"
#include "wvstringlist.h"

/**
 * UniSecureGen wraps a given generator and checks permissions (using a
 * Unix-style scheme) before responding to requests.  The permissions for
 * generator gen are stored in a parallel tree, perms.
 *
 * It is up to the caller to ensure that the UniPermGen is itself secure.
 * (The easiest way is probably to back it with an ini file in a secure
 * directory.)  Note that there is a race condition here: there is no locking
 * to be sure that the UniPermGen is not altered while a key is being looked
 * up.  This could come into play, for instance, if the exec permission is
 * removed from a subtree while the UniSecureGen is in the middle of
 * drilldown().
 *
 * UniSecureGen cannot be created with a moniker due to its extra methods.
 * Instead, just create one with new and mount it with UniConf::mountgen.
 */
class UniSecureGen : public UniFilterGen
{
    UniPermGen *perms;
    UniPermGen::Credentials cred;

public:
    UniSecureGen(UniConfGen *_gen, UniPermGen *_perms);
    UniSecureGen(WvStringParm moniker, UniPermGen *_perms);

    void setcredentials(const UniPermGen::Credentials &_cred);
    void setcredentials(WvStringParm user, const WvStringList &groups);
    
    /** Overridden methods */
    virtual WvString get(const UniConfKey &key);
    virtual bool exists(const UniConfKey &key);
    virtual void set(const UniConfKey &key, WvStringParm value);
    virtual bool haschildren(const UniConfKey &key);
    virtual Iter *iterator(const UniConfKey &key);

private:

    /** Check the perms tree for the given permission */
    bool findperm(const UniConfKey &key, UniPermGen::Type type);

    /**
     * Search the fullpath of key to be sure we are able to view each
     * element.  If we ever find a missing exec permission, return false
     * immediately.
     */
    bool drilldown(const UniConfKey &key);

    /** Override gencallback to check for permissions before sending a delta */
    virtual void gencallback(const UniConfKey &key, WvStringParm value,
            void *userdata);
};


#endif // __UNISECUREGEN_H
