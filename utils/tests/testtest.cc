#include "wvtest.h"
#include "wvstring.h"
#include <unistd.h>

static int sequence = 0;


WVTEST_MAIN("basic 1")
{
    WvString a, b;
    
    WVPASS(sequence++ == 0);
    
    WVPASS(a==NULL);
    WVPASS("Next one should fail");
    WVFAIL(b==NULL); // should fail
    WVPASS(1);
    WVPASS(a==b);
    WVFAIL(a!=b);
    
    a.append(b);
    WVPASS(a==b);
    WVPASS("Next one should fail");
    WVFAIL(a == b); // should fail
    WVFAIL(a != b);
    WVPASS(a == b);
    WVPASS(!sleep(1));
    WVFAIL(a != b);
    
    a.append("blah");
    WVPASS(a=="blah");
    WVPASS(a!=b);
}


WVTEST_MAIN("basic 2")
{
    WVPASS(++sequence == 2);
    WVPASS("booga booga");
}


WVTEST_MAIN("basic 3")
{
    WVFAIL(++sequence != 3);
    WVPASS("booga booga");
    WVPASS("Next one should fail");
    WVPASS(sequence != 3); // should fail
}


int main()
{
    return WvTest::run_all();
}
