#include "wvtest.h"
#include "wvlinklist.h"
#include "wvfile.h"
#include "strutils.h"
#include <unistd.h>

/**
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

/** Tests terminate_string().
 * terminate_string() should remove any trailing whitespace on the
 * incoming string and append the provided char.
 */
WVTEST_MAIN("terminate_string")
{
    char *input[] = {new char[6], new char[7], new char[2], new char[4]};
    strcpy(input[0], "blah"); strcpy(input[1], "blah\n");
    strcpy(input[2], "");     strcpy(input[3], "\r\n");
    const char *desired[] = {"blah!", "blah!", "!", "!"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        char *result = terminate_string(input[i], '!');
        if (!WVFAIL(strcmp(result, desired[i])))
            printf("   because [%s] != [%s]\n", result, desired[i]);
        delete[] input[i];
    }
}

/** Tests trim_string().
 * trim_string() should remove any leading or trailing whitespace from
 * the incoming string.
 */
WVTEST_MAIN("trim")
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

/** Tests trim_string().
 * trim_string(), when provided with a trim character, should return
 * the portion of the incoming string that is before the first
 * occurence of that character.
 */
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

/** Tests non_breaking().
 * non_breaking() should replace any whitespace character in the
 * incoming string with "&nbsp;".
 */
WVTEST_MAIN("nbsp")
{
    char *input[] = {"a b c", "  a", "a\nb\tc ", "ab c\r"};
    const char *desired[] = {"a&nbsp;b&nbsp;c", "&nbsp;&nbsp;a", "a&nbsp;b&nbsp;c&nbsp;", "ab&nbsp;c&nbsp;"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        char *result = non_breaking(input[i]);
        if (!WVFAIL(strcmp(result, desired[i])))
            printf("   because [%s] != [%s]\n", result, desired[i]);
        delete[] result;
    }
}

/** Tests replace_char().
 * replace_char() should replace all instances of one given character
 * with another given character.
 */
WVTEST_MAIN("replace_char")
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
        delete[] input[i];
    }
}

/** Tests snip_string().
 * snip_string() should snip a given input string A from another string
 * B iff A is a prefix of B.
 */
WVTEST_MAIN("snip")
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

/** Tests strlwr().
 * strlwr() should convert all characters in an input string to lower case.
 */
WVTEST_MAIN("strlwr")
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

/** Tests strupr().
 * strupr() should convert all characters in an input string to upper case.
 */
WVTEST_MAIN("strupr")
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

/** Tests is_word().
 * is_word() should return whether or not all the characters in an
 * input string are alphanumeric (ie, the string is a 'word').
 */
WVTEST_MAIN("is_word")
{
    char *input[] = {"q1w2e3", "q!w@e#", "Q 86", "\t\n\r52", "hy-phen"};
    const bool desired[] = {true, false, false, false, false};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
        WVPASS(is_word(input[i]) == desired[i]);
}

/** Tests web_unescape().
 * web_unescape() should convert all url-encoded characters in an input
 * string (%xx) to their corresponding ASCII characters.
 */
WVTEST_MAIN("web_unescape")
{
    char *input = "%49+%6c%69%6b%65+%70%69%7a%7a%61%21";
    const char* desired = "I like pizza!";

    WVPASS(web_unescape(input) == desired);
}

/** Tests url_encode().
 * url_encode() should convert all appropriate ASCII characters to
 * their url-encoded equivalent.
 */
WVTEST_MAIN("url_encode")
{
    char *input = "http://www.free_email-account.com/~ponyman/mail.pl?name=\'to|\\|Y |)4|\\|Z4\'&pass=$!J83*p&folder=1N8()><";
    const char *desired = "http%3a//www.free_email-account.com/~ponyman/mail.pl%3fname%3d%27to%7c%5c%7cY%20%7c%294%7c%5c%7cZ4%27%26pass%3d%24%21J83%2ap%26folder%3d1N8%28%29%3e%3c";

    WVPASS(url_encode(input) == desired);
}

/** Tests backslash_escape().
 * backslash_escape() should escape all non-alphanumeric characters
 * with a leading backslash.
 */
