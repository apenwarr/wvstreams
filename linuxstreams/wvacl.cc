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


WvString fix_acl(WvStringParm shortform)
{
    bool mask_found = false, mask_read = false, mask_write = false,
         mask_execute = false;

    WvStringList acl_text_entries;
    acl_text_entries.split(shortform, ",");
    WvStringList::Iter i(acl_text_entries);
    // search for mask
    for (i.rewind(); i.next(); )
    {
	if (i()[0] != 'm')
	    continue;

	mask_found = true;
	// Characters other than the permission will just be 'm' and ':',
	// so we can search the whole line.
	if (strchr(i(), 'r'))
	    mask_read = true;
	if (strchr(i(), 'w'))
	    mask_write = true;
	if (strchr(i(), 'x'))
	    mask_execute = true;
	break;
    }

    if (!mask_found)
    {
	WvString res(shortform);
	res.append(",m::rwx");
	return res;
    }

    if (mask_read && mask_write && mask_execute)
	return shortform;

    WvString newshortform("m::rwx");
    // Convert entries to their effective values.  In other words, if
    // mask_write if false, set all maskable entries' read bit to false.
    for (i.rewind(); i.next();)
    {
	WvStringList parts;
	parts.splitstrict(i(), ":");
		
	WvString this_type(parts.popstr());
	WvString this_qualifier(parts.popstr());
	WvString this_perm(parts.popstr());

	if (!this_type.len())
	    continue;

	if (this_type[0] == 'm')
	    continue;

	// If "other" entry or owning user, write 'em back as is since they
	// aren't maskable.
	if (this_type[0] == 'o' || (this_type[0] == 'u' && !this_qualifier))
	    newshortform.append(",%s", i());
	else
	{
	    WvString newperm;
	    if (mask_read && strchr(this_perm, 'r'))
		newperm.append("r");
	    if (mask_write && strchr(this_perm, 'w'))
		newperm.append("w");
	    if (mask_execute && strchr(this_perm, 'x'))
		newperm.append("x");
	    if (!newperm)
		newperm = "---";

	    newshortform.append(",%s:%s:%s", this_type, this_qualifier,
				newperm);
	}
    }

    return newshortform;
}


void get_simple_acl_permissions(WvStringParm filename,
				WvSimpleAclEntryList &acl_entries)
{
    WvLog log("ACL", WvLog::Debug5);

    struct stat st;
    if (stat(filename, &st) != 0)
	return;

#ifdef WITH_ACL
    acl_t aclchk = acl_get_file(filename, ACL_TYPE_ACCESS);
    if (aclchk)
    {
        // Library and kernel support.
        acl_free(aclchk);
	WvSimpleAclEntry *mask_entry;
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
	    case 'm':
		mask_entry = simple_entry;
		break;
	    default:
		break;
	    }

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

	    if (this_type[0] != 'm')
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
WvString get_acl_short_form(WvStringParm filename, bool get_default)
{
    WvLog log("ACL", WvLog::Debug5);
    WvString short_form;

#ifdef WITH_ACL
    acl_t acl = acl_get_file(filename, get_default ? ACL_TYPE_DEFAULT :
			                             ACL_TYPE_ACCESS);
    if (acl != NULL)
    {
	char *text = acl_to_any_text(acl, NULL, ',', TEXT_ABBREVIATE);
	short_form = fix_acl(text);
	acl_free(text);
        acl_free(acl);
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
	if (!get_default)
	{
	    short_form = build_default_acl(st.st_mode);
	    log("Using constructed ACL for %s: %s\n", filename, short_form);
	}
    }
    else
	log(WvLog::Error, "Could not get ACL entry: file %s (or what it's "
	    "linked to) does not exist.\n", filename);

    return short_form;
}


bool set_acl_permissions(WvStringParm filename, WvStringParm text_form,
			 bool set_default_too)
{
    WvLog log("ACL", WvLog::Debug3);
    struct stat st;
    if (stat(filename, &st) != 0)
    {
	log(WvLog::Error, "File %s not found.\n", filename);
	return false;
    }

    if (!S_ISDIR(st.st_mode))
	set_default_too = false;

#ifdef WITH_ACL
    acl_t acl = acl_from_text(text_form);
    if (acl_valid(acl) == 0)
    {
	int res = acl_set_file(filename, ACL_TYPE_ACCESS, acl);

	if (res == 0)
	{
	    log("Access permissions successfully changed for %s.\n",
		filename);
	    if (set_default_too)
	    {
		res = acl_set_file(filename, ACL_TYPE_DEFAULT, acl);
		if (res == 0)
		    log("Default permissions successfully changed for %s.\n",
			filename);
	    }
	}
	else
	{
	    log(WvLog::Error, 
		"Can't modify permissions for %s: ACL could not be set (%s).\n",
		filename, strerror(errno));
	}

	acl_free(acl);
	return !res;
    }
    else
	log(WvLog::Error, "Can't modify permissions for %s: ACL %s invalid "
	    "(%s).\n", filename, text_form, strerror(errno));

    if (acl) acl_free(acl);

#else
    log(WvLog::Warning, "ACL library not found.\n");
#endif

    return false;
}


bool set_acl_permission(WvStringParm filename, WvStringParm type,
                        WvString qualifier,
			bool read, bool write, bool execute, bool kill,
			bool set_default_too)
{
    WvLog log("ACL", WvLog::Debug5);
    struct stat st;
    if (stat(filename, &st) != 0)
    {
	log(WvLog::Error, "File %s not found.\n", filename);
	return false;
    }

    WvString rwx("");

    if (read)
	rwx.append("r");
    else rwx.append("-");
    if (write)
	rwx.append("w");
    else rwx.append("-");
    if (execute)
	rwx.append("x");
    else rwx.append("-");

#ifdef WITH_ACL
    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);
    if ((type.cstr()[0] == 'u' && (qualifier == WvString(pw->pw_name)
				   || qualifier == WvString(st.st_uid))) || 
	(type.cstr()[0] == 'g' && (qualifier == WvString(gr->gr_name)
				   || qualifier == WvString(st.st_gid))))
    {
	log("Setting %s::%s rather than %s:%s:%s.\n", type, rwx, type,
 	    qualifier, rwx);
	qualifier = "";
    }

    // begin building actual acl, composed of old + new
    WvString aclString("");

    WvString initacl_str(get_acl_short_form(filename));
    if (initacl_str)
    {
	WvStringList acl_entries;
	acl_entries.split(initacl_str, ",");

	WvStringList::Iter i(acl_entries);
	for (i.rewind(); i.next();)
	{
	    WvStringList parts;
	    parts.splitstrict(i(), ":");
		
	    WvString this_type(parts.popstr());
	    WvString this_qualifier(parts.popstr());

	    if (!this_type.len())
		continue;

	    // Record all the entries we aren't concerned with, and append
	    // our entry at the end.
	    if (type[0] != this_type[0] ||
		strcmp(qualifier, this_qualifier) != 0)
		aclString.append("%s\n", i());
	}
	
	if (!kill)
	    aclString.append("%s:%s:%s\n", type, qualifier, rwx);
	
	return set_acl_permissions(filename, aclString, set_default_too);
    }
#endif

    // FIXME? Use chmod() to set basic permissions if possible...

    return false;
}

