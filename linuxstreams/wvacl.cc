/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * Handles access control lists (ACLs).
 */
#include "wvautoconf.h"
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

int wvsimpleaclentry_sort(const WvSimpleAclEntry *a, const WvSimpleAclEntry *b)
{
    if (a->type == b->type)
	return strcasecmp(a->name, b->name);

    return a->type - b->type;
}


void acl_check()
{
    WvLog log("ACL", WvLog::Info);

#ifndef WITH_ACL
    log("No ACL library support detected.  Not checking for kernel "
        "support.\n");
#else
    log("ACL library support detected.\n");
    acl_t aclchk = acl_get_file("/", ACL_TYPE_ACCESS);
    if (aclchk)
    {
        log("ACL kernel support detected.\n");
        acl_free(aclchk);
    }
    else
        log("No ACL kernel support detected.\n");
#endif
}


void get_simple_acl_permissions(WvStringParm filename,
				WvSimpleAclEntryList &acl_entries)
{
    WvLog log("ACL", WvLog::Info);

    struct stat st;
    if (stat(filename, &st) != 0)
	return;

#ifdef WITH_ACL
    acl_t aclchk = acl_get_file(filename, ACL_TYPE_ACCESS);
    if (aclchk)
    {
        // Library and kernel support.
        acl_free(aclchk);
        WvString short_form(get_acl_short_form(filename));
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
	    simple_entry->owner = false;
	    if (!!this_qualifier)
		simple_entry->name = this_qualifier;

	    switch (this_type[0])
	    {
	    case 'u':
		simple_entry->type = WvSimpleAclEntry::AclUser;
		if (!this_qualifier && (pw = getpwuid(st.st_uid))) // owner
		{
		    simple_entry->owner = true;
       		    simple_entry->name = pw->pw_name;
		}
		break;
	    case 'g':
		simple_entry->type = WvSimpleAclEntry::AclGroup;
		if (!this_qualifier && (gr = getgrgid(st.st_gid))) // owner
		{
		    simple_entry->owner = true;
		    simple_entry->name = gr->gr_name;
		}
		break;
	    case 'o':
		simple_entry->type = WvSimpleAclEntry::AclOther;
		break;
	    default:   // don't care about mask
		delete simple_entry;
		simple_entry = NULL;
		break;
	    }

	    if (!simple_entry)
		continue;

	    if (strchr(this_permission, 'r'))
		simple_entry->read = true;
	    else
		simple_entry->read = false;
	    if (strchr(this_permission, 'w'))
		simple_entry->write = true;
	    else
		simple_entry->write = false;
	    if (strchr(this_permission, 'x'))
		simple_entry->execute = true;
	    else
		simple_entry->execute = false;

	    log("name %s type %s read %s write %s execute %s\n",
		simple_entry->name, simple_entry->type, simple_entry->read,
		simple_entry->write, simple_entry->execute);

	    acl_entries.append(simple_entry, true);
	}

        return;
    }
#endif

    // No ACL support.
    // Fill in default values--don't DoubleTranslate through
    // build_default_acl() though.
    
    // owners
    WvSimpleAclEntry *acl = new WvSimpleAclEntry;
    struct passwd *pw = getpwuid(st.st_uid);
    if (!pw || !pw->pw_name || !strlen(pw->pw_name))
        acl->name = WvString(st.st_uid);
    else
        acl->name = pw->pw_name;
    acl->type = WvSimpleAclEntry::AclUser;
    acl->read = st.st_mode & S_IRUSR;
    acl->write = st.st_mode & S_IWUSR;
    acl->execute = st.st_mode & S_IXUSR;
    acl_entries.append(acl, true);

    acl = new WvSimpleAclEntry;
    struct group *gr = getgrgid(st.st_gid);
    if (!gr || !gr->gr_name || !strlen(gr->gr_name))
        acl->name = WvString(st.st_gid);
    else
        acl->name = gr->gr_name;
    acl->type = WvSimpleAclEntry::AclGroup;
    acl->read = st.st_mode & S_IRGRP;
    acl->write = st.st_mode & S_IWGRP;
    acl->execute = st.st_mode & S_IXGRP;
    acl_entries.append(acl, true);

    // other
    acl = new WvSimpleAclEntry;
    acl->type = WvSimpleAclEntry::AclOther;
    acl->read = st.st_mode & S_IROTH;
    acl->write = st.st_mode & S_IWOTH;
    acl->execute = st.st_mode & S_IXOTH;
    acl_entries.append(acl, true);
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
    // c) ACL isn't in the kernel.
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
                        WvString qualifier,
			bool read, bool write, bool execute, bool kill)
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

    acl_t initacl = acl_get_file(filename, ACL_TYPE_ACCESS);

    if (initacl)
    {
	struct stat st;
	if (stat(filename, &st) != 0)
	{
	    // Probably not necessary...
	    log(WvLog::Error, "Found ACL but not stat() for %s.\n", filename);
	    return false;
	}

	struct passwd *pw = getpwuid(st.st_uid);
	struct group *gr = getgrgid(st.st_gid);
	if ((type.cstr()[0] == 'u' && qualifier == WvString(pw->pw_name)) || 
	    (type.cstr()[0] == 'g' && qualifier == WvString(gr->gr_name)))
	{
	    log("Setting %s::%s rather than %s:%s:%s.\n", type, rwx, type,
		qualifier, rwx);
	    qualifier = "";
	}

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
	
	if (!kill)
	    aclString.append("%s:%s:%s\n", type, qualifier, rwx);
	
	// Add a default mask (max permissions for groups & non-owning users).
	// This is only required if we set permissions for non-owning users or
	// groups, but it doesn't hurt.
	if (!mask_exists)
	    aclString.append("mask::rwx\n");

	return set_acl_permissions(filename, aclString);
    }

#endif

    // FIXME? Use chmod() to set basic permissions if possible...

    return false;
}

