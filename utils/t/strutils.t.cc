#include "wvtest.h"
#include "wvlinklist.h"
#include "wvfile.h"
#include "strutils.h"
#include <unistd.h>

/*
 * Functions in strutils.h left untested:
 *  hexdump_buffer
 *  isnewline
 *  rfc822_date
 *  rfc1123_date
 *  passwd_crypt
 *
 * Tests that may be incorrect:
 *  fqdomainname
 */


WVTEST_MAIN("terminate_stringtest.cc")
{
    char *input[] = {new char[6], new char[7], new char[2], new char[3]};
    strcpy(input[0], "blah"); strcpy(input[1], "blah\n");
    strcpy(input[2], "");     strcpy(input[3], "\n");
    const char *desired[] = {"blah!", "blah!", "!", "!"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        char *result = terminate_string(input[i], '!');
        if (!WVFAIL(strcmp(result, desired[i])))
            printf("   because [%s] != [%s]\n", result, desired[i]);
        delete[] input[i];
    }
}

WVTEST_MAIN("trimtest.cc")
{
    char *input[] = {new char[7], new char[10],
                     new char[10], new char[10], new char[8]};
    strcpy(input[0], "foobar");    strcpy(input[1], "\t foobar");
    strcpy(input[2], "foobar\n "); strcpy(input[3], " \tfoo\r\t \n");
    strcpy(input[4], "foo bar");
    const char *desired[] = {"foobar", "foobar", "foobar", "foo", "foo bar"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        char *result = trim_string(input[i]);
        if (!WVFAIL(strcmp(result, desired[i])))
            printf("   because [%s] != [%s]\n", result, desired[i]);
        delete[] input[i];
    } 
}

WVTEST_MAIN("trimtest2.cc")
{
    char *input[] = {new char[9], new char[9], new char[9], new char[9]};
    strcpy(input[0], "abcdefgh"); strcpy(input[1], "xbcdefgh");
    strcpy(input[2], "abcdefgx"); strcpy(input[3], "abcxefgh");
    const char *desired[] = {"abcdefgh", "", "abcdefg", "abc"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        char *result = trim_string(input[i], 'x');
        if (!WVFAIL(strcmp(result, desired[i])))
            printf("   because [%s] != [%s]\n", result, desired[i]);
        delete[] input[i];
    }
}

WVTEST_MAIN("nbsptest.cc")
{
    char *input[] = {"a b c", "  a", "a\nb\tc ", "ab c\r"};
    const char *desired[] = {"a&nbsp;b&nbsp;c", "&nbsp;&nbsp;a", "a&nbsp;b&nbsp;c&nbsp;", "ab&nbsp;c&nbsp;"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        char *result = non_breaking(input[i]);
        if (!WVFAIL(strcmp(result, desired[i])))
            printf("   because [%s] != [%s]\n", result, desired[i]);
    }
}

WVTEST_MAIN("replace_chartest.cc")
{
    char *input[] = {new char[9], new char[9], new char[9], new char[9]};
    strcpy(input[0], "rbcdefgh"); strcpy(input[1], "abrdergh");
    strcpy(input[2], "abrdergr"); strcpy(input[3], "arrdefrh");
    const char *desired[] = {"xbcdefgh", "abxdergh", "abxdexgx", "axxdefrh"};

    const int len[] = {8, 5, 8, 5};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        replace_char((void *)input[i], 'r', 'x', len[i]);
        if (!WVFAIL(strcmp(input[i], desired[i])))
            printf("   because [%s] != [%s]\n", input[i], desired[i]);
    }
}

WVTEST_MAIN("sniptest.cc")
{
    char *input[] = {"foomatic", "automatic", "mafootic", "   foobar"};
    const char *desired[] = {"matic", "automatic", "mafootic", "   foobar"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        char *result = snip_string(input[i], "foo");
        if (!WVFAIL(strcmp(result, desired[i])))
            printf("   because [%s] != [%s]\n", result, desired[i]);
    }
}

WVTEST_MAIN("strlwrtest.cc")
{
    char *input[] = {new char[6], new char[6], new char[6], new char[6]};
    strcpy(input[0], "AbcdE"); strcpy(input[1], "aB De");
    strcpy(input[2], "a@C^e"); strcpy(input[3], "\tB\ndE");
    const char *desired[] = {"abcde", "ab de", "a@c^e", "\tb\nde"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        char *result = strlwr(input[i]);
        if (!WVFAIL(strcmp(result, desired[i])))
            printf("   because [%s] != [%s]\n", result, desired[i]);
        delete[] input[i];
    }
}

