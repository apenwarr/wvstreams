#include "wvacl.h"
#include "wvfile.h"
#include "wvtest.h"
#include "wvlogrcv.h"
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>

WvLogConsole logrcv(1);

WVTEST_MAIN("default get_simple_acl_permissions()")
{
    WvString testfn("/tmp/wvacltest"), username, groupname;
    acl_check();
    WvFile tmp(testfn, O_WRONLY | O_CREAT | O_TRUNC, 0754);

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

    WvSimpleAclEntryList acls;
    get_simple_acl_permissions("/tmp/wvacltest", acls);
 
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

