#include "wvtest.h"
#include "uniconfroot.h"
#include "unitempgen.h"
#include "unipermgen.h"
#include "unisecuregen.h"
#include "uniunwrapgen.h"

WVTEST_MAIN("permgen basic")
{
    UniConfRoot root;
    IUniConfGen *tempgen = new UniTempGen();
    UniPermGen permgen("temp:");
    permgen.setexec(UniConfKey("/"), UniPermGen::WORLD, true);
    permgen.setread(UniConfKey("/"), UniPermGen::WORLD, true);
    permgen.setwrite(UniConfKey("/"), UniPermGen::WORLD, true);

    UniSecureGen *sec = new UniSecureGen(tempgen, &permgen);
    fprintf(stderr, "Mounting securegen\n");
    WVPASS(root.mountgen(sec));
    fprintf(stderr, "Done\n");

    root["/open/foo"].setmeint(1);
    root["/open/bar"].setmeint(1);
    root["/executable_only/readable"].setmeint(1);
    root["/executable_only/unreadable"].setmeint(1);
    root["/closed/foo"].setmeint(1);
    root["/closed/bar"].setmeint(1);
    
    permgen.chmod(UniConfKey("/executable_only"), 7, 7, 1);
    // FIXME: chmodding one key seems to automatically chmod its
    // children. Is this correct?
    permgen.chmod(UniConfKey("/executable_only/readable"), 7, 7, 4);
    permgen.chmod(UniConfKey("/executable_only/unreadable"), 7, 7, 0);
    permgen.chmod(UniConfKey("/closed"), 7, 7, 0);

    // testing "get"
    WVPASS(root["/open"].getme() == "");
    WVPASS(root["/open/foo"].getme() == "1");
    WVPASS(root["/open/bar"].getme() == "1");

    WVPASS(root["/executable_only"].getme() == WvString::null);
    WVPASS(root["/executable_only/readable"].getme() == "1");
    WVPASS(root["/executable_only/unreadable"].getme() == WvString::null);

    WVPASS(root["/closed"].getme() == WvString::null);
    WVPASS(root["/closed/foo"].getme() == WvString::null);
    WVPASS(root["/closed/bar"].getme() == WvString::null);

    // testing "set" (obviously incomplete)
    root["/executable_only"].setmeint(1);
    WVPASS(root["/executable_only"].getme() == WvString::null);

    root["/closed"].setmeint(1);
    WVPASS(root["/closed"].getme() == WvString::null);

    // testing iteration
    UniConf::Iter i(root);
    i.rewind();
    for (int k=0; k<3; k++)
    {
        WVPASS(i.next());
        if (i.ptr()->key() == "closed")
        {
            WVPASS(i.ptr()->getme() == WvString::null);
            WVPASS(i._value() == WvString::null);
        }
        else if (i.ptr()->key() == "executable_only")
        {
            WVPASS(i.ptr()->getme() == WvString::null);
            WVPASS(i._value() == WvString::null);
        }
        else if (i.ptr()->key() == "open")
        {
            WVPASS(i.ptr()->getme() == "");
            WVPASS(i._value() == "");
        }
    }
    WVFAIL(i.next());

    // testing recursive iteration
    UniConf::RecursiveIter j(root);
    j.rewind();
    for (int k=0; k<3; k++)
    {
        WVPASS(j.next());
        if (j.ptr()->key() == "closed")
        {
            WVPASS(j.ptr()->getme() == WvString::null);
            WVPASS(j._value() == WvString::null);
        }
        else if (j.ptr()->key() == "executable_only")
        {
            WVPASS(j.ptr()->getme() == WvString::null);
            WVPASS(j._value() == WvString::null);

            for (int l=0; l<2; l++)
            {
                WVPASS(j.next());
                if (j.ptr()->key() == "readable")
                {
                    WVPASS(j.ptr()->getme() == "1");
                    WVPASS(j._value() == "1");
                }
                else if (j.ptr()->key() == "unreadable")
                {
                    WVPASS(j.ptr()->getme() == WvString::null);
                    WVPASS(j._value() == WvString::null);

                }
            }
        }
        else if (j.ptr()->key() == "open")
        {
            WVPASS(j.ptr()->getme() == "");
            WVPASS(j._value() == "");

            for (int l=0; l<2; l++)
            {
                WVPASS(j.next());
                if (j.ptr()->key() == "bar")
                {
                    WVPASS(j.ptr()->getme() == "1");
                    WVPASS(j._value() == "1");
                }
                else if (j.ptr()->key() == "foo")
                {
                    WVPASS(j.ptr()->getme() == "1");
                    WVPASS(j._value() == "1");

                }
            }
        }
    }
    WVFAIL(j.next());
}