WVTEST_MAIN("backslash_escape")
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

/** Tests strcount().
 * strcount() should return the number of occurences of a given
 * character in the input string.
 */
WVTEST_MAIN("strcount")
{
    char *input[] = {"abj;lewi", "lk327ga", "a87gai783a", "aaaaaaa", "ao8&ATO@a"};
    int desired[] = {1, 1, 3, 7, 2};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
        WVPASS(strcount(input[i], 'a') == desired[i]);
}

/** Tests encode_hostname_as_DN().
 * encode_hostname_as_DN should return a DName with dc components
 * containing each part of the given URI (host, domain, suffix, etc.)
 * and a cn component containing the entire URI.
 */
WVTEST_MAIN("encode_hostname")
{
    char *input[] = {"www.service.net", "www.you-can-too.com", "happybirthday.org", "www.canada.bigco.co.uk"};
    WvString desired[] = {"dc=www,dc=service,dc=net,cn=www.service.net", "dc=www,dc=you-can-too,dc=com,cn=www.you-can-too.com", "dc=happybirthday,dc=org,cn=happybirthday.org", "dc=www,dc=canada,dc=bigco,dc=co,dc=uk,cn=www.canada.bigco.co.uk"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
        WVPASS(encode_hostname_as_DN(input[i]) == desired[i]);
}

/** Tests nice_hostname().
 * nice_hostname() should replace underscores with hyphens, removing
 * duplicates, and remove any invalid (for a URI) characters, unless
 * they are the first character in the string, in which case they
 * should be replaced by 'x'.
 */
WVTEST_MAIN("nice_hostname")
{
    char *input[] = {"n-i_c-e", "@2COOL", "E\\/1|_.1", "ha--ha__ha"};
    WvString desired[] = {"n-i-c-e", "x2COOL", "E1-.1", "ha-ha-ha"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
        WVPASS(nice_hostname(input[i]) == desired[i]);
}

/** Tests getfilename().
 * getfilename() should return the bottom-most entry in a given
 * filename.
 */
WVTEST_MAIN("getfilename")
{
    char *input[] = {"/tmp/file", "file.ext", "../../.file.wot", "/snick/dir/", "/snick/dira/../dirb/file"};
    WvString desired[] = {"file", "file.ext", ".file.wot", "dir", "file"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
        WVPASS(getfilename(input[i]) == desired[i]);
}

/** Tests getdirname().
 * getdirname() should return the directory (hypothetically) containing
 * the bottom-most entry in a specified filename.
 */
WVTEST_MAIN("getdirname")
{
    char *input[] = {"/tmp/file", "file.ext", "../../.file.wot", "/snick/dir/", "/snick/dira/../dirb/file"};
    WvString desired[] = {"/tmp", ".", "../..", "/snick", "/snick/dira/../dirb"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
        WVPASS(getdirname(input[i]) == desired[i]);
}

/** Tests sizetoa().
 * sizetoa() should return an appropriate text description of the size
 * of a given number of blocks with a given blocksize.
 */
WVTEST_MAIN("sizetoa")
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

/** Tests lookup().
 * lookup() should return the index into an array of string where
 * the input string can be found (can be/not be a case sensitive search)
 * or -1 if the string is not in the array.
 */
WVTEST_MAIN("lookup")
{
    char *input[] = {"", "AbC", "a3k3 ", "abc", "ABC", NULL};
    
    WVPASS(lookup("abc", input, true) == 3);
    WVPASS(lookup("abc", input, false) == 1);
    WVPASS(lookup("ab", input, false) == -1);
    WVPASS(lookup("ABC", input, true) == 4);
    WVPASS(lookup("abcd", input, false) == -1);
}

/** Compares all the elements of a WvList.
 * Returns true iff the elements of lhs and rhs are pairwise equivalent
 * via the != operator.
 */
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

/** Tests strcoll_split().
 * strcoll_split() should return a list of strings that have been
 * extracted from an input string with fields separated by an arbitrary
 * number of given delimiters, with blank strings in the input being
 * ignored.
 */
WVTEST_MAIN("strcoll_split")
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

/** Tests strcoll_splitstrict().
 * strcoll_splitstrict() should return a list of strings that have been
 * extracted from an input string with fields separated by an arbitrary
 * number of given delimiters, with blank fields in the input causing
 * blank strings in the output.
 */
WVTEST_MAIN("strcoll_splitstrict")
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

/** Tests strcoll_join().
 * strcoll_join() should return a string formed by extracting all
 * elements from a list and concatenating them, separated by a given
 * string.
 */
WVTEST_MAIN("strcoll_join")
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

/** Tests strreplace().
 * strreplace() should replace all instances of a given string A in the
 * input string with a given string B.
 */
WVTEST_MAIN("replace")
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

/** Tests undupe().
 * undupe() should remove all consecutive instances of a given char in
 * the input string, replacing them with a single instance.
 */
WVTEST_MAIN("undupe")
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

/** Tests hostname().
 * hostname() should return the hostname of the local computer.
 */
WVTEST_MAIN("hostname")
{
    char host[1024];
    gethostname(host, sizeof(host));
    WVPASS(hostname() == host);
}

/** Tests fqdomainname().
 * fqdomainname() should return the fully-qualified domain name of the
 * local computer.  Of course, it's hard to tell if this worked or not.  At
 * least we should be able to trust the hostname, although the domainname
 * probably isn't right.
 */
WVTEST_MAIN("fqdomainname")
{
#if THIS_WERENT_TOTAL_CRAP
    char host[1024], *cptr;
    WvString n(fqdomainname());

    if(!!n) {
        cptr = strchr(n.edit(), '.');
        WVPASS(cptr); // FQDN must have a domain name part :)
        
        if (cptr)
        {
            WVPASS(strchr(cptr+1, '.')); // domain names have more than one part
            *cptr = 0; // now trim off the domain name part
        }

        gethostname(host, sizeof(host));

        printf("host='%s'  fqdomainname().host='%s'\n", host, n.cstr());
        WVPASS(n == host);
    } 
    else
        printf("Work around for Segfault");
#endif
}

/** Tests metriculate().
 * metriculate() should convert an input number to a string using
 * metric spacing conventions (ie. a space every three digits, going
 * from right to left).
 */
WVTEST_MAIN("metriculate")
{
    int input[] = {293, 218976, 1896234178, 12837, -28376, -24, -2873, -182736};
    const char *desired[] = {"293", "218 976", "1 896 234 178", "12 837", "-28 376", "-24", "-2 873", "-182 736"};

    for (unsigned int i = 0; i < sizeof(input) / sizeof(char *); ++i)
    {
        WvString result = metriculate(input[i]);
        if (!WVPASS(result == desired[i]))
            printf("   because [%s] != [%s]\n", result.cstr(), desired[i]);
    }
}

WVTEST_MAIN("locatestr")
{
    WvString big = "foobarman";
    
    WVPASS(locatestr(big, "foo") == &big.cstr()[0]);
    WVPASS(locatestr(big, "o") == &big.cstr()[1]);
    WVPASS(locatestr(big, "barman") == &big.cstr()[3]);

    WVFAIL(locatestr(big, "smarch"));
}

WVTEST_MAIN("afterstr")
{
    WvString big = "foobarman";

    WVPASS(afterstr(big, "foo") == "barman");
    WVPASS(afterstr(big, "o") == "obarman");
    WVPASS(afterstr(big, "man") == "");
    WVPASS(afterstr(big, "smarch") == "");
}

WVTEST_MAIN("beforestr")
{
    WvString big = "foobarman";

    WVPASS(beforestr(big, "foo") == "");
    WVPASS(beforestr(big, "o") == "f");
    WVPASS(beforestr(big, "man") == "foobar");
    WVPASS(beforestr(big, "smarch") == big);
}

WVTEST_MAIN("substr")
{
    WvString big = "foobarman";

    WVPASS(substr(big, 0, 9) == big);
    WVPASS(substr(big, 0, 10) == big);
    WVPASS(substr(big, 0, 6) == "foobar");
    WVPASS(substr(big, 5, 0) == "");
    WVPASS(substr(big, 10, 1) == "");
}
