#include "strutils.h"
#ifdef ISLINUX
#include <crypt.h>
#endif

#include <unistd.h>
#include <stdlib.h>

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
    s.unique();
    return s;
}
