#include "wvtest.h"
#include "wvtimeutils.h"
#include "wvstring.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif



WVTEST_MAIN("msecdiff()")
{
    time_t result;
    WvTime tmp;
 
    // Diff between zero time is 0
    result = msecdiff(wvtime_zero, wvtime_zero);
    WVPASS(result == 0);

    // Diff between random time (in sec) and zero is random time * 1000
    tmp.tv_sec = 854;
    tmp.tv_usec =  0;
    result = msecdiff(tmp, wvtime_zero);
    WVPASS(result == tmp.tv_sec * 1000);

    // Diff between zero and random time (in sec) is -random time * 1000
    tmp.tv_sec = 854;
    tmp.tv_usec =  0;
    result = msecdiff(wvtime_zero, tmp);
    WVPASS(result == tmp.tv_sec * -1000);

    // Diff between random time (in usec) and zero is random time / 1000
    // note: msecdiff simply chops the decimals away
    tmp.tv_sec = 0;
    tmp.tv_usec = 999;
    result = msecdiff(tmp, wvtime_zero);
    WVPASS(result == 0);
    tmp.tv_usec = 1000;
    result = msecdiff(tmp, wvtime_zero);
    WVPASS(result == 1);
    tmp.tv_usec = 1001;
    result = msecdiff(tmp, wvtime_zero);
    WVPASS(result == 1);
    
    // Diff between zero and random time (in usec) is random time / -1000
    // note: msecdiff simply chops the decimals away
    tmp.tv_sec = 0;
    tmp.tv_usec = 999;
    result = msecdiff(wvtime_zero, tmp);
    WVPASS(result == 0);
    tmp.tv_usec = 1000;
    result = msecdiff(wvtime_zero, tmp);
    WVPASS(result == -1);
    tmp.tv_usec = 1001;
    result = msecdiff(wvtime_zero, tmp);
    WVPASS(result == -1);

    // Diff between random time and same time is 0
    tmp.tv_sec = 854;
    tmp.tv_usec = 854;
    result = msecdiff(tmp, tmp);
    WVPASS(result == 0);

    // Test for integer overflow in msecdiff()
    WvTime t1(1109780720, 0);
    WvTime t2(1126725000, 0);
    time_t tdiff = msecdiff(t1, t2);
    // WVPASS(tdiff < 0); BUGZID:16357
}

/* 
 * FIXME: I don't know if there is any way to test this.
WVTEST_MAIN("wvtime()")
{
}
*/


WVTEST_MAIN("msecadd()")
{
    WvTime result, tmp;
    
    // test that wvtime_zero + 0 = wvtime_zero
    result = msecadd(wvtime_zero, 0);
    WVPASS(result == wvtime_zero);

    // test that wvtime_zero + 999 = 0sec (999*1000)usec
    tmp.tv_sec = 0;
    tmp.tv_usec = 999 * 1000;
    result = msecadd(wvtime_zero, 999);
    WVPASS(result == tmp);
    
    // test that wvtime_zero + 1000 = 1sec 0usec
    tmp.tv_sec = 1;
    tmp.tv_usec = 0;
    result = msecadd(wvtime_zero, 1000);
    WVPASS(result == tmp);

    // test that wvtime_zero + 1001 = 1sec 1000usec
    tmp.tv_sec = 1;
    tmp.tv_usec = 1000;
    result = msecadd(wvtime_zero, 1001);
    WVPASS(result == tmp);

    // test that wvtime_zero + -999 = 0sec (-999*1000)usec
    tmp.tv_sec = 0;
    tmp.tv_usec = -999 * 1000;
    normalize(tmp);
    result = msecadd(wvtime_zero, -999);
    WVPASS(result == tmp);
    
    // test that wvtime_zero + -1000 = -1sec 0usec
    tmp.tv_sec = -1;
    tmp.tv_usec = 0;
    result = msecadd(wvtime_zero, -1000);
    WVPASS(result == tmp);

    // test that wvtime_zero + -1001 = -1sec -1000usec
    tmp.tv_sec = -1;
    tmp.tv_usec = -1000;
    normalize(tmp);
    result = msecadd(wvtime_zero, -1001);
    WVPASS(result == tmp);
}

