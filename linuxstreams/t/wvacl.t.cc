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
    WvFile tmp("/tmp/wvacltest", O_WRONLY | O_CREAT | O_TRUNC, 0754);
    WvString username(getpwuid(getuid())->pw_name);
    WvString groupname(getgrgid(getgid())->gr_name);
    
    WvSimpleAclEntryList acls;
    get_simple_acl_permissions("/tmp/wvacltest", acls);
 
    bool u_chk = false, g_chk = false, o_chk = false;
    WvSimpleAclEntryList::Iter i(acls);
    for (i.rewind(); i.next(); )
    {
	switch (i().type)
	{
	case WvSimpleAclEntry::AclUser:
	    WVPASS(!u_chk);
	    WVPASS(i().name == username);
	    WVPASS(i().read);
	    WVPASS(i().write);
	    WVPASS(i().execute);
	    if (i().name == username)
		u_chk = true;
	    break;

	case WvSimpleAclEntry::AclGroup:
	    WVPASS(!g_chk);
	    WVPASS(i().name == groupname);
	    WVPASS(i().read);
	    WVPASS(!i().write);
	    WVPASS(i().execute);
	    if (i().name == groupname)
		g_chk = true;
	    break;

	case WvSimpleAclEntry::AclOther:
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

