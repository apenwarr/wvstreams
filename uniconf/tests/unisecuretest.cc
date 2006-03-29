#include <uniconfroot.h>
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
 */


/**
 * Setup/teardown
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
        u.mount("default:ini:secure.ini");

        u["nondef/*"].setme("*1");
        u["defaults/*/*"].setme("*1");
        u["inherited/*"].setme("*1");
        u.commit();
    }

    // create and populate the perms gen
    UniPermGen *p = new UniPermGen(perms);
    UniSecureGen *s = new UniSecureGen("default:ini:secure.ini", p);

    // defaults tree has owner group set with a pattern
    p->setowner("defaults/*/*", "clampy");
    p->setgroup("defaults/*/*", "cloggers");

    // inherited tree inherits everything 
    p->setowner("inherited", "clampy");
    p->setgroup("inherited", "cloggers");
    p->chmod("inherited", 0777);

    // now chmod each key in nondef/NNN and defaults/*/NNN to 0NNN, and set
    // the owner/group of nondef/* directly 
    for (int user = 0; user < 8; user++)
    {
        for (int group = 0; group < 8; group++)
        {
            for (int world = 0; world < 8; world++)
            {
                WvString key("%s%s%s", user, group, world);
                WvString nondef("nondef/%s", key);
                WvString defaults("defaults/*/%s", key);
                p->setowner(nondef, "clampy");
                p->setgroup(nondef, "cloggers");
                p->chmod(nondef, user, group, world);
                p->chmod(defaults, user, group, world);
            }
        }
    }

    // all access to the defaults/2 tree should fail due to missing exec
    p->setexec("defaults/2", UniPermGen::USER, false);
    p->setexec("defaults/2", UniPermGen::GROUP, false);
    p->setexec("defaults/2", UniPermGen::WORLD, false);

    p->commit();

    // create the uniconf obj and mount the generators
    Objects *o = new Objects(p, s);
    return o;
}


void teardown(Objects *o)
{
    delete o;
    unlink("secure.ini");
}


/**
 * Headers/footers
 */


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
    if (pass)
        wvcon->print("\n***** PASSED *****\n\n");
    else
        wvcon->print("\n///// FAILED /////\n\n");
}


void printfinal(bool pass)
{
    WvString footer("===   TEST SUITE %s   ===", pass ? "PASSED" : "FAILED");
    wvcon->print("%s\n",footer);
}


/**
 * Test get/set of a single key
 */


bool testaget(const UniConf &u, WvStringParm prefix, WvStringParm key, bool expectsucc)
{
    WvString expect = expectsucc ? WvString(key) : WvString("nothing");
    WvString got = u[prefix][key].getme("nothing");
    if (expect == got)
        wvcon->print("OK - %s/%s: got \"%s\"\n", prefix, key, got);
    else
        wvcon->print("FAIL - %s/%s: expected \"%s\", got \"%s\"\n", prefix, key, expect, got);
    return (expect == got);
}


bool testaset(const UniConf &u, WvStringParm prefix, WvStringParm key, bool expectsucc)
{
    WvString val = "brandnew";
    WvString orig = u[prefix][key].getme();
    WvString expect = expectsucc ? val : orig;
    u[prefix][key].setme(val);
    WvString got = u[prefix][key].getme();
    if (expect == got)
        wvcon->print("OK - set %s/%s: got \"%s\"\n", prefix, key, got);
    else
        wvcon->print("FAIL - set %s/%s: expected \"%s\", got \"%s\"\n", prefix, key, expect, got);
    return (got == expect);
}


/**
 * Test get/set of a single key in 4 branches (nondef / foo, defaults / 1 /
 * foo, defaults / 2 / foo, inherited / foo)
 */


bool testget(const UniConf &u, WvStringParm key, bool expectsucc, bool noperms)
{
    bool pass = true;
    // no default
    pass = testaget(u, "nondef", key, expectsucc) && pass;
    if (!noperms)
    {
        // with default
        pass = testaget(u, "defaults/1", key, expectsucc) && pass;
        // drilldown
        pass = testaget(u, "defaults/2", key, false) && pass;
        // inherited
        pass = testaget(u, "inherited", key, true) && pass;
    }
    return pass;
}


bool testset(const UniConf &u, WvStringParm key, bool expectsucc, bool noperms)
{
    bool pass = true;
    // no default
    pass = testaset(u, "nondef", key, expectsucc) && pass;
    if (!noperms)
    {
        // with default
        pass = testaset(u, "defaults/1", key, expectsucc) && pass;
        // drilldown
        pass = testaset(u, "defaults/2", key, false) && pass;
        // inherited
        pass = testaset(u, "inherited", key, true) && pass;
    }
    return pass;
}


/**
 * Test a variety of keys for the given permissions
 */


