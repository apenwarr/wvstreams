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
        deletev input[i];
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
        deletev input[i];
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
        deletev input[i];
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
        deletev result;
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
        deletev input[i];
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
        deletev input[i];
    }

    WVPASS(strlwr(NULL) == NULL);
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
        deletev input[i];
    }
    WVPASS(strupr(NULL) == NULL);
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
    // Check various blocksizes
    {
        long blocks = 987654321;
        long blocksize = 1000000;
        char *desired[15] = {"987.7 TB", "98.8 TB", "9.9 TB", "987.7 GB",
            "98.8 GB", "9.9 GB", "987.7 MB", "98.8 MB", "9.9 MB", "987.7 KB",
            "98.8 KB", "9.9 KB", "987 bytes", "98 bytes", "9 bytes"};
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

    // Now check rounding
    WVPASS(sizetoa(0) == "0 bytes");
    WVPASS(sizetoa(1) == "1 bytes");
    WVPASS(sizetoa(42) == "42 bytes");
    WVPASS(sizetoa(666) == "666 bytes");
    WVPASS(sizetoa(949) == "949 bytes");
    WVPASS(sizetoa(999) == "999 bytes");
    WVPASS(sizetoa(1000) == "1.0 KB");
    WVPASS(sizetoa(1049) == "1.0 KB");
    WVPASS(sizetoa(1050) == "1.1 KB");
    WVPASS(sizetoa(1051) == "1.1 KB");
    WVPASS(sizetoa(1099) == "1.1 KB");
    WVPASS(sizetoa(1999) == "2.0 KB");
    WVPASS(sizetoa(949999) == "950.0 KB");
    WVPASS(sizetoa(999999) == "1.0 MB");
    WVPASS(sizetoa(1000000) == "1.0 MB");
    WVPASS(sizetoa(1049999) == "1.0 MB");
    WVPASS(sizetoa(1050000) == "1.1 MB");
    WVPASS(sizetoa(1050001) == "1.1 MB");
    WVPASS(sizetoa(1099999) == "1.1 MB");
    WVPASS(sizetoa(1999999) == "2.0 MB");
    WVPASS(sizetoa(949999999) == "950.0 MB");
    WVPASS(sizetoa(999999999) == "1.0 GB");
    WVPASS(sizetoa(1000000000) == "1.0 GB");
    WVPASS(sizetoa(1049999999) == "1.0 GB");
    WVPASS(sizetoa(1050000000) == "1.1 GB");
    WVPASS(sizetoa(1050000001) == "1.1 GB");
    WVPASS(sizetoa(1099999999) == "1.1 GB");
    WVPASS(sizetoa(1999999999) == "2.0 GB");
    WVPASS(sizetoa(2147483647) == "2.1 GB");
    WVPASS(sizetoa(0, 1000) == "0 bytes");
    WVPASS(sizetoa(1, 1000) == "1.0 KB");
    WVPASS(sizetoa(42, 1000) == "42.0 KB");
    WVPASS(sizetoa(666, 1000) == "666.0 KB");
    WVPASS(sizetoa(949, 1000) == "949.0 KB");
    WVPASS(sizetoa(999, 1000) == "999.0 KB");
    WVPASS(sizetoa(949999999, 1000) == "950.0 GB");
    WVPASS(sizetoa(999999999, 1000) == "1.0 TB");
    WVPASS(sizetoa(1000000000, 1000) == "1.0 TB");
    WVPASS(sizetoa(1049999999, 1000) == "1.0 TB");
    WVPASS(sizetoa(1050000000, 1000) == "1.1 TB");
    WVPASS(sizetoa(1050000001, 1000) == "1.1 TB");
    WVPASS(sizetoa(1099999999, 1000) == "1.1 TB");
    WVPASS(sizetoa(1999999999, 1000) == "2.0 TB");
    WVPASS(sizetoa(2147483647, 1000) == "2.1 TB");
    WVPASS(sizetoa(949999999, 1000000) == "950.0 TB");
    WVPASS(sizetoa(999999999, 1000000) == "1.0 PB");
    WVPASS(sizetoa(1000000000, 1000000) == "1.0 PB");
    WVPASS(sizetoa(1049999999, 1000000) == "1.0 PB");
    WVPASS(sizetoa(1050000000, 1000000) == "1.1 PB");
    WVPASS(sizetoa(1050000001, 1000000) == "1.1 PB");
    WVPASS(sizetoa(1099999999, 1000000) == "1.1 PB");
    WVPASS(sizetoa(1999999999, 1000000) == "2.0 PB");
    WVPASS(sizetoa(2147483647, 1000000) == "2.1 PB");
    WVPASS(sizetoa(949999999, 1000000000) == "950.0 PB");
    WVPASS(sizetoa(999999999, 1000000000) == "1.0 EB");
    WVPASS(sizetoa(1000000000, 1000000000) == "1.0 EB");
    WVPASS(sizetoa(1049999999, 1000000000) == "1.0 EB");
    WVPASS(sizetoa(1050000000, 1000000000) == "1.1 EB");
    WVPASS(sizetoa(1050000001, 1000000000) == "1.1 EB");
    WVPASS(sizetoa(1099999999, 1000000000) == "1.1 EB");
    WVPASS(sizetoa(1999999999, 1000000000) == "2.0 EB");
    WVPASS(sizetoa(2147483647, 1000000000) == "2.1 EB");
/* Since sizetoa only currently takes signed long longs, then we will not be
   able to test more than 2.1 exabytes.
    WVPASS(sizetoa(949999999, 1000000000000) == "950.0 EB");
    WVPASS(sizetoa(999999999, 1000000000000) == "1.0 EB");
    WVPASS(sizetoa(1000000000, 1000000000000) == "1.0 EB");
    WVPASS(sizetoa(1049999999, 1000000000000) == "1.0 EB");
    WVPASS(sizetoa(1050000000, 1000000000000) == "1.1 EB");
    WVPASS(sizetoa(1050000001, 1000000000000) == "1.1 EB");
    WVPASS(sizetoa(1099999999, 1000000000000) == "1.1 EB");
    WVPASS(sizetoa(1999999999, 1000000000000) == "2.0 EB");
    WVPASS(sizetoa(2147483647, 1000000000000) == "2.1 EB");
    WVPASS(sizetoa(949999999, 1000000000000000) == "950.0 EB");
    WVPASS(sizetoa(999999999, 1000000000000000) == "1000.0 YB");
    WVPASS(sizetoa(1000000000, 1000000000000000) == "1000.0 YB");
    WVPASS(sizetoa(1049999999, 1000000000000000) == "1050.0 YB");
    WVPASS(sizetoa(1050000000, 1000000000000000) == "1050.0 YB");
    WVPASS(sizetoa(1050000001, 1000000000000000) == "1050.0 YB");
    WVPASS(sizetoa(1099999999, 1000000000000000) == "1100.0 YB");
    WVPASS(sizetoa(1999999999, 1000000000000000) == "2000.0 YB");
    WVPASS(sizetoa(2147483647, 1000000000000000) == "2147.5 YB");
    WVPASS(sizetoa(949999999, 10000000000000000) == "9500.0 YB");
    WVPASS(sizetoa(999999999, 10000000000000000) == "10000.0 YB");
*/
}

