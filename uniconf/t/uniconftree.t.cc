#include "wvtest.h"
#include "uniconftree.h"

WVTEST_MAIN("univaluetree basics")
{
    UniConfValueTree t(NULL, "key", "value");
    WVPASSEQ(t.value(), "value");
    WVFAIL(t.haschildren());
    UniConfValueTree t2(&t, "key2", "value2");
    WVPASS(t.haschildren());
    WVFAIL(t2.haschildren());
}


bool keyvalcomp(const UniConfValueTree *a, const UniConfValueTree *b,
		void *userdata)
{
    return a && b && a->key() == b->key() && a->value() == b->value();
}


WVTEST_MAIN("recursivecompare")
{
    UniConfValueTree a(NULL, "key", "value");
    UniConfValueTree b(NULL, "key", "value");
    
    for (int i = 1; i <= 1000; i++)
	new UniConfValueTree(&a, i, i);
    for (int i = 1000; i >= 1; i--)
	new UniConfValueTree(&b, i, i);
    
    WVPASS(a.compare(&b, keyvalcomp, NULL));
}