WVTEST_MAIN("tvdiff()")
{
    WvTime result, tmp;
    
    // test that zero - zero = 0
    result = tvdiff(wvtime_zero, wvtime_zero);
    WVPASS(result == wvtime_zero);

    // diff between random time and wvtime_zero = random time
    tmp.tv_sec = 854;
    tmp.tv_usec = 854;
    result = tvdiff(tmp, wvtime_zero);
    WVPASS(result == tmp);

    // diff between wvtime_zero and random time = -random time
    tmp.tv_sec = 854;
    tmp.tv_usec = 854;
    result = tvdiff(wvtime_zero, tmp);
    tmp.tv_sec = -854;
    tmp.tv_usec = -854;
    normalize(tmp);
    WVPASS(result == tmp);

    // diff between random time and same time = wvzero_time
    tmp.tv_sec = 854;
    tmp.tv_usec = 854;
    result = tvdiff(tmp, tmp);
    WVPASS(result == wvtime_zero);
}

WVTEST_MAIN("normalize()")
{
    WvTime result, tmp;

    // test that zero is normalized to zero
    result = wvtime_zero;
    normalize(result);
    WVPASS(result == wvtime_zero);

    // Test that time doesn't change if it doesn't need to
    tmp.tv_sec = 854;
    tmp.tv_usec = 999999;
    result = tmp;
    normalize(result);
    WVPASS(result == tmp);

    // Test that a usec value greater than 1000000 is reduced and put into
    // the sec value
    result.tv_sec = 854;
    result.tv_usec = 1000000;
    normalize(result);
    tmp.tv_sec = 855;
    tmp.tv_usec = 0;
    WVPASS(result == tmp);

    result.tv_sec = 854;
    result.tv_usec = 1000001;
    normalize(result);
    tmp.tv_sec = 855;
    tmp.tv_usec = 1;
    WVPASS(result == tmp);

    // Test that a negative usec value get converted to a positive val
    result.tv_sec = 854;
    result.tv_usec = -1;
    normalize(result);
    tmp.tv_sec = 853;
    tmp.tv_usec = 999999;
    WVPASS(result == tmp);

    result.tv_sec = 854;
    result.tv_usec = -1000001;
    normalize(result);
    tmp.tv_sec = 852;
    tmp.tv_usec = 999999;
    WVPASS(result == tmp); 

    result.tv_sec = 854;
    result.tv_usec = -853000001;
    normalize(result);
    tmp.tv_sec = 0;
    tmp.tv_usec = 999999;
    WVPASS(result == tmp); 

    // test that both vals negative works.
    result.tv_sec = -854;
    result.tv_usec = -1;
    normalize(result);
    tmp.tv_sec = -855;
    tmp.tv_usec = 999999;
    WVPASS(result == tmp);

    result.tv_sec = -854;
    result.tv_usec = -1000001;
    normalize(result);
    tmp.tv_sec = -856;
    tmp.tv_usec = 999999;
    WVPASS(result == tmp); 

    result.tv_sec = -854;
    result.tv_usec = -853000001;
    normalize(result);
    tmp.tv_sec = -1708;
    tmp.tv_usec = 999999;
    WVPASS(result == tmp); 
}

WVTEST_MAIN("operator<()")
{
    WvTime a = wvtime_zero;
    WvTime b = wvtime_zero;

    // Test that zero is not less than zero
    WVFAIL(wvtime_zero < wvtime_zero);

    // Test a few simple positive values
    a.tv_sec = 2;
    a.tv_usec = 999999;
    b.tv_sec = 1;
    a.tv_usec = 1999998;
    WVPASS(b < a);
	
    a.tv_sec = 2;
    a.tv_usec = 999999;
    b.tv_sec = 1;
    b.tv_usec = 1999999;
    WVFAIL(b < a);
    WVFAIL(a < b);

    a.tv_sec = 2;
    a.tv_usec = 999999;
    b.tv_sec = 1;
    b.tv_usec = 2000000;
    WVPASS(a < b);

    // Test a few simple negative values
    a.tv_sec = -2;
    a.tv_usec = -999999;
    b.tv_sec = -1;
    a.tv_usec = -1999998;
    WVPASS(a < b);
	
    a.tv_sec = -2;
    a.tv_usec = -999999;
    b.tv_sec = -1;
    b.tv_usec = -1999999;
    WVFAIL(b < a);
    WVFAIL(a < b);

    a.tv_sec = -2;
    a.tv_usec = -999999;
    b.tv_sec = -1;
    b.tv_usec = -2000000;
    WVPASS(b < a);
}
