#include "wvtest.h"
#include "wvfile.h"
#include "wvstringlist.h"
#include "uniconfroot.h"
#include "uniinigen.h"
#include "unilistgen.h"
#include "unitempgen.h"

WVTEST_MAIN("Testing refresh()")
{
    ::unlink("tmp.ini");
    WvFile file("tmp.ini", O_WRONLY|O_TRUNC|O_CREAT);
    file.write("[foo]\n"
               "a = b\n");
    WVPASS(file.isok());
    
    UniConfRoot uni("list:ini:/tmp/foobee.ini ini:tmp.ini");
    WVPASSEQ(uni["foo"]["a"].getme(), "b");
}

WVTEST_MAIN("Testing for use with weaver")
{
    UniTempGen *tmp1 = new UniTempGen();
    UniTempGen *tmp2 = new UniTempGen();
    UniConfGenList *l = new UniConfGenList();
    l->append(tmp1, false);
    l->append(tmp2, false);
    UniListGen *unigen = new UniListGen(l);
    
    UniConfRoot uniconf(unigen);
    UniConfRoot front(tmp1);
    UniConfRoot back(tmp2);

    //should work as a normal generator
    uniconf.xsetint("Monkey", 1);
    WVPASS(uniconf.xgetint("Monkey"));
    WVPASS(front.xgetint("Monkey"));
    WVFAIL(back.xgetint("Monkey"));
    
    WVFAIL(uniconf.xgetint("Banana"));
    //should read from tmp2 in background
    back.xsetint("Banana", 1);
    WVPASS(uniconf.xgetint("Banana"));
    WVPASS(back.xgetint("Banana"));
    
    uniconf.xsetint("Banana", 0);
    WVFAIL(uniconf.xgetint("Banana"));
    WVFAIL(front.xgetint("Banana"));
    // back should still be as it was
    WVPASS(back.xgetint("Banana"));
    
    // testing when both are set to same
    WVFAIL(uniconf.xgetint("Manana"));
    uniconf.xsetint("Manana", 1);
    WVPASS(uniconf.xgetint("Manana"));
    back.xsetint("Manana", 0);
    WVPASS(uniconf.xgetint("Manana"));
}

WVTEST_MAIN("Testing iterator")
{
    UniTempGen *tmp1 = new UniTempGen();
    UniTempGen *tmp2 = new UniTempGen();
    UniConfGenList *l = new UniConfGenList();
    l->append(tmp1, true);
    l->append(tmp2, true);
    UniListGen *unigen = new UniListGen(l);
    UniConfRoot uniconf(unigen);
    UniConfKey key("key");
    
    uniconf.xset("section/key", "value");
    printf("%s\n", uniconf.xget("section/key", "DONG").cstr());
    UniConf::Iter i1(uniconf["section"]);
    for (i1.rewind(); i1.next(); )
    {
        WVPASS(i1().key() == key);
    }

    WvString a[5] = {"foo/goose","foo/moose","foo/garoose","foo/setme!","foo/bloing"}, 
        expected[5] = {"bloing", "setme!", "goose", "garoose", "moose"};
    int i;
    bool iterated_properly = true, iter_didnt_mangle = true;
        
    for (i = 0; i < 5; i++)
        uniconf.xsetint(a[i], 1);
    
    UniConf::Iter i2(uniconf["foo"]);
    i = 0;
    for (i2.rewind(); i2.next(); i++)
    {
        //printf("iterated over: %s\n", i2->fullkey().cstr());
        if (!(i2->fullkey().removefirst() == expected[i]))
            iterated_properly = false;
    }
    WVPASS(iterated_properly);

    //verify iterating didn't destroy
    for (int i = 0; i < 5; i++)
        if (!(uniconf.xgetint(a[i]) == 1))
            iter_didnt_mangle = false;
    WVPASS(iter_didnt_mangle);

    
    WvString expected2[15] = {"bloing", "bloing/foo", "bloing/foo/bloing", "setme!", 
        "setme!/foo", "setme!/foo/setme!", "goose", "goose/foo", "goose/foo/goose",
        "garoose", "garoose/foo", "garoose/foo/garoose", "moose", "moose/foo",
        "moose/foo/moose"};
    for (i = 0; i < 5; i++)
        uniconf.xsetint(WvString("%s/%s", a[i] , a[i]), 1);
    
    UniConf::RecursiveIter i3(uniconf["foo"]);
    i = 0;
    iterated_properly = true;
    for (i3.rewind(); i3.next(); i++)
    {
        //printf("iterated over: %s\n", i3->fullkey().cstr());
        if (!(i3->fullkey().removefirst() == expected2[i]))
            iterated_properly = false;
    }
    WVPASS(iterated_properly);
    
    //verify iterating didn't destroy
    iter_didnt_mangle = true;
    for (int i = 0; i < 5; i++)
        if (!(uniconf.xgetint(a[i]) == 1))
            iter_didnt_mangle = false;
    WVPASS(iter_didnt_mangle);
}


WVTEST_MAIN("List of inigens bug 6198")
{
    ::unlink("tmp.ini");
    ::unlink("tmp2.ini");
    WvFile file("tmp2.ini", O_WRONLY|O_TRUNC|O_CREAT);
    file.write("[foo]\n"
               "a = b\n");
    WVPASS(file.isok());

    UniConfRoot uni("list:ini:tmp.ini ini:tmp2.ini");
    WVPASSEQ(uni["foo"].xget("a", "notb"),"b");
}
