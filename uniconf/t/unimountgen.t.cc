#include "wvtest.h"
#include "unimountgen.h"
#include "uniconf.h"
#include "unitempgen.h"

WVTEST_MAIN("mountgen basics")
{
    UniMountGen g;
    WVFAIL(g.haschildren("/"));
    
    g.mount("/", "null:", true);
    WVFAIL(g.haschildren("/"));
}

WVTEST_MAIN("mounting multiple generators")
{
    //basic stuff
    UniMountGen g;
    IUniConfGen *t1 = g.mount("/foo", "temp:", true);
    IUniConfGen *t2 = g.mount("/bar", "temp:", true);

    t1->set("bum", "boo");
    t2->set("dum", "far");
    WVPASSEQ(g.get("/foo/bum"), "boo");
    WVPASSEQ(g.get("/bar/dum"), "far");
    
    //nested generators
    g.set("/foo/gah", "goop");
    WVPASSEQ(t1->get("gah"), "goop");

    IUniConfGen *t3 = g.mount("/foo/mink", "temp:", true);
    t3->set("moo", "foo");
    WVPASSEQ(g.get("/foo/mink/moo"), "foo");
    WVFAIL(t1->get("mink/moo"));

    //generator t3 should take precedence
    t1->set("mink/moo", "cabbage");
    WVPASSEQ(g.get("/foo/mink/moo"), "foo");
    
    g.unmount(t3, true);
    WVPASSEQ(g.get("/foo/mink/moo"), "cabbage");

    WVFAIL(g.get("/moo"));
    t3 = g.mount("/", "temp:", true);
    t3->set("moo", "foo");
    WVPASSEQ(g.get("/moo"), "foo");
    /*FIXME: t3 should *not* take precedence, innermost generators should be first
     * since the generators should be sorted deepest first.
    WVPASSEQ(g.get("/foo/bum"), "boo");
    t3->set("/foo/bum", "fools");
    WVPASSEQ(g.get("/foo/bum"), "boo");
    */
}

WVTEST_MAIN("multiple generators - iterators")
{
    UniMountGen g;
    IUniConfGen *t3 = g.mount("/", "temp:", true);
    IUniConfGen *t1 = g.mount("/foo", "temp:", true);
    IUniConfGen *t2 = g.mount("/bar", "temp:", true);
        

    t1->set("bum", "foo");
    t1->set("bum/bum", "foo");
    t1->set("bim", "foo");
    t1->set("bam", "foo");
    t2->set("dum", "bar");
    t2->set("bum", "bar");
    t2->set("bum/gum", "bar");
    t2->set("bum/scum", "bar");
    t2->set("bum/scum/flum", "bar");
    t3->set("frump", "bung");

    UniMountGen::Iter *i = g.iterator("/foosball");
    WVFAIL(i);
    
    delete i;
    i = g.iterator("/foo");
    if (WVPASS(i))
    {
        int num_values = 0;
        for (i->rewind(); i->next(); )
        {
            WVPASSEQ(i->value(), "foo");
            num_values++;
        }
        WVPASSEQ(num_values, 3);
    }
    
    delete i; 
    i = g.iterator("/bar");
    if (WVPASS(i))
    {
        int num_values = 0;
        for (i->rewind(); i->next(); )
        {
            WVPASSEQ(i->value(), "bar");
            num_values++;
        }
        WVPASSEQ(num_values, 2);
    }
//FIXME: iterator on / producing 2 null results.
/*
    delete i; 
    i = g.iterator("/");
    if (WVPASS(i))
    {
        int num_values = 0;
        for (i->rewind(); i->next(); )
        {
            WVPASSEQ(i->value(), "bung");
            num_values++;
        }
        WVPASSEQ(num_values, 1);
    }*/
    
    delete i; 
    i = g.recursiveiterator("/foo");
    if (WVPASS(i))
    {
        int num_values = 0;
        for (i->rewind(); i->next(); )
        {
            WVPASSEQ(i->value(), "foo");
            num_values++;
        }
        WVPASSEQ(num_values, 4);
    }
    
    delete i; 
    i = g.recursiveiterator("/bar");
    if (WVPASS(i))
    {
        int num_values = 0;
        for (i->rewind(); i->next(); )
        {
            WVPASSEQ(i->value(), "bar");
            num_values++;
        }
        WVPASSEQ(num_values, 5);
    }
//FIXME: recursive iterator not working here at all
/*
    delete i; 
    i = g.recursiveiterator("/");
    if (WVPASS(i))
    {
        int num_values = 0;
        for (i->rewind(); i->next(); )
        {
            WVPASSEQ(i->value(), "");
            num_values++;
        }
        WVPASSEQ(num_values, 10);
    }*/
}
//FIXME: iterators can't get past areas where no generator is mounted
/*
WVTEST_MAIN("multiple generators - iterating with gaps")
{
    UniMountGen g;
    IUniConfGen *t1 = g.mount("/foo", "temp:", true);
    IUniConfGen *t2 = g.mount("/bar", "temp:", true);
        
    t1->set("bum", "foo");
    t1->set("bum/bum", "foo");
    t1->set("bim", "foo");
    t1->set("bam", "foo");
    t2->set("dum", "bar");
    t2->set("bum", "bar");
    t2->set("bum/gum", "bar");
    t2->set("bum/scum", "bar");
    t2->set("bum/scum/flum", "bar");
   
    //should be disregarding the fact that nothing is mounted on / and iterating
    UniMountGen::Iter *i = g.iterator("/");
    if (WVPASS(i))
    {
        int num_values = 0;
        for (i->rewind(); i->next(); )
        {
            if (num_values < 4)
                WVPASSEQ(i->value(), "foo");
            else
                WVPASSEQ(i->value(), "bar");
            num_values++;
        }
        WVPASSEQ(num_values, 9);
    }
}*/
