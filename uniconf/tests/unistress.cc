#include "uniconfroot.h"
#include "wvstream.h"
#include "wvtimeutils.h"

int main(int argc, char **argv)
{
    const char *mon = (argc > 1) ? argv[1] : "ini:/tmp/big.cfg";
    wvcon->print("Using uniconf moniker '%s'\n", mon);
    
    UniConfRoot cfg(mon);
    UniConf c2(cfg["/uids"]);
    WvTime start;
    int count;
    
    while (1)
    {
	for (start = wvtime(), count = 0;
	     msecdiff(wvtime(), start) < 5000;
	     count++)
	{
	    if (!cfg.whichmount() || !cfg.whichmount()->isok())
	    {
		wvcon->print("not isok! aborting.\n");
		return 1;
	    }
	    
	    UniConf::Iter i(c2);//cfg["/uids"]);
	    for (i.rewind(); i.next(); )
	    {
		UniConf c(*i);
		WvString v(i._value());
	    }
	}
	
	wvcon->print("%s iters/sec (%s in 5ms)\n",
		     count/5,  count);
    }
    
    return 0;
}
