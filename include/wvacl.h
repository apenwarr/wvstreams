/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * Functions (essentially wrappers) for access control lists (ACLs).
 */
#ifndef __WVACL_H
#define __WVACL_H

#include "wvstring.h"
#include "wvlinklist.h"

/// Abstraction for ACL stuff.
struct WvSimpleAclEntry
{
    enum WvSimpleAclEntryType { AclUser, AclGroup, AclOther } type;
    bool read;
    bool write;
    bool execute;

    /// For type user or group, this is the username/groupname; otherwise blank.
    WvString name;
};
DeclareWvList(WvSimpleAclEntry);

/// Assemble a default, short-form ACL from 'mode'.
WvString build_default_acl(mode_t mode);

/// Returns the short form of the ACL for 'filename'.
WvString get_acl_short_form(WvStringParm filename);

/// Populates 'acl_entries' with simple entries.
void get_simple_acl_permissions(WvStringParm filename, WvSimpleAclEntryList
				&acl_entries);

/// Set one or more ACL entries through standard short or long text form.
bool set_acl_permissions(WvStringParm filename, WvStringParm text_form);

/// Set one ACL entry with individual parameters.
bool set_acl_permission(WvStringParm filename, WvStringParm type,
                        WvStringParm qualifier,
			bool read, bool write, bool execute);

#endif // __WVACL_H