WVTEST_MAIN("struprtest.cc")
{
    char *input[] = {new char[6], new char[6], new char[6], new char[6]};
    strcpy(input[0], "aBCDe"); strcpy(input[1], "Ab dE");
    strcpy(input[2], "A@c^E"); strcpy(input[3], "\tb\nDe");
    const char *desired[] = {"ABCDE", "AB DE", "A@C^E", "\tB\nDE"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        char *result = strupr(input[i]);
        if (!WVFAIL(strcmp(result, desired[i])))
            printf("   because [%s] != [%s]\n", result, desired[i]);
        delete[] input[i];
    }
}

WVTEST_MAIN("is_wordtest.cc")
{
    char *input[] = {"q1w2e3", "q!w@e#", "Q 86", "\t\n\r52", "hy-phen"};
    const bool desired[] = {true, false, false, false, false};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
        WVPASS(is_word(input[i]) == desired[i]);
}

WVTEST_MAIN("web_unescapetest.cc")
{
    char *input = "%49+%6c%69%6b%65+%70%69%7a%7a%61%21";
    const char* desired = "I like pizza!";

    WVPASS(web_unescape(input) == desired);
}

WVTEST_MAIN("url_encodetest.cc")
{
    char *input = "http://www.free_email-account.com/~ponyman/mail.pl?name=\'to|\\|Y |)4|\\|Z4\'&pass=$!J83*p&folder=1N8()><";
    const char *desired = "http%3a//www.free_email-account.com/~ponyman/mail.pl%3fname%3d%27to%7c%5c%7cY%20%7c%294%7c%5c%7cZ4%27%26pass%3d%24%21J83%2ap%26folder%3d1N8%28%29%3e%3c";

    WVPASS(url_encode(input) == desired);
}

WVTEST_MAIN("backslash_escapetest.cc")
{
    char *input[] = {"hoopla!", "q!w2e3r$", "_+:\"<>?\\/", "J~0|<3R"};
    const char *desired[] = {"hoopla\\!", "q\\!w2e3r\\$", "\\_\\+\\:\\\"\\<\\>\\?\\\\\\/", "J\\~0\\|\\<3R"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        WvString result = backslash_escape(input[i]);
        if (!WVPASS(result == desired[i]))
            printf("   because [%s] != [%s]\n", result.cstr(), desired[i]);
    }
}

WVTEST_MAIN("strcounttest.cc")
{
    char *input[] = {"abj;lewi", "lk327ga", "a87gai783a", "aaaaaaa", "ao8&ATO@a"};
    int desired[] = {1, 1, 3, 7, 2};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
        WVPASS(strcount(input[i], 'a') == desired[i]);
}

