#include "wvcrash.h"
#include <unistd.h>
#include <malloc.h>
#include <stdio.h>

int glob = 5;

void zfunc()
{
    glob = 99;
}

void yfunc()
{
    glob = 6;
    zfunc();
    *(char *)NULL = 0;
    glob = 9;
}

void xfunc()
{
    glob = 7;
    yfunc();
    glob = 8;
}

int main(int argc, char **argv)
{
    wvcrash_setup(argv[0]);
    free(malloc(1));

    xfunc();
}