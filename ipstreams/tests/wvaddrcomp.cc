/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvAddr comparison test program.
 *
 */

#include "wvaddr.h"
#include "assert.h"

int main()
{
    free(malloc(1));

    WvEncap tmp;
    WvStringAddr string("1.2.3.4", tmp);
    WvIPAddr justip1("1.2.3.4");
    WvIPAddr justip2("7.2.3.4");
    WvIPNet ipnet1("1.2.3.4", 24);
    WvIPNet ipnet2("7.2.3.4", 24);
    WvIPNet ipnet3("7.2.3.4", 42);
    WvIPNet ipport1("1.2.3.4", 75);
    WvIPNet ipport2("7.2.3.4", 75);
    WvIPNet ipport3("7.2.3.4", 57);

    assert (justip1 == ipnet1 && ipnet2 == justip2 && justip1 == justip1 &&
            ipnet2 == ipnet2 && ipnet1 != ipnet2 && justip1 != justip2 &&
            justip2 == ipport3 && ipport3 == justip2 && ipport3 != ipport2 &&
            ipport3 == ipport3 && string != justip1 && justip1 != string &&
            string == string); 
    
    return 0;
}