WVTEST_MAIN("encode_hostnametest.cc")
{
    char *input[] = {"www.service.net", "www.you-can-too.com", "happybirthday.org", "www.canada.bigco.co.uk"};
    WvString desired[] = {"dc=www,dc=service,dc=net,cn=www.service.net", "dc=www,dc=you-can-too,dc=com,cn=www.you-can-too.com", "dc=happybirthday,dc=org,cn=happybirthday.org", "dc=www,dc=canada,dc=bigco,dc=co,dc=uk,cn=www.canada.bigco.co.uk"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
        WVPASS(encode_hostname_as_DN(input[i]) == desired[i]);
}

WVTEST_MAIN("nice_hostnametest.cc")
{
    char *input[] = {"n-i_c-e", "@2COOL", "E\\/1|_.1", "ha--ha__ha"};
    WvString desired[] = {"n-i-c-e", "x2COOL", "E1-.1", "ha-ha-ha"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
        WVPASS(nice_hostname(input[i]) == desired[i]);
}

WVTEST_MAIN("getfilenametest.cc")
{
    char *input[] = {"/tmp/file", "file.ext", "../../.file.wot", "/snick/dir/", "/snick/dira/../dirb/file"};
    WvString desired[] = {"file", "file.ext", ".file.wot", "dir", "file"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
        WVPASS(getfilename(input[i]) == desired[i]);
}

WVTEST_MAIN("getdirnametest.cc")
{
    char *input[] = {"/tmp/file", "file.ext", "../../.file.wot", "/snick/dir/", "/snick/dira/../dirb/file"};
    WvString desired[] = {"/tmp", ".", "../..", "/snick", "/snick/dira/../dirb"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
        WVPASS(getdirname(input[i]) == desired[i]);
}

WVTEST_MAIN("sizetoatest.cc")
{
    {
        long blocks = 987654321;
        long blocksize = 1000000;
        char *desired[15] = {"987.6 TB", "98.7 TB", "9.8 TB", "987.6 GB", 
            "98.7 GB", "9.8 GB", "987.6 MB", "98.7 MB", "9.8 MB", "987.6 KB",
            "98.7 KB", "9.8 KB", "987 bytes", "98 bytes", "9 bytes"};
        int i = 0;

        while(blocksize != 1)
        {
            if (!WVPASS(sizetoa(blocks, blocksize) == desired[i]))
                printf("   because [%s] != [%s]\n", 
                        sizetoa(blocks, blocksize).cstr(), desired[i]);
            blocksize /= 10;
            i++;
        }

        while(blocks)
        {
            if (!WVPASS(sizetoa(blocks, blocksize) == desired[i]))
                printf("   because [%s] != [%s]\n",
                        sizetoa(blocks, blocksize).cstr(), desired[i]);
            blocks /= 10;
            i++;
        }
    }
}

WVTEST_MAIN("lookuptest.cc")
{
    char *input[] = {"", "AbC", "a3k3 ", "abc", "ABC", NULL};
    
    WVPASS(lookup("abc", input, true) == 3);
    WVPASS(lookup("abc", input, false) == 1);
    WVPASS(lookup("ab", input, false) == -1);
    WVPASS(lookup("ABC", input, true) == 4);
    WVPASS(lookup("abcd", input, false) == -1);
}

template <typename T>
static bool listcmp(const WvList<T>& lhs, const WvList<T>& rhs)
{
    if (lhs.count() != rhs.count())
        return false;

    WvList<T>::Iter l(lhs), r(rhs);
    for (l.rewind(), r.rewind(); l.next() && r.next(); )
    {
        if (*l.ptr() != *r.ptr())
            return false;
    }

    return true;
}

WVTEST_MAIN("strcoll_splittest.cc")
{
    char *input[] = {"i:am colon\t:separated::", "i::too:am colon\tseparated"};
    WvList<WvString> desired[sizeof(input) / sizeof(char *)];
    WvList<WvString> desired_lim[sizeof(input) / sizeof(char *)];

    desired[0].add(new WvString("i"), true);
    desired[0].add(new WvString("am"), true);
    desired[0].add(new WvString("colon"), true);
    desired[0].add(new WvString("separated"), true);
    desired[0].add(new WvString(""), true);
    desired[1].add(new WvString("i"), true);
    desired[1].add(new WvString("too"), true);
    desired[1].add(new WvString("am"), true);
    desired[1].add(new WvString("colon"), true);
    desired[1].add(new WvString("separated"), true);

    desired_lim[0].add(new WvString("i"), true);
    desired_lim[0].add(new WvString("am"), true);
    desired_lim[0].add(new WvString("colon\t:separated::"), true);
    desired_lim[1].add(new WvString("i"), true);
    desired_lim[1].add(new WvString("too"), true);
    desired_lim[1].add(new WvString("am colon\tseparated"), true);

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        WvList<WvString> result;
        strcoll_split(result, input[i], " \t:");
        WVPASS(listcmp(result, desired[i]));
    }

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        WvList<WvString> result;
        strcoll_split(result, input[i], " \t:", 3);
        WVPASS(listcmp(result, desired_lim[i]));
    }
}

WVTEST_MAIN("strcoll_splitstricttest.cc")
{
    char *input[] = {"i:am colon\t:separated::", "i::too:am colon\tseparated"};
    WvList<WvString> desired[sizeof(input) / sizeof(char *)];
    WvList<WvString> desired_lim[sizeof(input) / sizeof(char *)];

    desired[0].add(new WvString("i"), true);
    desired[0].add(new WvString("am"), true);
    desired[0].add(new WvString("colon"), true);
    desired[0].add(new WvString(""), true);
    desired[0].add(new WvString("separated"), true);
    desired[0].add(new WvString(""), true);
    desired[0].add(new WvString(""), true);
    desired[1].add(new WvString("i"), true);
    desired[1].add(new WvString(""), true);
    desired[1].add(new WvString("too"), true);
    desired[1].add(new WvString("am"), true);
    desired[1].add(new WvString("colon"), true);
    desired[1].add(new WvString("separated"), true);

    desired_lim[0].add(new WvString("i"), true);
    desired_lim[0].add(new WvString("am"), true);
    desired_lim[0].add(new WvString("colon\t:separated::"), true);
    desired_lim[1].add(new WvString("i"), true);
    desired_lim[1].add(new WvString(""), true);
    desired_lim[1].add(new WvString("too:am colon\tseparated"), true);

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        WvList<WvString> result;
        strcoll_splitstrict(result, input[i], " \t:");
        WVPASS(listcmp(result, desired[i]));
    }

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        WvList<WvString> result;
        strcoll_splitstrict(result, input[i], " \t:", 3);
        WVPASS(listcmp(result, desired_lim[i]));
    }
}

