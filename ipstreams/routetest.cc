#include "wviproute.h"
#include "wvlog.h"


int main()
{
    WvLog l("test");
    WvIPRouteList r;
    WvIPRouteList::Iter i(r);
    
    for (i.rewind(); i.next(); )
	l("%s\n", *i.data());
    
    WvIPAddr a("192.168.42.22");
    l("\n%s through:\n  %s\n", a, *r.find(a));

    WvIPAddr b("1.2.3.4");
    l("\n%s through:\n  %s\n", b, *r.find(b));
}
