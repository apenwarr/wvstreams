/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvString test program.  Correct results:
 *
 *      A WvString is 8 bytes long.
 *      Foo Foo
 *      0 1 0 0 1
 *      1 0 1 1 0
 *      1 0 0 1
 *      0 1
 *      0 1
 *      Foo
 *      1 1 0
 *      0 1 0 0
 *      hello
 *      xyzzi 5                                        hello          b
 *      uest string
 *      vest string
 *      vest string .. west string
 */

#include "wvstring.h"

WvString test1(WvString s)
{
    s.edit()[0]++;
    return s;
}


int main()
{
    printf("A WvString is %d bytes long.\n", sizeof(WvString));

    WvString a, b("Foo"), c("Foo"), d("Blue"), e;
    char *ca = "Fork", *cb = "Foo", *cc = "Foo", *cd = NULL;
    
    {
	WvString *x = new WvString(a);
	WvString *y = new WvString(b);
	printf("%s %s\n", (const char *)b, (const char *)*y);
	delete y;
	delete x;
    }
    
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
    
    e = cb;
    
    // 1 1 0
    printf("%s\n", (const char *)e);
    printf("%d %d %d\n", e == cb, e == cc, e == cd);

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
    printf("%s .. %s\n", (const char *)s, (const char *)t);
    
    // NULL value tests.  NULL strings are different from zero-length strings.
    // null: ok1
    // null: 1 1 0 1 0
    // null: 0 1 1 1 0
    WvString n1, n2((const char *)NULL), n3(n1), n4("foo"), n5("blah"), n6("");
    n4 = n1;
    n1 = n4;
    n5 = n1;
    printf("null: ok1\n");
    printf("null: %d %d %d %d %d\n", !n1, !n2, n1.len(), n1==n2, n3.num());
    n3.append("junk");
    printf("null: %d %d %d %d %d\n", n1==n6, !n1, !n6, n1==NULL, n6==NULL);
    WvString ns("%s %s %s %s %s %s\n", n1, n2, n3, n4, n5, n6);
    printf("null: %s\n", ns.cstr());
    
    return 0;
}
