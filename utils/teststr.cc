#include "wvstring.h"

int main()
{
    WvString a, b("Foo"), c("Foo"), d("Blue"), e;
    char *ca = "Fork", *cb = "Foo", *cc = "Blue", *cd = NULL;

    // 0 1 0 0 1
    printf("%d %d %d %d %d\n", a == b, b == c, c == d, d == e, e == a);
    
    e = c;

    // 0 1
    printf("%d %d\n", e == a, e == b);
    
    e = e;
    
    // 1 0
    printf("%d %d\n", e == a, e == b);

    // 0 1 0 0
    printf("%d %d %d %d\n", a == ca, b == cb, d == cb, d == cd);
    
    return 0;
}