bool testreadable(const UniConf &u, bool none, bool o, bool g, bool w, bool og,
        bool ow, bool gw, bool ogw, bool noperms = false)
{
    bool pass = true;

    //readable:   4567
    //unreadable: 0123

    // all unreadable
    pass = testget(u, "000", none, noperms) && pass;
    pass = testget(u, "123", none, noperms) && pass;

    // user readable
    pass = testget(u, "400", o, noperms) && pass;
    pass = testget(u, "700", o, noperms) && pass;
    pass = testget(u, "512", o, noperms) && pass;

    // group readable
    pass = testget(u, "040", g, noperms) && pass;
    pass = testget(u, "070", g, noperms) && pass;
    pass = testget(u, "152", g, noperms) && pass;

    // world readable
    pass = testget(u, "004", w, noperms) && pass;
    pass = testget(u, "007", w, noperms) && pass;
    pass = testget(u, "236", w, noperms) && pass;

    // user, group readable
    pass = testget(u, "440", og, noperms) && pass;
    pass = testget(u, "770", og, noperms) && pass;
    pass = testget(u, "561", og, noperms) && pass;
 
    // user, world readable
    pass = testget(u, "404", ow, noperms) && pass;
    pass = testget(u, "707", ow, noperms) && pass;
    pass = testget(u, "625", ow, noperms) && pass;
  
    // group, world readable
    pass = testget(u, "044", gw, noperms) && pass;
    pass = testget(u, "077", gw, noperms) && pass;
    pass = testget(u, "265", gw, noperms) && pass;
     
    // all readable
    pass = testget(u, "444", ogw, noperms) && pass;
    pass = testget(u, "777", ogw, noperms) && pass;
    pass = testget(u, "465", ogw, noperms) && pass;
   
    return pass;
}


/** Make sure this is called last - it updates the keys */
bool testwriteable(const UniConf &u, bool none, bool o, bool g, bool w, bool og,
        bool ow, bool gw, bool ogw, bool noperms = false)
{
    bool pass = true;

    //writeable:   2367
    //unwriteable: 0145

    //FIXME: we never test writeable but not readable because it's not
    //convenient

    // all unwriteable
    pass = testset(u, "000", none, noperms) && pass;
    pass = testset(u, "145", none, noperms) && pass;

    // user writeable
    pass = testset(u, "700", o, noperms) && pass;
    pass = testset(u, "614", o, noperms) && pass;

    // group writeable
    pass = testset(u, "070", g, noperms) && pass;
    pass = testset(u, "164", g, noperms) && pass;

    // world writeable
    pass = testset(u, "007", w, noperms) && pass;
    pass = testset(u, "516", w, noperms) && pass;

    // user, group writeable
    pass = testset(u, "770", og, noperms) && pass;
    pass = testset(u, "661", og, noperms) && pass;
 
    // user, world writeable
    pass = testset(u, "707", ow, noperms) && pass;
    pass = testset(u, "646", ow, noperms) && pass;
  
    // group, world writeable
    pass = testset(u, "077", gw, noperms) && pass;
    pass = testset(u, "566", gw, noperms) && pass;
     
    // all writeable
    pass = testset(u, "777", ogw, noperms) && pass;
    pass = testset(u, "666", ogw, noperms) && pass;
 
    return pass;
}


/**
 * Main test suite
 */


int main(int argc, char **argv)
{
    WvLogConsole cons(2, WvLog::Debug4);

    WvString ini("ini:secure.ini");

    bool allpassed = true;
    
    {
        printheader("DEFAULT PERMISSIONS");
        Objects *o = setup("null");

        /** All reads succeed, all writes fail */
        bool pass = true;
        pass = testreadable(o->u, true, true, true, true, true, true, true, true, true) && pass;
        pass = testwriteable(o->u, false, false, false, false, false, false, false, false, true) && pass;

        teardown(o);
        printfooter(pass);

        allpassed = pass && allpassed;
    }

    {
        printheader("OWNER, GROUP MATCH");
        Objects *o = setup("temp");    

        UniPermGen::Credentials c;
        c.user = "clampy";
        c.groups.add(new WvString("cloggers"), true);
        o->s->setcredentials(c);

        /** ---, -g-, --w, -gw fail; o--, og-, o-w, ogw succeed */
        bool pass = true;
        pass = testreadable(o->u, false, true, false, false, true, true, false, true) && pass;
        pass = testwriteable(o->u, false, true, false, false, true, true, false, true) && pass;
            
        teardown(o);
        printfooter(pass);

        allpassed = pass && allpassed;
    }

    {
        printheader("OWNER MATCHES BUT NOT GROUP");
        Objects *o = setup("temp");    

        UniPermGen::Credentials c;
        c.user = "clampy";
        c.groups.add(new WvString("froggers"), true);
        o->s->setcredentials(c);

        /** ---, -g-, --w, -gw fail; o--, og-, o-w, ogw succeed */
        bool pass = true;
        pass = testreadable(o->u, false, true, false, false, true, true, false, true) && pass;
        pass = testwriteable(o->u, false, true, false, false, true, true, false, true) && pass;
        
        teardown(o);
        printfooter(pass);

        allpassed = pass && allpassed;
    }

    {
        printheader("GROUP MATCHES BUT NOT OWNER");
        Objects *o = setup("temp");    

        UniPermGen::Credentials c;
        c.user = "stumpy";
        c.groups.add(new WvString("cloggers"), true);
        o->s->setcredentials(c);

        /** ---, o--, --w, o-w fail; -g-, og-, -gw, ogw succeed */ 
        bool pass = true;
        pass = testreadable(o->u, false, false, true, false, true, false, true, true) && pass;
        pass = testwriteable(o->u, false, false, true, false, true, false, true, true) && pass;

        teardown(o);
        printfooter(pass);

        allpassed = pass && allpassed;
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

        allpassed = pass && allpassed;
    }

    printfinal(allpassed);

}
