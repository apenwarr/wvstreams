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
    enum WvSimpleAclEntryType { AclUser = 0, AclGroup, AclOther } type;
    bool read;
    bool write;
    bool execute;
    bool owner;  /// true if owning group or user

    /// For type user or group, this is the username/groupname; otherwise blank.
    WvString name;
};
DeclareWvList(WvSimpleAclEntry);

int wvsimpleaclentry_sort(const WvSimpleAclEntry *a, const WvSimpleAclEntry *b);

/// Prints log messages indicating if we have library and/or kernel support.
void acl_check();

/** Returns a NITI-compliant ACL short text form of 'oldacl'.
 * This involves ensuring that the mask is "rwx".  If it isn't, change user
 * and group entries to their effective entries and set mask to "rwx".
 */
WvString fix_acl(WvStringParm oldacl);

/// Assemble a default, short-form ACL from 'mode'.
WvString build_default_acl(mode_t mode);

/** Returns the short form of the ACL for 'filename'.
 * If 'default' is true, returns the ACL_TYPE_DEFAULT list.
 * If false, returns ACL_TYPE_ACCESS list.
 */
WvString get_acl_short_form(WvStringParm filename, bool get_default = false);

/// Populates 'acl_entries' with simple entries.
void get_simple_acl_permissions(WvStringParm filename, WvSimpleAclEntryList
				&acl_entries);

/// Set one or more ACL entries through standard short or long text form.
bool set_acl_permissions(WvStringParm filename, WvStringParm text_form,
			 bool set_default_too);

/// Set one ACL entry with individual parameters.
bool set_acl_permission(WvStringParm filename, WvStringParm type,
                        WvString qualifier,
			bool read, bool write, bool execute,
			bool kill = false, bool set_default_too = false);

#endif // __WVACL_H
