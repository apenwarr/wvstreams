/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * Handles access control lists (ACLs).
 */

#include "wvacl.h"
#include "wvstringlist.h"
#include "wvlog.h"
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>

#ifdef WITH_ACL
#include <acl.h>
#include <acl/libacl.h>
#endif

void get_simple_acl_permissions(WvStringParm filename, WvSimpleAclEntryList
				&acl_entries)
{
    struct stat st;
    if (stat(filename, &st) != 0)
	return;

#ifdef WITH_ACL
    WvString short_form(get_acl_short_form(filename));
    if (!!short_form)
    {
	struct passwd *pw;
	struct group *gr;

	WvStringList acl_text_entries;
	acl_text_entries.split(short_form, ",");
	WvStringList::Iter i(acl_text_entries);
	for (i.rewind(); i.next(); )
	{
	    WvStringList this_entry;
	    this_entry.splitstrict(i(), ":");
	    WvString this_type(this_entry.popstr());
	    WvString this_qualifier(this_entry.popstr());
	    WvString this_permission(this_entry.popstr());

	    WvSimpleAclEntry *simple_entry = new WvSimpleAclEntry;
	    if (!!this_qualifier)
		simple_entry->name = this_qualifier;

	    switch (this_type[0])
	    {
	    case 'u':
		simple_entry->type = WvSimpleAclEntry::AclUser;
		if (!this_qualifier && (pw = getpwuid(st.st_uid))) // owner
       		    simple_entry->name = pw->pw_name;
		break;
	    case 'g':
		simple_entry->type = WvSimpleAclEntry::AclGroup;
		if (!this_qualifier && (gr = getgrgid(st.st_gid))) // owner
		    simple_entry->name = gr->gr_name;
		break;
	    case 'o':
		simple_entry->type = WvSimpleAclEntry::AclOther;
		break;
	    default:   // don't care about mask
		break;
	    }

	    if (strchr(this_permission, 'r'))
		simple_entry->read = true;
	    if (strchr(this_permission, 'w'))
		simple_entry->write = true;
	    if (strchr(this_permission, 'x'))
		simple_entry->execute = true;

	    acl_entries.append(simple_entry, true);
	}
	return;
    }
#endif

    // No ACL support.
    // Fill in default values
}


WvString build_default_acl(mode_t mode)
{
    WvString short_form("u::%s%s%s,g::%s%s%s,o::%s%s%s",
			mode & S_IRUSR ? "r" : "-",
			mode & S_IWUSR ? "w" : "-",
			mode & S_IXUSR ? "x" : "-",
			mode & S_IRGRP ? "r" : "-",
			mode & S_IWGRP ? "w" : "-",
			mode & S_IXGRP ? "x" : "-",
			mode & S_IROTH ? "r" : "-",
			mode & S_IWOTH ? "w" : "-",
			mode & S_IXOTH ? "x" : "-");
    return short_form;
}


/** There is, surprisingly, apparently no library function to actually get the
 * short form of an ACL.  This function creates one.
 */
WvString get_acl_short_form(WvStringParm filename)
{
    WvLog log("ACL", WvLog::Debug);
    log("Getting short form ACL for %s\n", filename);
    WvString short_form;

#ifdef WITH_ACL
    acl_t acl = acl_get_file(filename, ACL_TYPE_ACCESS);
    if (acl != NULL)
    {
	char *text = acl_to_any_text(acl, NULL, ',', TEXT_ABBREVIATE);
	short_form = text;
	acl_free(text);
	log("Successfully retrieved ACL for %s: %s\n", filename, short_form);
	return short_form;
    }
#endif

    // a) the file doesn't exist or
    // b) ACL libraries are missing or
    // b) ACL isn't in the kernel.
    // Return empty string if a) or default if b) or c).

    struct stat st;
    if (stat(filename, &st) == 0)
    {
	short_form = build_default_acl(st.st_mode);
	log("Using constructed ACL for %s: %s\n", filename, short_form);
    }
    else
	log(WvLog::Error, "Could not get ACL entry: file %s does not "
	    "exist.\n", filename);

    return short_form;
}


bool set_acl_permissions(WvStringParm filename, WvStringParm text_form)
{
    WvLog log("ACL", WvLog::Debug);
#ifdef WITH_ACL
    acl_t acl = acl_from_text(text_form);
    if (acl_valid(acl) == 0)
    {
	int res = acl_set_file(filename, ACL_TYPE_ACCESS, acl);
	acl_free(acl);

	if (res == 0)
	{
	    log(WvLog::Debug, "Permissions successfully changed.\n");
	    return true;
	}

	log(WvLog::Error, 
	    "Can't modify permissions for %s: ACL could not be set.\n",
	    filename);
    }
    else
	log(WvLog::Error, "Can't modify permissions for %s: ACL invalid.\n",
	    filename);
#endif

    return false;
}


bool set_acl_permission(WvStringParm filename, WvStringParm type,
                        WvStringParm qualifier,
			bool read, bool write, bool execute)
{
    WvLog log("ACL", WvLog::Debug);
#ifdef WITH_ACL
    WvString rwx("");

    if (read)
	rwx.append("r");
    if (write)
	rwx.append("w");
    if (execute)
	rwx.append("x");

    log("Trying to set permission %s, type %s, qualifier %s, on file %s.\n",
	rwx, type, qualifier, filename);

    acl_t initacl = acl_get_file(filename, ACL_TYPE_ACCESS);

    if (initacl)
    {
	// begin building actual acl, composed of old + new
	WvString aclString("");

	char *initacl_text = acl_to_text(initacl, NULL);
	WvString initacl_str(initacl_text);

	acl_free(initacl_text);
	acl_free(initacl);

	bool mask_exists = false;
	if (strcmp("mask", type) == 0)
	    mask_exists = true;  // don't need to create default mask

	WvStringList acl_entries;
	acl_entries.split(initacl_str, "\n");

	WvStringList::Iter i(acl_entries);
	for (i.rewind(); i.next();)
	{
	    WvStringList parts;
	    parts.splitstrict(i(), ":");

	    WvString this_type(parts.popstr());
	    WvString this_qualifier(parts.popstr());

	    if (!this_type.len())
		continue;

	    if (!mask_exists && (strcmp("mask", this_type) == 0))
		mask_exists = true;

	    // Record all the entries we aren't concerned with, and append
	    // our entry at the end.
	    if (strcmp(type, this_type) != 0 ||
		strcmp(qualifier, this_qualifier) != 0)
		aclString.append("%s\n", i());
	}
	
	aclString.append("%s:%s:%s\n", type, qualifier, rwx);
	
	// Add a default mask (max permissions for groups & non-owning users).
	// This is only required if we set permissions for non-owning users or
	// groups, but it doesn't hurt.
	if (!mask_exists)
	    aclString.append("mask::rwx\n");

	return set_acl_permissions(filename, aclString);
    }

    log(WvLog::Error,
	"Can't modify permissions for %s: could not get ACL entry.\n", filename);
#endif

    return false;
}

