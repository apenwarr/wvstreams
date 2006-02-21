/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvAddr comparison test program.
 *
 */

#include "wvaddr.h"
#include "assert.h"

void test1(WvIPAddr& justip1, WvIPAddr& justip2, WvIPAddr& ipnet1, WvIPAddr& ipnet2, WvIPAddr& ipnet3, WvIPAddr& ipport1, WvIPAddr& ipport2, WvIPAddr& ipport3)
{
    assert (justip1 == ipnet1 && ipnet2 == justip2 && justip1 == justip1 &&
            ipnet2 == ipnet2 && ipnet1 != ipnet2 && justip1 != justip2 &&
            justip2 == ipport3 && ipport3 == justip2 && ipport3 != ipport2 &&
            ipport3 == ipport3);
};

void test2(WvAddr& string, WvAddr& justip1, WvAddr& justip2, WvAddr& ipnet1, WvAddr& ipnet2, WvAddr& ipnet3, WvAddr& ipport1, WvAddr& ipport2, WvAddr& ipport3)
{
    assert (justip1 == ipnet1 && ipnet2 == justip2 && justip1 == justip1 &&
            ipnet2 == ipnet2 && ipnet1 != ipnet2 && justip1 != justip2 &&
            justip2 == ipport3 && ipport3 == justip2 && ipport3 != ipport2 &&
            ipport3 == ipport3 && string != justip1 && justip1 != string &&
            string == string);
};

int main()
{
    free(malloc(1));

    WvEncap tmp;
    WvStringAddr string("1.2.3.4", tmp);
    WvIPAddr justip1("1.2.3.4");
    WvIPAddr justip2("7.2.3.4");
    WvIPNet ipnet1("1.2.3.4", 24);
    WvIPNet ipnet2("7.2.3.4", 24);
    WvIPNet ipnet3("7.2.3.4", 32);
    WvIPNet ipport1("1.2.3.4", 75);
    WvIPNet ipport2("7.2.3.4", 75);
    WvIPNet ipport3("7.2.3.4", 57);

    test1(justip1, justip2, ipnet1, ipnet2, ipnet3, ipport1, ipport2, ipport3);

    test2(string, justip1, justip2, ipnet1, ipnet2, ipnet3, ipport1, ipport2, ipport3);

    assert (justip1 == ipnet1 && ipnet2 == justip2 && justip1 == justip1 &&
            ipnet2 == ipnet2 && ipnet1 != ipnet2 && justip1 != justip2 &&
            justip2 == ipport3 && ipport3 == justip2 && ipport3 != ipport2 &&
            ipport3 == ipport3 && string != justip1 && justip1 != string &&
            string == string); 
    
    return 0;
}
