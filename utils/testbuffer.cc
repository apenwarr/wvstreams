/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * WvBuffer test program.  Comments below indicate correct results.
 *
 */

#include "wvbuffer.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    WvMiniBuffer b(1024);
    WvBuffer bb;
    char *s, xx[1024];
    size_t in, i, max, total;
    
    printf("MINIBUFFER TEST\n");
    printf("A %20u used, %u free, offset %d/%d\n",
	   b.used(), b.free(), b.strchr('c'), b.strchr((unsigned char)'c'));

    b.put("frogs on ice", 13);
    printf("B %20u used, %u free, offset %d/%d\n",
	   b.used(), b.free(), b.strchr('c'), b.strchr((unsigned char)'c'));
    
    s = (char *)b.get(8);
    printf("C s: %s\n", s);
    
    printf("D %20u used, %u free, offset %d/%d\n",
	   b.used(), b.free(), b.strchr('c'), b.strchr((unsigned char)'c'));

    s = (char *)b.get(5);
    printf("E s: %s\n", s);
    
    printf("F %20u used, %u free, offset %d/%d\n",
	   b.used(), b.free(), b.strchr('c'), b.strchr((unsigned char)'c'));
    printf("\n");
    
    printf("BUFFER TEST\n");
    
    printf("A %20u used, offset %d/%d\n",
	   bb.used(), bb.strchr('c'), bb.strchr((unsigned char)'c'));
    
    bb.put("frogs on ice", 13);
    printf("B %20u used, offset %d/%d\n",
	   bb.used(), bb.strchr('c'), bb.strchr((unsigned char)'c'));
    
    bb.put("frogs on rice", 14);
    printf("C %20u used, offset %d/%d\n",
	   bb.used(), bb.strchr('c'), bb.strchr((unsigned char)'c'));
    
    s = (char *)bb.get(8);
    printf("D s: %s\n", s);  // "frogs on ice"
    
    printf("E %20u used, offset %d/%d\n",
	   bb.used(), bb.strchr('c'), bb.strchr((unsigned char)'c'));
    
    bb.put("frogs on bryce", 15);
    s = (char *)bb.get(5);
    printf("F s: %s\n", s);  // " ice"

    printf("G %20u used, offset %d/%d\n",
	   bb.used(), bb.strchr('c'), bb.strchr((unsigned char)'c'));
    
    s = (char *)bb.get(16);
    printf("H s: %s\n", s);  // "frogs on rice"
    
    printf("I %20u used, offset %d/%d\n",
	   bb.used(), bb.strchr('c'), bb.strchr((unsigned char)'c'));

    bb.unget(12);
    printf("I2 %19u used, offset %d/%d\n",
	   bb.used(), bb.strchr('c'), bb.strchr((unsigned char)'c'));
    
    s = (char *)bb.get(11);
    printf("J s: %s\n", s);  // "s on rice"
    
    s = (char *)bb.get(14);
    printf("J2 s: %s\n", s);  // "rogs on bryce"
    
    printf("K %20u used, offset %d/%d\n",
	   bb.used(), bb.strchr('c'), bb.strchr((unsigned char)'c'));
    printf("\n");
    
    printf("BUFFER STRESS\n");
    in = max = total = 0;
    while (1)
    {
	i = random() % sizeof(xx);
	s = (char *)bb.alloc(i);
	memcpy(s, xx, i);
	in += i;
	total += i;
	
	i = random() % sizeof(xx);
	if (i > in)
	    i = in;
	bb.get(i);
	in -= i;
	
	i = random() % sizeof(xx);
	bb.put(xx, i);
	in += i;
	total += i;

	i = random() % sizeof(xx);
	if (i > in)
	    i = in;
	bb.get(i);
	in -= i;
	
	if (bb.used() > max)
	{
	    max = bb.used();
	    printf("New max: %u bytes in %d minibuffer(s) after %u bytes\n",
		   max, bb.num_of_bufs(), total);
	}
	
	fprintf(stderr, "[%6d]", in);
    }

    return 0;
}
