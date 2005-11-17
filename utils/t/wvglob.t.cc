#include "wvtest.h"
#include "wvglob.h"
#include "wvstream.h"


WVTEST_MAIN("glob to regex")
{
    const char **glob;

    // Good globs:
    const char *good_globs[] = {
        "", "?", "*", "[abc]", "[!abc]", "[^abc]", "*.{cc,h}", "*.{}",
        "file?.[abc]d*", "+|()$^", "[!0-9]",
        "{n{1,+|2{a,b,c},3}e\\{st.{c\\[c*,?h},fi\\?le[!0-9]}", NULL
    };
    for (glob=&good_globs[0]; *glob; ++glob)
    {
        WvString errstr;
        WvString regex = WvGlob::glob_to_regex(*glob, &errstr);
        WVPASS(!errstr);
        wvout->print("glob=%s regex=%s errstr=%s\n",
                *glob, regex, errstr);
    }

    // Bad globs:
    const char *bad_globs[] = {
        "[a-z", "file*.{a,b", "{a,b{c,d{e,f},g},h[def", "\\[\\*\\", NULL
    };
    for (glob=&bad_globs[0]; *glob; ++glob)
    {
        WvString errstr;
        WvString regex = WvGlob::glob_to_regex(*glob, &errstr);
        WVFAIL(!errstr);
        wvout->print("glob=%s regex=%s errstr=%s\n",
                *glob, regex, errstr);
    }
}

WVTEST_MAIN("glob object")
{
    WvGlob glob("file[0-9].{cc,h}");

    WVPASS(glob.match("file3.h"));
    WVPASS(glob.match("file9.cc"));
    WVFAIL(glob.match("fil3.h"));
    WVFAIL(glob.match(""));
    WVFAIL(glob.match("file"));
}

WVTEST_MAIN("glob registers")
{
    WvGlob glob("file?.{cc,h}.*.[a-z]");

    WvString one, two, three, four;
    WVPASS(glob.match("file7.cc.fred.q", one, two, three, four));
    WVPASS(one == "7");
    WVPASS(two == "cc");
    WVPASS(three == "fred");
    WVPASS(four == "q");
}
