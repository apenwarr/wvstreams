#include "wvcrash.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

#ifndef  ISDARWIN
# include <malloc.h>
#endif

int glob = 5;
int sig = 0;

void zfunc()
{
    glob = 99;
}

void yfunc()
{
    glob = 6;
    zfunc();
    
    if (sig == 0)
        *(char *)NULL = 0;
    else
        kill(getpid(), sig);
    
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

    if (argc > 1)
        sig = atoi(argv[1]);
    
    xfunc();
}
