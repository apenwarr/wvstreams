#include "wvtest.h"
#include "wvstream.h"
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
    // basic stuff
    UniMountGen g;
    IUniConfGen *t1 = g.mount("/foo", "temp:", true);
    IUniConfGen *t2 = g.mount("/bar", "temp:", true);

    t1->set("bum", "boo");
    t2->set("dum", "far");
    WVPASSEQ(g.get("/foo/bum"), "boo");
    WVPASSEQ(g.get("/bar/dum"), "far");
    
    // nested generators
    g.set("/foo/gah", "goop");
    WVPASSEQ(t1->get("gah"), "goop");

    IUniConfGen *t3 = g.mount("/foo/mink", "temp:", true);
    t3->set("moo", "foo");
    WVPASSEQ(g.get("/foo/mink/moo"), "foo");
    WVFAIL(t1->get("mink/moo"));

    // generator t3 should take precedence
    t1->set("mink/moo", "cabbage");
    WVPASSEQ(g.get("/foo/mink/moo"), "foo");
    
    g.unmount(t3, true);
    WVPASSEQ(g.get("/foo/mink/moo"), "cabbage");

    WVFAIL(g.get("/moo"));
    t3 = g.mount("/", "temp:", true);
    t3->set("moo", "foo");
    WVPASSEQ(g.get("/moo"), "foo");
    
    /* FIXME: t3 should *not* take precedence, innermost generators should be first
     * since the generators should be sorted deepest first.
     */
#if 0
    WVPASSEQ(g.get("/foo/bum"), "boo");
    t3->set("/foo/bum", "fools");
    WVPASSEQ(g.get("/foo/bum"), "boo");
#endif
}

WVTEST_MAIN("multiple generators - iterators")
{
    UniMountGen g;
    IUniConfGen *t3 = g.mount("/", "temp:", true);
    IUniConfGen *t1 = g.mount("/foo", "temp:", true);
    IUniConfGen *t2 = g.mount("/bar", "temp:", true);
        

    t1->set("/", "bung");
    t1->set("bum", "foo");
    t1->set("bum/bum", "foo");
    t1->set("bim", "foo");
    t1->set("bam", "foo");
    t2->set("/", "bung");
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

#if 0 // FIXME: unimountgen iterates badly through nested mounts
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
        WVPASSEQ(num_values, 3);
    }
#endif
    
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

#if 0 // FIXME: unimountgen deals badly with nested mounts
    delete i; 
    i = g.recursiveiterator("/");
    if (WVPASS(i))
    {
        int num_values = 0;
        for (i->rewind(); i->next(); )
            num_values++;
        WVPASSEQ(num_values, 10);
    }
#endif
}

WVTEST_MAIN("multiple generators - iterating with gaps")
{
    UniMountGen g; // nothing mounted on '/'
    IUniConfGen *t1 = g.mount("/foo", "temp:", true);
    IUniConfGen *t2 = g.mount("/bar", "temp:", true);
        
    t1->set("/", "foo");
    t1->set("bum", "foo");
    t1->set("bum/bum", "foo");
    t1->set("bim", "foo");
    t1->set("bam", "foo");
    t2->set("/", "bar");
    t2->set("dum", "bar");
    t2->set("bum", "bar");
    t2->set("bum/gum", "bar");
    t2->set("bum/scum", "bar");
    t2->set("bum/scum/flum", "bar");
   
    // should be disregarding the fact that nothing is mounted on / and
    // iterating anyway
    UniMountGen::Iter *i = g.iterator("/");
    if (WVPASS(i))
    {
        int num_values = 0;
        for (i->rewind(); i->next(); )
        {
	    if (!WVPASS(i->value() == "foo" || i->value() == "bar"))
		wvcon->print("...value was '%s'\n", i->value());
            num_values++;
        }
        WVPASSEQ(num_values, 2);
    }
}
