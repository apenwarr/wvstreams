#include "wvslp.h"
#include "wvcrash.h"
#include "wvlog.h"
#include "wvstringlist.h"

int main(int argc, char **argv)
{ 
    wvcrash_setup(argv[0]);
    
    WvLog log("slplisttest", WvLog::Info);

#ifdef WITH_SLP
    WvStringList list;
    
    slp_get_servs("uniconf.niti", list);
    
    log("Got the list...\n");
    
    WvStringList::Iter i(list);
    for (i.rewind(); i.next(); )
        log("Found: %s", *i);

#else
    log("No SLP Support!\n");
#endif
    
    return 0;
}
