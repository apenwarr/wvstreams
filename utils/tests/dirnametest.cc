#include "strutils.h"


void test(WvStringParm s)
{
    printf("'%s': '%s' -- '%s'\n",
	   s.cstr(), getdirname(s).cstr(), getfilename(s).cstr());
}


int main()
{
    WvString a("/tmp"), b("frog"), c("frog/bog/blog"), d("%s/", c),
              e("%s//", d), f("%s/zot", e), g(""), h("/big/fat/file");
    
    test(a);
    test(b);
    test(c);
    test(d);
    test(e);
    test(f);
    test(g);
    test(h);
}
