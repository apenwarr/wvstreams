#include <uniconf.h>
#include <unisecuregen.h>
#include <unipermgen.h>

#include <wvstring.h>
#include <wvstream.h>
#include <wvlog.h>
#include <wvlogrcv.h>

#include <unistd.h>

/**
 * FIXME: this doesn't test the following:
 * - exec permissions
 * - drilldown (ie. intermediate keys missing exec)
 * - default permissions (ie. users / * / foo) 
 */

struct Objects
{
    UniConfRoot root;
    UniConf u;
    UniPermGen *p;
    UniSecureGen *s;

    Objects(UniPermGen *_p, UniSecureGen *_s) :
        u(root), p(_p), s(_s)
    {
        u.mountgen(s);
    }
};


Objects *setup(WvStringParm perms)
{
    // populate secure.ini
    {
        // FIXME: should maybe not overwrite this if it exists
        unlink("secure.ini");
        UniConfRoot root;
        UniConf u(root);
        u.mount("ini:secure.ini");

        for (int user = 0; user < 8; user++)
            for (int group = 0; group < 8; group++)
                for (int world = 0; world < 8; world++)
                {
                    WvString key("%s%s%s", user, group, world);
                    u[key].set(key);
                }

        u.commit();
    }

    // create and populate the perms gen
    UniPermGen *p = new UniPermGen(perms);
    UniSecureGen *s = new UniSecureGen("ini:secure.ini", p);
    
    for (int user = 0; user < 8; user++)
        for (int group = 0; group < 8; group++)
            for (int world = 0; world < 8; world++)
            {
                WvString key("%s%s%s", user, group, world);
                p->setowner(key, "clampy");
                p->setgroup(key, "cloggers");
                p->chmod(key, user, group, world);
            }

    // create the uniconf obj and mount the generators
    Objects *o = new Objects(p, s);
    return o;
}


void teardown(Objects *o)
{
    delete o;
    unlink("secure.ini");
}


void printheader(WvStringParm h)
{
    WvString header("\n\nTEST %s", h);
    wvcon->print("%s\n",header);
    
    for (size_t i = 0; i < header.len(); i++)
        wvcon->print("=");
    wvcon->print("\n\n");
}


void printfooter(bool pass)
{
    WvString footer("===   TEST %s   ===", pass ? "PASSED" : "FAILED");
    wvcon->print("%s\n",footer);
}


bool testget(const UniConf &u, WvStringParm key, bool expectsucc)
{
    WvString expect = expectsucc ? key : WvString("nothing");
    WvString got = u[key].get("nothing");
    if (expect != got)
        wvcon->print("FAILED: get %s expected %s, got %s\n", key, expect, got);
    return (expect == got);
}


bool testset(const UniConf &u, WvStringParm key, bool expectsucc)
{
    WvString val = "test";
    WvString orig = u[key].get();
    WvString expect = expectsucc ? val : orig;
    u[key].set(val);
    WvString got = u[key].get();
    if (got != expect)
        wvcon->print("FAILED: set %s expected %s, got %s (orig was %s)\n",
                key, expect, got, orig);
    return (got == expect);
}


bool testreadable(const UniConf &u, bool none, bool o, bool g, bool w, bool og,
        bool ow, bool gw, bool ogw)
{
    bool pass = true;

    //readable:   4567
    //unreadable: 0123

    // all unreadable
    pass = testget(u, "000", none) && pass;
    pass = testget(u, "123", none) && pass;

    // user readable
    pass = testget(u, "400", o) && pass;
    pass = testget(u, "700", o) && pass;
    pass = testget(u, "512", o) && pass;

    // group readable
    pass = testget(u, "040", g) && pass;
    pass = testget(u, "070", g) && pass;
    pass = testget(u, "152", g) && pass;

    // world readable
    pass = testget(u, "004", w) && pass;
    pass = testget(u, "007", w) && pass;
    pass = testget(u, "236", w) && pass;

    // user, group readable
    pass = testget(u, "440", og) && pass;
    pass = testget(u, "770", og) && pass;
    pass = testget(u, "561", og) && pass;
 
    // user, world readable
    pass = testget(u, "404", ow) && pass;
    pass = testget(u, "707", ow) && pass;
    pass = testget(u, "625", ow) && pass;
  
    // group, world readable
    pass = testget(u, "044", gw) && pass;
    pass = testget(u, "077", gw) && pass;
    pass = testget(u, "265", gw) && pass;
     
    // all readable
    pass = testget(u, "444", ogw) && pass;
    pass = testget(u, "777", ogw) && pass;
    pass = testget(u, "465", ogw) && pass;
   
    return pass;
}


