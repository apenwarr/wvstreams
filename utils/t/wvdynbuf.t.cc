#include "wvbuf.h"
#include "wvtest.h"
#include "wvstrutils.h"


WVTEST_MAIN("DynBuf")
{
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

    {
        WvDynBuf buf;
        buf.put("get ", 4);
        buf.put("not ", 4);
        buf.put("this", 4);
        
        WvString s;
        s.append((char *)buf.get(4));
        buf.skip(4);
        s.append((char *)buf.get(4));
        
        WVPASS(strcmp(s.cstr(), "Get this"));
    }

    {
        WvDynBuf big;
        char bigbuf[1024];
        memset(bigbuf, 'c', 1024);
        for (int x=0; x < 768; x++)
            big.put(bigbuf, 1024);
        WVPASSEQ(big.used(), 786432);
    
        bool ok = true;
        for (int x=0; x < 1024; x++)
        {
            if (strncmp((const char *)big.get(768), bigbuf, 768))
                ok = false;
        }
        WVPASS(ok);
    }
}


const int NUM_ZEROS = 4096;

// this test makes sure that one can grab large chunks of data from a dynbuf
WVTEST_MAIN("dynbuf contiguous getting")
{
    WvDynBuf b;    

    WvStringList outlines;
    for (int j=0; j<3; j++) 
    {
        outlines.append("Version: 4");
        outlines.append("Authenticate: foo");
        char zeros[NUM_ZEROS];
        memset(zeros, (int)'0', NUM_ZEROS);
        outlines.append(zeros);
    }

    WvStringList::Iter iter(outlines);
    for (iter.rewind(); iter.next();)
    {
        b.put(iter().cstr(), iter().len());
        b.put('\n');
    }

    for (iter.rewind(); iter.next();) 
    {
        size_t i = 0;
        i = b.strchr('\n');
        if (i > 0) 
        {
            char *eol = (char *)b.mutablepeek(i - 1, 1);
            assert(eol);
            *eol = 0;
            WvString bar(const_cast<char*>((const char *)b.get(i)));
            WVPASS(bar == iter());
        }
    }
}

