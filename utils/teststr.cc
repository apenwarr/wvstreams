#include "wvstring.h"

WvString test1(WvString s)
{
    s.edit()[0]++;
    return s;
}


int main()
{
    WvString a, b("Foo"), c("Foo"), d("Blue"), e;
    char *ca = "Fork", *cb = "Foo", *cd = NULL;

    // 0 1 0 0 1
    printf("%d %d %d %d %d\n", a == b, b == c, c == d, d == e, e == a);

    // 1 0 1 1 0
    printf("%d %d %d %d %d\n", a != b, b != c, c != d, d != e, e != a);
    
    // 1 0 0 1
    printf("%d %d %d %d\n", !a, !b, !c, !e);
    
    e = c;

    // 0 1
    printf("%d %d\n", e == a, e == b);
    
    e = e;
    
    // 1 0
    printf("%d %d\n", e == a, e == b);

    // 0 1 0 0
    printf("%d %d %d %d\n", a == ca, b == cb, d == cb, d == cd);
    
    // formatting test
    WvString ft0("%s", "hello");
    printf("%s\n", (const char *)ft0);
    WvString ft("%s %-40s %s %10s", "xyzzi", 5, "hello", "b");
    printf("%s\n", (const char *)ft);
    
    // parameter passing test
    WvString s(test1("test string"));
    printf("%s\n", (const char *)s);
    s = test1(s);
    printf("%s\n", (const char *)s);
    WvString t;
    t = test1(s);
    printf("%s -- %s\n", (const char *)s, (const char *)t);
    
    return 0;
}
