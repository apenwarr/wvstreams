#include "wvtest.h"
#include "pwvstream.h"
#include "wvstreamclone.h"

WVTEST_MAIN("pclones")
{
    PWvStream s("loop:");
    
    {
	PWvStream c(s.addRef()), c2(s.addRef());
	c->print("hi!\n");
	WVPASSEQ(c2->getline(-1), "hi!");
	WVPASSEQ(c2->getline(0), "");
    }
    
    PWvStream c3(s.addRef());
    c3->print("boink\ngack");
    c3->nowrite();
    WVPASSEQ(c3->getline(-1), "boink");
    WVPASSEQ(c3->getline(-1), "gack");
    WVPASSEQ(c3->getline(-1), "");
}


WVTEST_MAIN("pcopy")
{
    char buf[4] = "xxx";
    PWvStream s("loop:");
    
    {
	PWvStream a1(s), a2, a3;
	a2 = a3;
	a2 = s;
	a2 = a2;
	a2 = a1;
	s = a1;
    }
    
    s->write("boo", 3);
    size_t len = s->read(buf, 3);
    WVPASSEQ(len, 3);
    WVPASSEQ(buf, "boo");
}
