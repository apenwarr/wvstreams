#include "wvtest.h"
#include "wvmagicloopback.h"
#include "wvfork.h"

#include <stdio.h>

WVTEST_MAIN("WvMagicLoopback Sanity") 
{
    WvMagicLoopback ml(1024);
    
    if (wvfork() == 0)
    {
    	int i;
    	
    	for (i=0; i<1024; ++i)
    	{
    	    while (!ml.iswritable());
    	    
    	    ml.write(&i, sizeof(i));
    	}
    	
    	_exit(0);
    }
    else
    {
    	int i, maybe_i;
    	
    	for (i=0; i<1024; ++i)
    	{
    	    while (!ml.isreadable());
    	    
    	    ml.read(&maybe_i, sizeof(maybe_i));
    	    
    	    if (i != maybe_i)
    	    {
    	    	WVPASS(i == maybe_i);
    	    	break;
    	    }
    	}
    }
    
    wait();
}

WVTEST_MAIN("WvMagicLoopback Race") 
{
    WvMagicLoopback ml(1);
    const int iter = 1 << 16;
    
    if (wvfork() == 0)
    {
    	int i;
    	
    	for (i=0; i<iter; ++i)
    	    ml.write("", 1);
    	
    	_exit(0);
    }
    else
    {
    	int i;
    	
    	for (i=0; i<iter; ++i)
        {
            char tmp;

            if (ml.select(1000))
                ml.read(&tmp, 1);
            else break;
        }

        WVPASS(i == iter);
    }
    
    wait();
}
