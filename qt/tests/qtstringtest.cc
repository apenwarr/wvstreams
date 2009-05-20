/*
 * Test the WvString -> QString compatibility stuff
 */
#include <qstring.h>
#include "wvstring.h"
#include "wvlog.h"

WvString qf(const QString &q)
{
    return q;
}


QString wf(WvStringParm w)
{
    return (const char *)w;
}


int main()
{
    WvLog test("qtstringtest", WvLog::Info);
    
    WvString a("hello a\n");
    WvFastString b("bellow b\n");
    const char *c = "yellow c\n";
    
    QString z("mellow z\n");
    QCString y("yellow y\n");
    
    test(a);
    test(b);
    test(c);

    test(z);
    test(y);
    
    WvString z2(z), y2(y);
    WvFastString z3(z), y3(y);
    test(z2);
    test(y2);
    test(z3);
    test(y3);
    
    QString a2(a.cstr()), b2(b.cstr()), c2(c);
    QCString a3(a), b3(b), c3(c);
    test(a2);
    test(b2);
    test(c2);
    test(a3);
    test(b3);
    test(b3);
    
    test(qf("bonk\n"));
    test(wf("wonk\n"));
    test(qf(wf(qf(wf(qf(WvString("fishy %s %s %s %s %s wishy\n",
				 a, b, c, z, y)))))));
    
    return 0;
}
