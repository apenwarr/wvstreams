#include "uniconfroot.h"
#include "wvlogrcv.h"
#include "strutils.h"
#include "wvtclstring.h"

#ifdef _WIN32
#pragma comment(linker, "/include:?UniRegistryGenMoniker@@3V?$WvMoniker@VIUniConfGen@@@@A")
#pragma comment(linker, "/include:?UniPStoreGenMoniker@@3V?$WvMoniker@VIUniConfGen@@@@A")
#pragma comment(linker, "/include:?UniIniGenMoniker@@3V?$WvMoniker@VIUniConfGen@@@@A")
#endif

void usage()
{
    fprintf(stderr,
	    "Usage: uni <cmd> <key> [extra stuff...]\n"
	    " where <cmd> is one of:\n"
	    "   get   - get the value of a key, with optional default\n"
	    "   set   - set a key to the given value from the command line\n"
	    "   xset  - set a key to the given value from stdin\n"
	    "   keys  - list the subkeys of a key\n"
	    "   hkeys - list the subkeys of a key, their subkeys, etc\n"
	    "   xkeys - list keys that match a wildcard\n"
	    "   dump  - list the subkeys/values of a key (key=value)\n"
	    "   hdump - list the subkeys/values recursively\n"
	    "   xdump - list keys/values that match a wildcard\n"
	    "   del   - delete all subkeys\n"
	    "   help  - this text\n");
}

int main(int argc, char **argv)
{
    WvLogConsole logcon(2, WvLog::Info);
    
    const char *confuri = getenv("UNICONF");
    if (!confuri)
    {
	fprintf(stderr, "%s: UNICONF environment variable not set!\n",
		argv[0]);
	return 2;
    }
    
    if (argc < 3)
    {
	usage();
	return 3;
    }
    
    UniConfRoot cfg(confuri);
    
    if (!cfg.whichmount() || !cfg.whichmount()->isok())
    {
	fprintf(stderr, "%s: can't connect to uniconf at '%s'\n",
		argv[0], confuri);
	return 5;
    }
    
    // note: we know cmd and arg1 are non-NULL, but arg2 may be the argv
    // terminator, which is a NULL.  That has a special meaning for some
    // commands, like 'set', and is different from the empty string.
    const char *_cmd = argv[1], *arg1 = argv[2],
	       *arg2 = argc > 3 ? argv[3] : NULL;
    WvString cmd(_cmd);
    strlwr(cmd.edit());
    
    if (cmd == "get")
    {
	WvString val = cfg[arg1].getme(arg2);
	if (!val.isnull())
	{
	    fputs(val, stdout);
	    //fflush(stdout); // shouldn't be necessary!
	    return 0; // okay
	}
	else
	    return 1; // not found and no default given
    }
    else if (cmd == "set")
    {
	cfg[arg1].setme(arg2);
	cfg.commit();
	return 0; // always works
    }
    else if (cmd == "xset")
    {
	// like set, but read from stdin
	WvDynBuf buf;
	size_t len;
	char *cptr;
	while (wvcon->isok())
	{
	    cptr = (char *)buf.alloc(10240);
	    len = wvcon->read(cptr, 10240);
	    buf.unalloc(10240 - len);
	}
	cfg[arg1].setme(buf.getstr());
	cfg.commit();
	return 0; // always works
    }
    else if (cmd == "keys")
    {
	UniConf::Iter i(cfg[arg1]);
	for (i.rewind(); i.next(); )
	    wvcon->print("%s\n", wvtcl_escape(i->key(), "\r\n"));
    }
    else if (cmd == "hkeys")
    {
	UniConf sub(cfg[arg1]);
	UniConf::RecursiveIter i(sub);
	for (i.rewind(); i.next(); )
	    wvcon->print("%s\n", wvtcl_escape(i->fullkey(sub), "\r\n"));
    }
    else if (cmd == "xkeys")
    {
	UniConf::XIter i(cfg, arg1);
	for (i.rewind(); i.next(); )
	    wvcon->print("%s\n", wvtcl_escape(i->fullkey(cfg), "\r\n"));
    }
    else if (cmd == "dump")
    {
	// note: the output of this command happens to be compatible with
	// (can be read by) the 'ini' UniConf backend.
	UniConf::Iter i(cfg[arg1]);
	for (i.rewind(); i.next(); )
	    wvcon->print("%s = %s\n",
			 wvtcl_escape(i->key(), "\r\n[]="),
			 wvtcl_escape(i->getme(""), "\r\n[]="));
    }
    else if (cmd == "hdump")
    {
	// note: the output of this command happens to be compatible with
	// (can be read by) the 'ini' UniConf backend.
	UniConf sub(cfg[arg1]);
	UniConf::RecursiveIter i(sub);
	for (i.rewind(); i.next(); )
	    wvcon->print("%s = %s\n",
			 wvtcl_escape(i->fullkey(sub), "\r\n[]="),
			 wvtcl_escape(i->getme(""), "\r\n[]="));
    }
    else if (cmd == "xdump")
    {
	// note: the output of this command happens to be compatible with
	// (can be read by) the 'ini' UniConf backend.
	UniConf::XIter i(cfg, arg1);
	for (i.rewind(); i.next(); )
	    wvcon->print("%s = %s\n",
			 wvtcl_escape(i->fullkey(cfg), "\r\n[]="),
			 wvtcl_escape(i->getme(""), "\r\n[]="));
    }
    else if (cmd == "del")
    {
	UniConf sub(cfg[arg1]);
	sub.remove();
	cfg.commit();
    }
    else if (cmd == "help")
    {
	usage();
	return 5;
    }
    else
    {
	fprintf(stderr, "%s: unknown command '%s'!\n", argv[0], _cmd);
	return 4;
    }
}
