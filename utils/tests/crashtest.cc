#include "wvcrash.h"
#include <assert.h>
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
    if (argc > 2)
    {
	printf("wvcrash_setup(argv[0], argv[2]);\n");
	wvcrash_setup(argv[0], argv[2]);
    }
    else
    {
	printf("wvcrash_setup(argv[0]);\n");
	wvcrash_setup(argv[0]);
    }

    printf("free(malloc(1));\n");
    free(malloc(1));

    if (argc > 1)
    {
	printf("sig = atoi(argv[1]);\n");
        sig = atoi(argv[1]);
    }

    printf("assert(argc < 4);\n");
    assert(argc < 4);
    
    printf("xfunc();\n");
    xfunc();
}
