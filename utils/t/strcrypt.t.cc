#include "wvtest.h"
#include "strutils.h"

#include <stdlib.h>



WVTEST_MAIN("passwd_crypttest.cc")
{
    srandom(time(0));

    char *blank, *word, *longword;
    WvString blank_result1, blank_result2, word_result, longword_result;

    blank = "";
    word = "12345678";
    longword = "1234567890000000000000000000000000000";
    
    blank_result1 = passwd_crypt(blank);
    blank_result2 = passwd_crypt(blank);
    word_result = passwd_crypt(word);
    longword_result = passwd_crypt(longword);

 
    WVPASS(!!blank_result1 && blank_result1 != "*");
    WVPASS(!!blank_result2 && blank_result2 != "*");
    WVFAIL(blank_result1 == blank_result2);
    WVPASS(!!word_result && word_result != "*");
    WVPASS(!!longword_result && longword_result != "*");
}

WVTEST_MAIN("passwd_md5test.cc")
{
    srandom(time(0));

    char *blank, *word, *longword;
    WvString blank_result1, blank_result2, word_result, longword_result;

    blank = "";
    word = "12345678";
    longword = "1234567890000000000000000000000000000";
    
    blank_result1 = passwd_md5(blank);
    blank_result2 = passwd_md5(blank);
    word_result = passwd_md5(word);
    longword_result = passwd_md5(longword);

 
    WVPASS(!!blank_result1 && blank_result1 != "*");
    WVPASS(!!blank_result2 && blank_result2 != "*");
    WVFAIL(blank_result1 == blank_result2);
    WVPASS(!!word_result && word_result != "*");
    WVPASS(!!longword_result && longword_result != "*");
}
