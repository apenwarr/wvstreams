#include "wvtest.h"
#include "wvbuf.h"
#include <stdio.h>
#include <stdlib.h>

//#define DEBUG_STRESS

WVTEST_MAIN("InPlaceBuffer")
{
    WvInPlaceBuf b(1024);
    char *s;
    WVPASS(b.used() == 0);
    WVPASS(b.free() == 1024);
    WVPASS(b.strchr('c') == b.strchr((unsigned char)'c'));
    
    b.put("frogs on ice", 13);
    WVPASS(b.used() == 13);
    WVPASS(b.free() == 1011);
    WVPASS(b.strchr('c') == b.strchr((unsigned char)'c'));            
    
    s = (char *)b.get(8);
    WVFAIL(strcmp(s, "frogs on ice"));
    WVPASS(b.used() == 5);
    WVPASS(b.free() == 1011);
    WVPASS(b.strchr('c') == b.strchr((unsigned char)'c'));                
    
    s = (char *)b.get(5);
    WVFAIL(strcmp(s, " ice"));
    
    WVPASS(b.used() == 0);
    WVPASS(b.free() == 1011);
    WVPASS(b.strchr('c') == b.strchr((unsigned char)'c'));            
    
    b.zap();
    WVPASS(b.used() == 0);
    WVPASS(b.free() == 1024);
    WVPASS(b.strchr('c') == b.strchr((unsigned char)'c')); 
}


WVTEST_MAIN("DynBuf")
{
    WvDynBuf b;
    char *s;
    WVPASS(b.used() == 0);
    WVPASS(b.strchr('c') == b.strchr((unsigned char)'c'));
    
    b.put("frogs on ice", 13);
    WVPASS(b.used() == 13);
    WVPASS(b.strchr('c') == b.strchr((unsigned char)'c'));    
    
    b.put("frogs on rice", 14);
    WVPASS(b.used() == 27);
    WVPASS(b.strchr('c') == b.strchr((unsigned char)'c'));    
    
    s = (char *)b.get(8);
    if (WVFAIL(strcmp(s, "frogs on ice")))
	printf("   because [%s] != [frogs on ice]\n", s);
    
    WVPASS(b.used() == 19);
    WVPASS(b.strchr('c') == b.strchr((unsigned char)'c'));    
    
    b.put("frogs on bryce", 15);
    s = (char *)b.get(5);
    if (WVFAIL(strcmp(s, " ice")))
	printf("   because [%s] != [ ice]\n", s);
    
    WVPASS(b.used() == 29);
    WVPASS(b.strchr('c') == b.strchr((unsigned char)'c'));    
    
    s = (char *)b.get(16);
    if (WVFAIL(strcmp(s, "frogs on rice")))
	printf("   because [%s] != [frogs on rice]\n", s);
    
    WVPASS(b.used() == 13);
    WVPASS(b.strchr('c') == b.strchr((unsigned char)'c'));
    
    b.unget(12);
    WVPASS(b.used() == 25);
    WVPASS(b.strchr('c') == b.strchr((unsigned char)'c'));    
    
    s = (char *)b.get(11);
    if (WVFAIL(strcmp(s, "s on rice")))
	printf("   because [%s] != [s on rice]\n", s);
    
    s = (char *)b.get(14);
    if (WVFAIL(strcmp(s, "rogs on bryce")))
	printf("   because [%s] != [rogs on bryce]\n", s);
    
    WVPASS(b.used() == 0);
    WVPASS(b.strchr('c') == b.strchr((unsigned char)'c'));
}


WVTEST_MAIN("unpeek")
{
    WvDynBuf buf;
    
    buf.alloc(10000);
    WVPASSEQ(buf.used(), 10000);
    buf.alloc(100000);
    WVPASSEQ(buf.used(), 110000);
    
    buf.get(8000);
    WVPASSEQ(buf.used(), 102000);
    
    buf.mutablepeek(0, 2000);
    WVPASSEQ(buf.used(), 102000);
    
    buf.mutablepeek(0, 102000);
    WVPASSEQ(buf.used(), 102000);
}


WVTEST_MAIN("Stress")
{
/* FIXME: find out how we'd know if the buffer failed the stress test, if there
 * are any more hints than just *not crashed*
    // Buffer Stress Test
    {
        WvDynBuf b;
        char *s, xx[1024];
        size_t in, i, max, total;
        
        in = max = total = 0;
        while (1)
        {
            i = random() % sizeof(xx);
            s = (char *)b.alloc(i);
            memcpy(s, xx, i);
#ifdef DEBUG_STRESS
            fprintf(stderr, "alloc(%d)\n", i);
#endif
            in += i;
            total += i;
            size_t lastalloc = i;
            
            i = random() % sizeof(xx);
            if (i > lastalloc)
                i = lastalloc;
#ifdef DEBUG_STRESS
            fprintf(stderr, "unalloc(%d)\n", i);
#endif
            b.unalloc(i);
            in -= i;
	
            i = random() % sizeof(xx);
            if (i > in)
                i = in;
#ifdef DEBUG_STRESS
            fprintf(stderr, "get(%d)\n", i);
#endif
            b.get(i);
            in -= i;
            
            i = random() % sizeof(xx);
#ifdef DEBUG_STRESS
            fprintf(stderr, "put(%d)\n", i);
#endif
            b.put(xx, i);
            in += i;
            total += i;
            
            i = random() % sizeof(xx);
            if (i > in)
                i = in;
#ifdef DEBUG_STRESS
            fprintf(stderr, "get(%d)\n", i);
#endif
            b.get(i);
            in -= i;
            size_t lastput = i;
            
            i = random() % sizeof(xx);
            if (i > lastput)
                i = lastput;
#ifdef DEBUG_STRESS
            fprintf(stderr, "unget(%d)\n", i);
#endif
            b.unget(i);
            in += i;
            
            assert(b.used() == in);
            if (b.used() > max)
            {
                max = b.used();
                printf("New max: %u bytes in subuffers after %u bytes\n",
                        max, total);
            }
#ifdef DEBUG_STRESS
            fprintf(stderr, "[%6d]", in);
#endif
        }
    }*/
}