WVTEST_MAIN("strcoll_jointest.cc")
{
    WvList<WvString> input[1];
    const char *desired[] = {"pop the top and diediedie"};
    const char *desired_join[] = {"pop Xthe top and XdieXdieXdieX"};
    
    input[0].add(new WvString("pop "), true);
    input[0].add(new WvString("the top and "), true);
    input[0].add(new WvString("die"), true);
    input[0].add(new WvString("die"), true);
    input[0].add(new WvString("die"), true);
    input[0].add(new WvString(""), true);

    for (unsigned int i = 0; i < sizeof(input) / sizeof(WvList<WvString>); ++i)
        WVPASS(strcoll_join(input[i], "") == desired[i]);
    
    for (unsigned int i = 0; i < sizeof(input) / sizeof(WvList<WvString>); ++i)
        WVPASS(strcoll_join(input[i], "X") == desired_join[i]);
}

WVTEST_MAIN("replacetest.cc")
{
    {
        char *input[2] = {"abbababababbba", "abbababababbbablab"};
        char *desired[2] = {"abxaxxaxxaxxaxbbxax", "abxaxxaxxaxxaxbbxaxblab"};

        for (int i = 0; i < 2; i++)
        {
            WvString result = strreplace(input[i], "ba", "xax");
            if (!WVPASS(result == desired[i]))
                printf("   because [%s] != [%s]\n", result.cstr(), desired[i]);
        }
    }
}

WVTEST_MAIN("undupetest.cc")
{
    char *input[] = {";alwg8", "aaog8", "absb  rd \raaaa", "aa8eai\na8\tawaa"};
    const char *desired[] = {";alwg8", "aog8", "absb  rd \ra", "a8eai\na8\tawa"};
    
    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        WvString result = undupe(input[i], 'a');
        if (!WVPASS(result == desired[i]))
            printf("   because [%s] != [%s]\n", result.cstr(), desired[i]);
    }
}

WVTEST_MAIN("hostnametest.cc")
{
    char host[1024];
    gethostname(host, sizeof(host));
    WVPASS(hostname() == host);
}

WVTEST_MAIN("fqdomainnametest.cc")
{
    char dom[1024];
    char host[1024];

    gethostname(host, sizeof(host));
    getdomainname(dom, sizeof(dom));

    if (strcmp("(none)", dom) && strlen(host) < 1023 - strlen(dom))
    {
        host[strlen(host) + 1] = 0;
        host[strlen(host)] = '.';
        strcat(host, dom);
    }
    WVPASS(fqdomainname() == host);
}

WVTEST_MAIN("metriculatetest.cc")
{
    int input[] = {293, 218976, 1896234178, 12837, -28376, -24, -2873, -182736};
    const char *desired[] = {"293", "218 976", "1 896 234 178", "12 837", "-28 376", "-24", "-2 873", "-182 736"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        WvString result = metriculate(input[i]);
        printf("%s FAIL\n", result.cstr());
        if (!WVPASS(result == desired[i]))
            printf("   because [%s] != [%s]\n", result.cstr(), desired[i]);
    }
}
