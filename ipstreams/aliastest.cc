/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvIPAliaser test program.
 *
 */

#include "wvipaliaser.h"

int main()
{
    free(malloc(1));
    
    WvIPAliaser a, b;
    
    a.start_edit();
    a.add("1.2.3.4");
    b.start_edit();
    a.add("1.2.3.4");
    b.add("2.4.6.5");
    b.add("3.4.5.6");
    b.add("1.2.3.4");
    b.done_edit();
    a.done_edit();

    a.dump(); b.dump();

    a.start_edit();
    a.add("1.2.3.5");
    a.add("1.2.3.6");
    a.done_edit();
    
    a.dump(); b.dump();
    
    return 0;
}
