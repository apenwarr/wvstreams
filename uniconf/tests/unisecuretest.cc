#include <uniconf.h>
#include <unisecuregen.h>
#include <unipermgen.h>

#include <wvstring.h>
#include <wvstream.h>
#include <wvlog.h>
#include <wvlogrcv.h>

#include <unistd.h>


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


void printheader(WvStringParm h, WvStringParm moniker)
{
    WvString header("\n\nTEST %s WITH PERMS GEN %s", h, moniker);
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


bool testaget(const UniConf &u, WvStringParm key, WvStringParm expect)
{
    WvString got = u[key].get("nothing");
    if (expect != got)
        wvcon->print("FAILED: get %s expected %s, got %s\n", key, expect, got);
    return (expect == got);
}


bool testaset(const UniConf &u, WvStringParm key, WvStringParm val)
{
    WvString orig = u[key].get();
    u[key].set(val);
    WvString got = u[key].get();
    if (val != got)
        wvcon->print("FAILED: set %s expected %s, got %s (orig was %s)\n",
                key, val, got, orig);
    return (val == got);
}


bool testdefault(const UniConf &u)
{
    bool pass = true;

    // all world
    for (int world = 0; world < 8; world++)
    {
        // all user
        for (int group = 0; group < 8; group++)
        {
            // all group
            for (int user = 0; user < 8; user++)
            {
                // should succeed
                WvString key("%s%s%s", user, group, world);
                pass = testaget(u, key, key) && pass;
            }
        }
    }

    return pass;
}


bool testownergroup(const UniConf &u)
{
    bool pass = true;

    // world unreadable
    for (int world = 0; world < 4; world++)
    {
        // group unreadble
        for (int group = 0; group < 4; group++)
        {
            // user unreadable
            for (int user = 0; user < 4; user++)
            {
                // should fail
                WvString key("%s%s%s", user, group, world);
                pass = testaget(u, key, "nothing") && pass;
            }
            // user readable
            for (int user = 4; user < 8; user++)
            {
                // should succeed
                WvString key("%s%s%s", user, group, world);
                pass = testaget(u, key, key) && pass;
            }
        }
        // group readable
        for (int group = 4; group < 8; group++)
        {
            // all user
            for (int user = 0; user < 8; user++)
            {
                // should succeed
                WvString key("%s%s%s", user, group, world);
                pass = testaget(u, key, key) && pass;
            }
        }
    }
    // world readable
    for (int world = 4; world < 8; world++)
    {
        // all group
        for (int group = 0; group < 8; group++)
        {
            // all user
            for (int user = 0; user < 8; user++)
            {
                // should succeed
                WvString key("%s%s%s", user, group, world);
                pass = testaget(u, key, key) && pass;
            }
        }
    }

    return pass;
}


bool testownernogroup(const UniConf &u)
{
    bool pass = true;

    // world unreadable
    for (int world = 0; world < 4; world++)
    {
        // all group
        for (int group = 0; group < 8; group++)
        {
            // user unreadable
            for (int user = 0; user < 4; user++)
            {
                // should fail
                WvString key("%s%s%s", user, group, world);
                pass = testaget(u, key, "nothing") && pass;
            }
            // user readable
            for (int user = 4; user < 8; user++)
            {
                // should succeed
                WvString key("%s%s%s", user, group, world);
                pass = testaget(u, key, key) && pass;
            }
        }
    }
    // world readable
    for (int world = 4; world < 8; world++)
    {
        // all group
        for (int group = 0; group < 8; group++)
        {
            for (int user = 0; user < 8; user++)
            {
                // should succeed
                WvString key("%s%s%s", user, group, world);
                pass = testaget(u, key, key) && pass;
            }
        }
    }

    return pass;
}


bool testnoownergroup(const UniConf &u)
{
    bool pass = true;

    // world unreadable
    for (int world = 0; world < 4; world++)
    {
        // group unreadble
        for (int group = 0; group < 4; group++)
        {
            // all user
            for (int user = 0; user < 8; user++)
            {
                // should fail
                WvString key("%s%s%s", user, group, world);
                pass = testaget(u, key, "nothing") && pass;
            }
        }
        // group readable
        for (int group = 4; group < 8; group++)
        {
            // all user
            for (int user = 0; user < 8; user++)
            {
                // should succeed
                WvString key("%s%s%s", user, group, world);
                pass = testaget(u, key, key) && pass;
            }
        }
    }
    // world readable
    for (int world = 4; world < 8; world++)
    {
        // all group
        for (int group = 0; group < 8; group++)
        {
            // all user
            for (int user = 0; user < 8; user++)
            {
                // should succeed
                WvString key("%s%s%s", user, group, world);
                pass = testaget(u, key, key) && pass;
            }
        }
    }

    return pass;
}


bool testnoownernogroup(const UniConf &u)
{
    bool pass = true;

    // world unreadable
    for (int world = 0; world < 4; world++)
    {
        // all group
        for (int group = 0; group < 4; group++)
        {
            // all user
            for (int user = 0; user < 4; user++)
            {
                // should fail
                WvString key("%s%s%s", user, group, world);
                pass = testaget(u, key, "nothing") && pass;
            }
        }
    }
    // world readable
    for (int world = 4; world < 8; world++)
    {
        // all group
        for (int group = 0; group < 8; group++)
        {
            // all user
            for (int user = 0; user < 8; user++)
            {
                // should succeed
                WvString key("%s%s%s", user, group, world);
                pass = testaget(u, key, key) && pass;
            }
        }
    }

    return pass;
}


int main(int argc, char **argv)
{
    WvLogConsole cons(2, WvLog::Debug4);

    WvString ini("ini:secure.ini");

    // test with default perms
    {
        WvString perms("null");
        Objects *o = setup(perms);

        printheader("READING WITH DEFAULT PERMISSIONS", perms);

        bool pass = testdefault(o->u);

        printfooter(pass);
        
        teardown(o);
    }

    // test without a defaults gen
    {
        WvString perms("temp");
        Objects *o = setup(perms);    

        {
            printheader("READING WHERE OWNER, GROUP MATCH", perms);
            
            UniPermGen::Credentials c;
            c.user = "clampy";
            c.groups.add(new WvString("cloggers"), true);
            o->s->setcredentials(c);

            bool pass = testownergroup(o->u);
            
            printfooter(pass);
        }

        {
            printheader("READS WHERE OWNER MATCHES BUT NOT GROUP", perms);

            UniPermGen::Credentials c;
            c.user = "clampy";
            c.groups.add(new WvString("froggers"), true);
            o->s->setcredentials(c);

            bool pass = testownernogroup(o->u);
            
            printfooter(pass);
        }

        {
            printheader("READS WHERE GROUP MATCHES BUT NOT OWNER", perms);

            UniPermGen::Credentials c;
            c.user = "stumpy";
            c.groups.add(new WvString("cloggers"), true);
            o->s->setcredentials(c);

            bool pass = testnoownergroup(o->u);
            
            printfooter(pass);
        }

        {
            printheader("READS WHERE OWNER, GROUP DON'T MATCH", perms);

            UniPermGen::Credentials c;
            c.user = "stumpy";
            c.groups.add(new WvString("froggers"), true);
            o->s->setcredentials(c);

            bool pass = testnoownernogroup(o->u);
            
            printfooter(pass);
        }

        teardown(o);
    }
}
