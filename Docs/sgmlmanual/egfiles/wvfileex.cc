/*
 * A WvFile example.
 *
 * Some text about this example...
 */

#include <wvfile.h>

int main()
{
    WvFile infile("/etc/passwd", O_RDONLY);
    char *s;
    
    while (infile.isok() && (s = infile.getline(-1)) != NULL)
    	wvcon->print("%s\n", s);
}
