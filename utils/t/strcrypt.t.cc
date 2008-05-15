#include "wvtest.h"
#include "strutils.h"

#include <stdlib.h>



WVTEST_MAIN("passwd_crypttest.cc")
{
#ifdef WIN32
    srand(time(0));
#else
    srandom(time(0));
#endif

    const char *blank = "";
    const char *word = "12345678";
    const char *longword = "1234567890000000000000000000000000000";
    
    WvString blank_result1 = passwd_crypt(blank);
    WvString blank_result2 = passwd_crypt(blank);
    WvString word_result = passwd_crypt(word);
    WvString longword_result = passwd_crypt(longword);

 
    WVPASS(!!blank_result1 && blank_result1 != "*");
    WVPASS(!!blank_result2 && blank_result2 != "*");
    WVFAIL(blank_result1 == blank_result2);
    WVPASS(!!word_result && word_result != "*");
    WVPASS(!!longword_result && longword_result != "*");
    
}

WVTEST_MAIN("passwd_md5test.cc")
{
    srandom(time(0));

    const char *blank = "";
    const char *word = "12345678";
    const char *longword = "1234567890000000000000000000000000000";
    
    WvString blank_result1 = passwd_md5(blank);
    WvString blank_result2 = passwd_md5(blank);
    WvString word_result = passwd_md5(word);
    WvString longword_result = passwd_md5(longword);

 
    WVPASS(!!blank_result1 && blank_result1 != "*");
    WVPASS(!!blank_result2 && blank_result2 != "*");
    WVFAIL(blank_result1 == blank_result2);
    WVPASS(!!word_result && word_result != "*");
    WVPASS(!!longword_result && longword_result != "*");
}
