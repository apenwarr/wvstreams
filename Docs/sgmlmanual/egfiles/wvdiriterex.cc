/*
 * A WvDirIter example.
 *
 * Takes a directory on the command line, and
 * prints everything it sees.
 *
 */

#include "wvdiriter.h"

int main()
{
    WvString dirname(".");
    // or WvString dirname("/home/user/");
    // dirname contains the directory you want to list

    bool     recurse = false;
    // If true, do recursively

    WvDirIter i( dirname, recurse );

    for( i.rewind(); i.next(); ) {
        printf( "%s \n", (const char *) i->fullname);
    }
    // prints something like:
    // ./a.txt
    // ./b.txt
    // ./c.txt

    return( 0 );
}