/** Tests sizektoa(),
 * sizektoa() should convert an integer of kilobytes
 * into a string that describes how big it is, in a human-readable format.
 */
WVTEST_MAIN("sizektoa")
{
    WVPASS(sizektoa(0) == "0 KB");
    WVPASS(sizektoa(1) == "1 KB");
    WVPASS(sizektoa(42) == "42 KB");
    WVPASS(sizektoa(666) == "666 KB");
    WVPASS(sizektoa(949) == "949 KB");
    WVPASS(sizektoa(999) == "999 KB");
    WVPASS(sizektoa(1000) == "1.0 MB");
    WVPASS(sizektoa(1049) == "1.0 MB");
    WVPASS(sizektoa(1050) == "1.1 MB");
    WVPASS(sizektoa(1051) == "1.1 MB");
    WVPASS(sizektoa(1099) == "1.1 MB");
    WVPASS(sizektoa(1999) == "2.0 MB");
    WVPASS(sizektoa(949999) == "950.0 MB");
    WVPASS(sizektoa(999999) == "1.0 GB");
    WVPASS(sizektoa(1000000) == "1.0 GB");
    WVPASS(sizektoa(1049999) == "1.0 GB");
    WVPASS(sizektoa(1050000) == "1.1 GB");
    WVPASS(sizektoa(1050001) == "1.1 GB");
    WVPASS(sizektoa(1099999) == "1.1 GB");
    WVPASS(sizektoa(1999999) == "2.0 GB");
    WVPASS(sizektoa(949999999) == "950.0 GB");
    WVPASS(sizektoa(999999999) == "1.0 TB");
    WVPASS(sizektoa(1000000000) == "1.0 TB");
    WVPASS(sizektoa(1049999999) == "1.0 TB");
    WVPASS(sizektoa(1050000000) == "1.1 TB");
    WVPASS(sizektoa(1050000001) == "1.1 TB");
    WVPASS(sizektoa(1099999999) == "1.1 TB");
    WVPASS(sizektoa(1999999999) == "2.0 TB");
    WVPASS(sizektoa(2147483647) == "2.1 TB");
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

    typename WvList<T>::Iter l(lhs), r(rhs);
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

WVTEST_MAIN("cstr_escape")
{
    WVPASS(cstr_escape(NULL, 0).isnull());
    WVPASS(cstr_escape("", 0) == "\"\"");
    WVPASS(cstr_escape("\"", 1) == "\"\\\"\"");
    WVPASS(cstr_escape("\\", 1) == "\"\\\\\"");
    WVPASS(cstr_escape("\a", 1) == "\"\\a\"");
    WVPASS(cstr_escape("\b", 1) == "\"\\b\"");
    WVPASS(cstr_escape("\r", 1) == "\"\\r\"");
    WVPASS(cstr_escape("\t", 1) == "\"\\t\"");
    WVPASS(cstr_escape("\n", 1) == "\"\\n\"");
    WVPASS(cstr_escape("\v", 1) == "\"\\v\"");
    WVPASS(cstr_escape("\0", 1) == "\"\\0\"");
    WVPASS(cstr_escape("a", 1) == "\"a\"");
    WVPASS(cstr_escape("\xFF", 1) == "\"\\xFF\"");
}

WVTEST_MAIN("cstr_escape_unescape")
{
    int i;
    const size_t max_size = 256;
    char orig_data[max_size], data[max_size];
    size_t size;
   
    for (i=0; i<256; ++i)
        orig_data[i] = (char)i;
   
    // Regular escapes
    WVPASS(cstr_unescape(cstr_escape(orig_data, max_size), data, max_size, size)
            && size == max_size && memcmp(orig_data, data, size) == 0);
   
    // With TCL escapes
    WVPASS(cstr_unescape(cstr_escape(orig_data, max_size, CSTR_TCLSTR_ESCAPES),
                data, max_size, size, CSTR_TCLSTR_ESCAPES)
            && size == max_size && memcmp(orig_data, data, size) == 0);
}

WVTEST_MAIN("cstr_unescape")
{
    const size_t max_size = 16;
    char data[max_size];
    size_t size;
   
    // Tests for detection of misformatting
    WVFAIL(cstr_unescape(WvString::null, data, max_size, size) || size);
    WVPASS(cstr_unescape("", data, max_size, size) && size == 0);
    WVFAIL(cstr_unescape("garbage", data, max_size, size) || size);
    WVFAIL(cstr_unescape("\"", data, max_size, size) || size);
    WVPASS(cstr_unescape(" \"\" ", data, max_size, size) && size == 0);
    WVPASS(cstr_unescape("\"\" ", data, max_size, size) && size == 0);
    WVFAIL(cstr_unescape("\"\" \"", data, max_size, size) || size);

    // Tests for correcly formatted strings with enough space
    WVPASS(cstr_unescape("\"\"", data, max_size, size) && size == 0);
    WVPASS(cstr_unescape("\"four\"", data, max_size, size)
            && size == 4 && memcmp(data, "four", size) == 0);
    WVPASS(cstr_unescape("\"sixteencharacter\"", data, max_size, size)
            && size == 16 && memcmp(data, "sixteencharacter", size) == 0);

    // Test for correctly formatted string without enough space
    WVPASS(!cstr_unescape("\"nsixteencharacter\"", data, max_size, size)
            && size == 17 && memcmp(data, "nsixteencharacter", max_size) == 0);
    
    // Test for passing data as a NULL
    WVPASS(cstr_unescape("\"four\"", NULL, max_size, size) && size == 4);

    // Tests for concatenation
    WVPASS(cstr_unescape(" \r\"one\" \t\"two\"\v\n", data, max_size, size)
            && size == 6 && memcmp(data, "onetwo", size) == 0);
}

void foo(WvStringParm s)
{
    wvcon->print("foo: s is `%s'\n", s);
    WvString str(s);
    str = trim_string(str.edit());
    wvcon->print("foo: str is `%s'\n", str);
}

WVTEST_MAIN("WvString: circular reference")
{
    {
	char cstr[] = "Hello";
	WvString CStr(cstr);
	CStr = CStr.cstr();
	wvcon->print("cstr is `%s'\n", CStr);
    }

    {
	WvString s("Law");
	s = s.cstr();
	wvcon->print("s is `%s'\n", s);
    }

    {
	WvString str("  abc ");
	str = trim_string(str.edit());
	wvcon->print("str is `%s'\n", str);
	str.append("a");
	wvcon->print("str is `%s'\n", str);
	str.append("lalalalala");
	wvcon->print("str is `%s'\n", str);
    }

    foo("def ");

    foo("     ");
}


WVTEST_MAIN("secondstoa")
{
    WVPASSEQ(secondstoa(0), "0 seconds");
    WVPASSEQ(secondstoa(1), "1 second");
    WVPASSEQ(secondstoa(2), "2 seconds");
    WVPASSEQ(secondstoa(59), "59 seconds");
    WVPASSEQ(secondstoa(60), "1 minute");
    WVPASSEQ(secondstoa(2*60), "2 minutes");
    WVPASSEQ(secondstoa(59*60+1), "59 minutes");
    WVPASSEQ(secondstoa(3600), "1 hour");
    WVPASSEQ(secondstoa(3600 + 60), "1 hour and 1 minute");
    WVPASSEQ(secondstoa(3600 + 2*60), "1 hour and 2 minutes");
    WVPASSEQ(secondstoa(2*3600), "2 hours");
    WVPASSEQ(secondstoa(2*3600 + 60), "2 hours and 1 minute");
    WVPASSEQ(secondstoa(2*3600 + 2*60), "2 hours and 2 minutes");
    WVPASSEQ(secondstoa(23*3600 + 59*60), "23 hours and 59 minutes");
    WVPASSEQ(secondstoa(23*3600 + 59*60 + 59), "23 hours and 59 minutes");
    WVPASSEQ(secondstoa(24*3600), "1 day");
    WVPASSEQ(secondstoa(24*3600 + 59), "1 day");
    WVPASSEQ(secondstoa(24*3600 + 3600), "1 day and 1 hour");
    WVPASSEQ(secondstoa(24*3600 + 3600 + 59), "1 day and 1 hour");
    WVPASSEQ(secondstoa(24*3600 + 2*3600), "1 day and 2 hours");
    WVPASSEQ(secondstoa(24*3600 + 3600 + 60), "1 day, 1 hour and 1 minute");
    WVPASSEQ(secondstoa(24*3600 + 3600 + 2*60), "1 day, 1 hour and 2 minutes");
    WVPASSEQ(secondstoa(24*3600 + 2*3600 + 60), "1 day, 2 hours and 1 minute");
    WVPASSEQ(secondstoa(24*3600 + 2*3600 + 2*60), 
            "1 day, 2 hours and 2 minutes");
    WVPASSEQ(secondstoa(24*3600 + 23*3600 + 59*60 + 59), 
            "1 day, 23 hours and 59 minutes");
    WVPASSEQ(secondstoa(2*24*3600), "2 days");
    WVPASSEQ(secondstoa(2*24*3600 + 3600), "2 days and 1 hour");
    WVPASSEQ(secondstoa(2*24*3600 + 2*3600), "2 days and 2 hours");
    WVPASSEQ(secondstoa(2*24*3600 + 3600 + 60), "2 days, 1 hour and 1 minute");
    WVPASSEQ(secondstoa(2*24*3600 + 3600 + 2*60), 
            "2 days, 1 hour and 2 minutes");
    WVPASSEQ(secondstoa(2*24*3600 + 2*3600 + 60), 
            "2 days, 2 hours and 1 minute");
    WVPASSEQ(secondstoa(2*24*3600 + 2*3600 + 2*60), 
            "2 days, 2 hours and 2 minutes");
    WVPASSEQ(secondstoa(2*24*3600 + 23*3600 + 59*60 + 59), 
            "2 days, 23 hours and 59 minutes");
    WVPASSEQ(secondstoa(10*24*3600), "10 days");
}

