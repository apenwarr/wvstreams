#include "wvacl.h"
#include "wvfile.h"
#include "wvtest.h"
#include "wvlogrcv.h"
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>

WvLogConsole logrcv(1);

static mode_t default_mode = 0754;
static WvString default_acl = "u::rwx,g::r-x,o::r--";

void create_foo(WvStringParm testfn, WvString &username, WvString &groupname,
                bool dir)
{
    acl_check();
    ::chmod(testfn, 0777);
    if (::unlink(testfn) != 0 && errno == EISDIR)
        ::rmdir(testfn);

    if (dir)
        ::mkdir(testfn, default_mode);
    else
        WvFile tmp(testfn, O_WRONLY | O_CREAT | O_TRUNC, default_mode);

    // On some filesystems (notably reiserfs), a new file's group is inherited
    // from its directory.  We use chown() here to set it to a known value.
    // We could, of course, figure this out from stat(), but that's what
    // get_simple_acl_permissions() does, so it wouldn't be much of a test.
    struct passwd *pw = getpwuid(getuid());
    struct group *gr = getgrgid(pw->pw_gid);
    chown(testfn, getuid(), pw->pw_gid);
    
    if (!pw->pw_name)
        username = getuid();
    else
	username = pw->pw_name;

    if (!gr || !gr->gr_name)
	groupname = pw->pw_gid;
    else
	groupname = gr->gr_name;
}


void create_file(WvStringParm testfn, WvString &username, WvString &groupname)
{
    create_foo(testfn, username, groupname, false);
}


void create_dir(WvStringParm testfn, WvString &username, WvString &groupname)
{
    create_foo(testfn, username, groupname, true);
}


WVTEST_MAIN("default get_simple_acl_permissions()")
{
    if (!acl_check()) return;
    
    WvString testfn("wvacltest.tmp"), username, groupname;

    create_file(testfn, username, groupname);

    WvSimpleAclEntryList acls;
    get_simple_acl_permissions(testfn, acls);
 
    bool u_chk = false, g_chk = false, o_chk = false;
    WvSimpleAclEntryList::Iter i(acls);
    for (i.rewind(); i.next(); )
    {
	switch (i().type)
	{
	case WvSimpleAclEntry::AclUser:
            printf("user entry for %s\n", i().name.cstr());
	    WVPASS(!u_chk);
	    WVPASSEQ(i().name, username);
	    WVPASS(i().read);
	    WVPASS(i().write);
	    WVPASS(i().execute);
	    if (i().name == username)
		u_chk = true;
	    break;

	case WvSimpleAclEntry::AclGroup:
            printf("group entry for %s\n", i().name.cstr());
	    WVPASS(!g_chk);
	    WVPASSEQ(i().name, groupname);
	    WVPASS(i().read);
	    WVPASS(!i().write);
	    WVPASS(i().execute);
	    if (i().name == groupname)
		g_chk = true;
	    break;

	case WvSimpleAclEntry::AclOther:
            printf("other entry\n");
	    WVPASS(!o_chk);
	    WVPASS(!i().name.len());
	    WVPASS(i().read);
	    WVPASS(!i().write);
	    WVPASS(!i().execute);
	    o_chk = true;
	    break;
	}
    }
    
    WVPASS(u_chk);
    WVPASS(g_chk);
    WVPASS(o_chk);
}

WVTEST_MAIN("get and set acl text")
{
    if (!acl_check()) return;
    
    WvString testfn("wvacltest.tmp"), username, groupname;
    create_file(testfn, username, groupname);
    chmod(testfn, 0421);

    // get the acl, reset the file's permissions, and, set it to the permission
    // we just read
    WvString acl = get_acl_short_form(testfn);
    chmod(testfn, 0000);
    set_acl_permissions(testfn, acl, false);
    struct stat st;
    stat(testfn, &st);
    WVPASSEQ(st.st_mode, 0100421);
}

WVTEST_MAIN("add another user and read it back")
{
    if (!acl_check()) return;
    
    WvString testfn("wvacltest.tmp"), username, groupname;
    WvString newuser("root");   // I think we can assume root exists

    create_file(testfn, username, groupname);
    chmod(testfn, 0421);

    set_acl_permission(testfn, "u", newuser, true, true, false);

    // our new permission changed the Unix group field to "rwx"
    struct stat st;
    stat(testfn, &st);
    WVPASSEQ(st.st_mode, 0100471);

    WvSimpleAclEntryList acls;
    get_simple_acl_permissions(testfn, acls);
 
    bool u_chk1 = false, u_chk2 = false, g_chk = false, o_chk = false;
    WvSimpleAclEntryList::Iter i(acls);
    for (i.rewind(); i.next(); )
    {
	switch (i().type)
	{
	case WvSimpleAclEntry::AclUser:
            printf("user entry for %s%s\n", i().name.cstr(), 
                   i().owner ? " (owner)" : "");
            if (i().owner)
            {
                WVPASS(!u_chk1);
                WVPASSEQ(i().name, username);
                WVPASS(i().read);
                WVFAIL(i().write);
                WVFAIL(i().execute);
                if (i().name == username)
                    u_chk1 = true;
            }
            else
            {
                WVPASS(!u_chk2);
                WVPASSEQ(i().name, newuser);
                WVPASS(i().read);
                WVPASS(i().write);
                WVFAIL(i().execute);
                if (i().name == newuser)
                    u_chk2 = true;
            }
	    break;

	case WvSimpleAclEntry::AclGroup:
            printf("group entry for %s\n", i().name.cstr());
	    WVPASS(!g_chk);
	    WVPASSEQ(i().name, groupname);
	    WVFAIL(i().read);
	    WVPASS(i().write);
	    WVFAIL(i().execute);
	    if (i().name == groupname)
		g_chk = true;
	    break;

	case WvSimpleAclEntry::AclOther:
            printf("other entry\n");
	    WVPASS(!o_chk);
	    WVPASS(!i().name.len());
	    WVFAIL(i().read);
	    WVFAIL(i().write);
	    WVPASS(i().execute);
	    o_chk = true;
	    break;
	}
    }
    
    WVPASS(u_chk1);
    WVPASS(u_chk2);
    WVPASS(g_chk);
    WVPASS(o_chk);
}


