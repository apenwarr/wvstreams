/* FIXME: horribly incomplete */
#include "wvtest.h"
#include "wvrsa.h"

WVTEST_MAIN("extremely basic test")
{
    // this test tests to make sure that the cause
    // of BUGZID:3850 hasn't re-appeared...
    // (pub and prv weren't being initialized when
    //  we were initializing with a NULL value)
    {
	WvRSAKey rsa(NULL, false);
	WVPASS(rsa.public_str() == WvString::null);
	WVPASS(rsa.private_str() == WvString::null);
    }
    {
	WvRSAKey rsa(NULL, true);
	WVPASS(rsa.private_str() == WvString::null);
	WVPASS(rsa.public_str() == WvString::null);
    }
}
                    
