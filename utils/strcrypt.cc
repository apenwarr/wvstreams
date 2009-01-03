#include "strutils.h"
#ifndef MACOS
  #include <crypt.h>
#endif
#include <unistd.h>
#include <stdlib.h>

/**.
 * Before calling this function, you should call srandom().
 * When 2 identical strings are encrypted, they will not return the same
 * encryption. Also, str does not need to be less than 8 chars as UNIX crypt
 * says, although it only works on the first 8 characters.
 */
WvString passwd_crypt(const char *str)
{
    static char saltchars[] =
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
    char salt[3], *result;

    salt[0] = saltchars[random() % (sizeof(saltchars) - 1)];
    salt[1] = saltchars[random() % (sizeof(saltchars) - 1)];
    salt[2] = 0;

    result = crypt(str, salt);
    if (!result)
	return "*";

    WvString s(result);
    return s;
}

/**.
 * Before calling this function, you should call srandom().
 * When 2 identical strings are encrypted, they will not return the same
 * encryption. Also, str does not need to be less than 8 chars as we're
 * using the glibc md5 algorithm.
 */
WvString passwd_md5(const char *str)
{
    static char saltchars[] =
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
    char salt[12], *result;

    salt[0] = '$';
    salt[1] = '1';
    salt[2] = '$';

    for (int i = 3; i < 11; ++i)
	salt[i] = saltchars[random() % (sizeof(saltchars) - 1)];

    salt[11] = 0;

    result = crypt(str, salt);
    if (!result)
	return "*";

    WvString s(result);
    return s;
}
