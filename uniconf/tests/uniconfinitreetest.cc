// Test client for the UniIniTreeGen
// FIXME:  Right now this just outputs everything in all the ini files.  Need to
// come up with a better way to do this to allow for automated checking.


#include "uniconf.h"
#include "uniinitreegen.h"

int main(int argc, char **argv)
{
    WvString mountstring("initree:initreetests");
    UniConfRoot root;
    wvout->print("Attempting to mount:  %s\n", mountstring);
    root.mount(mountstring);

    UniConf::RecursiveIter i(root);
    for (i.rewind(); i.next();)
    {
        wvout->print("Key:  %s.  Value:  %s.\n",i->fullkey(), i->get());
    }
    return 0;
}
