#include "uniconfroot.h"
#include "wvlogrcv.h"
#include "strutils.h"
#include "wvtclstring.h"

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
	fprintf(stderr, "Usage: %s <cmd> <key> [extra stuff...]\n",
		argv[0]);
	return 3;
    }
    
    UniConfRoot cfg(confuri);
    
    // note: we know cmd and arg1 are non-NULL, but arg2 may be the argv
    // terminator, which is a NULL.  That has a special meaning for some
    // commands, like 'set', and is different from the empty string.
    const char *_cmd = argv[1], *arg1 = argv[2], *arg2 = argv[3];
    WvString cmd(_cmd);
    strlwr(cmd.edit());
    
    if (cmd == "get")
    {
	WvString val = cfg[arg1].get(arg2);
	if (!val.isnull())
	{
	    fputs(val, stdout);
	    return 0; // okay
	}
	else
	    return 1; // not found and no default given
    }
    else if (cmd == "set")
    {
	cfg[arg1].set(arg2);
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
	cfg[arg1].set(buf.getstr());
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
    else if (cmd == "dump")
    {
	// note: the output of this command happens to be compatible with
	// (can be read by) the 'ini' UniConf backend.
	UniConf::Iter i(cfg[arg1]);
	for (i.rewind(); i.next(); )
	    wvcon->print("%s = %s\n",
			 wvtcl_escape(i->key(), "\r\n[]="),
			 wvtcl_escape(i->get(""), "\r\n[]="));
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
			 wvtcl_escape(i->get(""), "\r\n[]="));
    }
    else
    {
	fprintf(stderr, "%s: unknown command '%s'!\n", argv[0], _cmd);
	return 4;
    }
}
