#include "wvinterface.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
	fprintf(stderr, "usage: %s <ifcname>\n", argv[0]);
	return 1;
    }
    
    WvInterface ifc(argv[1]);
    
    wvcon->print("exists: %s\n"
		 "IP: %s\n"
		 "dst: %s\n"
		 "hwaddr: %s\n"
		 "flags: %s\n"
		 "up: %s\n"
		 "promisc: %s\n"
		 "arp: %s\n",
		 ifc.valid,
		 ifc.ipaddr(), ifc.dstaddr(), ifc.hwaddr(),
		 ifc.getflags(), ifc.isup(), ifc.ispromisc(), ifc.isarp());
    return 0;
}