WVTEST_MAIN("default ACL permissions")
{
    if (!acl_check()) return;

    WvString testfn("wvacltest.tmp"), username, groupname;
    WvString p1("u::rwx,g::rwx,o::rwx"), p2("u::---,g::---,o::---"),
             p3("u::r-x,g::r-x,o::r-x");

    // FIXME: these have to be in the same order to match strings
    WvString mask("u::r-x,u:root:rwx,g::r-x,m::r--,o::r-x"),
             mask_fixed("m::rwx,u::r-x,u:root:r--,g::r--,o::r-x");

    printf("create file but don't set anything\n");
    create_file(testfn, username, groupname);
    WVPASSEQ(get_acl_short_form(testfn, false), default_acl);
    WVPASSEQ(get_acl_short_form(testfn, true), WvString::null);

    printf("create dir but don't set anything\n");
    create_dir(testfn, username, groupname);
    WVPASSEQ(get_acl_short_form(testfn, false), default_acl);
    WVPASSEQ(get_acl_short_form(testfn, true), WvString::null);

    printf("default on non-dir\n");
    create_file(testfn, username, groupname);
    WVFAIL(set_default_acl_permissions(testfn, p1));
    WVPASSEQ(get_acl_short_form(testfn, false), default_acl);
    WVPASSEQ(get_acl_short_form(testfn, true), WvString::null);

    printf("set ACL w/no default\n");
    create_dir(testfn, username, groupname);
    WVPASS(set_acl_permissions(testfn, p1, false));
    WVPASSEQ(get_acl_short_form(testfn, false), p1);
    WVPASSEQ(get_acl_short_form(testfn, true), WvString::null);
 
    printf("set ACL + same default\n");
    create_dir(testfn, username, groupname);
    WVPASS(set_acl_permissions(testfn, p1, true));
    WVPASSEQ(get_acl_short_form(testfn, false), p1);
    WVPASSEQ(get_acl_short_form(testfn, true), p1);
  
    printf("default w/no ACL\n");
    create_dir(testfn, username, groupname);
    WVPASS(set_default_acl_permissions(testfn, p1));
    WVPASSEQ(get_acl_short_form(testfn, false), default_acl);
    WVPASSEQ(get_acl_short_form(testfn, true), p1);

    printf("ACL then different default\n");
    create_dir(testfn, username, groupname);
    WVPASS(set_acl_permissions(testfn, p1, false));
    WVPASS(set_default_acl_permissions(testfn, p2));
    WVPASSEQ(get_acl_short_form(testfn, false), p1);
    WVPASSEQ(get_acl_short_form(testfn, true), p2);

    printf("ACL + same default, then overwrite default\n");
    create_dir(testfn, username, groupname);
    WVPASS(set_acl_permissions(testfn, p1, true));
    WVPASS(set_default_acl_permissions(testfn, p2));
    WVPASSEQ(get_acl_short_form(testfn, false), p1);
    WVPASSEQ(get_acl_short_form(testfn, true), p2);

    printf("ACL w/different default, then overwrite ACL and default\n");
    create_dir(testfn, username, groupname);
    WVPASS(set_acl_permissions(testfn, p1, false));
    WVPASS(set_default_acl_permissions(testfn, p2));
    WVPASSEQ(get_acl_short_form(testfn, false), p1);
    WVPASSEQ(get_acl_short_form(testfn, true), p2);
    WVPASS(set_acl_permissions(testfn, p3, true));
    WVPASSEQ(get_acl_short_form(testfn, false), p3);
    WVPASSEQ(get_acl_short_form(testfn, true), p3);

    printf("ACL default mask\n");
    create_dir(testfn, username, groupname);
    WVPASS(set_acl_permissions(testfn, mask, true));
    WVPASSEQ(get_acl_short_form(testfn, true, false), mask);
    WVPASSEQ(get_acl_short_form(testfn, true, true), mask_fixed);
}