/** Make sure this is called last - it updates the keys */
bool testwriteable(const UniConf &u, bool none, bool o, bool g, bool w, bool og,
        bool ow, bool gw, bool ogw)
{
    bool pass = true;

    //writeable:   2367
    //unwriteable: 0145

    //FIXME: we never test writeable but not readable because it's not
    //convenient

    // all unwriteable
    pass = testset(u, "000", none) && pass;
    pass = testset(u, "145", none) && pass;

    // user writeable
    pass = testset(u, "700", o) && pass;
    pass = testset(u, "614", o) && pass;

    // group writeable
    pass = testset(u, "070", g) && pass;
    pass = testset(u, "164", g) && pass;

    // world writeable
    pass = testset(u, "007", w) && pass;
    pass = testset(u, "516", w) && pass;

    // user, group writeable
    pass = testset(u, "770", og) && pass;
    pass = testset(u, "661", og) && pass;
 
    // user, world writeable
    pass = testset(u, "707", ow) && pass;
    pass = testset(u, "646", ow) && pass;
  
    // group, world writeable
    pass = testset(u, "077", gw) && pass;
    pass = testset(u, "566", gw) && pass;
     
    // all writeable
    pass = testset(u, "777", ogw) && pass;
    pass = testset(u, "666", ogw) && pass;
 
    return pass;
}


int main(int argc, char **argv)
{
    WvLogConsole cons(2, WvLog::Debug4);

    WvString ini("ini:secure.ini");

    {
        printheader("DEFAULT PERMISSIONS");
        Objects *o = setup("null");

        /** All reads succeed, all writes fail */
        bool pass = true;
        pass = testreadable(o->u, true, true, true, true, true, true, true, true) && pass;
        pass = testwriteable(o->u, false, false, false, false, false, false, false, false) && pass;

        teardown(o);
        printfooter(pass);
    }

    {
        printheader("OWNER, GROUP MATCH");
        Objects *o = setup("temp");    
            
        UniPermGen::Credentials c;
        c.user = "clampy";
        c.groups.add(new WvString("cloggers"), true);
        o->s->setcredentials(c);

        /** All succeed except --- */
        bool pass = true;
        pass = testreadable(o->u, false, true, true, true, true, true, true, true) && pass;
        pass = testwriteable(o->u, false, true, true, true, true, true, true, true) && pass;
            
        teardown(o);
        printfooter(pass);
    }

    {
        printheader("OWNER MATCHES BUT NOT GROUP");
        Objects *o = setup("temp");    

        UniPermGen::Credentials c;
        c.user = "clampy";
        c.groups.add(new WvString("froggers"), true);
        o->s->setcredentials(c);

        /** ---, -g- fail; o--, --w, og-, o-w, -gw, ogw succeed */
        bool pass = true;
        pass = testreadable(o->u, false, true, false, true, true, true, true, true) && pass;
        pass = testwriteable(o->u, false, true, false, true, true, true, true, true) && pass;
        
        teardown(o);
        printfooter(pass);
    }

    {
        printheader("GROUP MATCHES BUT NOT OWNER");
        Objects *o = setup("temp");    

        UniPermGen::Credentials c;
        c.user = "stumpy";
        c.groups.add(new WvString("cloggers"), true);
        o->s->setcredentials(c);

        /** ---, o-- fail; -g-, --w, og-, o-w, -gw, ogw succeed */ 
        bool pass = true;
        pass = testreadable(o->u, false, false, true, true, true, true, true, true) && pass;
        pass = testwriteable(o->u, false, false, true, true, true, true, true, true) && pass;

        teardown(o);
        printfooter(pass);
    }

    {
        printheader("OWNER, GROUP DON'T MATCH");
        Objects *o = setup("temp");    

        UniPermGen::Credentials c;
        c.user = "stumpy";
        c.groups.add(new WvString("froggers"), true);
        o->s->setcredentials(c);

        /** ---, o--, -g-, og- fail; -w, o-w, -gw, ogw succeed */ 
        bool pass = true;
        pass = testreadable(o->u, false, false, false, true, false, true, true, true) && pass;
        pass = testwriteable(o->u, false, false, false, true, false, true, true, true) && pass;

        teardown(o);
        printfooter(pass);
    }

}
