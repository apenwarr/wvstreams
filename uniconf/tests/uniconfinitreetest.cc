// Test client for the UniIniTreeGen

#include "uniconf.h"
#include "uniinitreegen.h"

int main(int argc, char **argv)
{
    WvString mount_dir("/home/dgtaylor/.kde/share/config");
    UniConfRoot root;
    root.mount(WvString("initree:%s", mount_dir));

    WvString key("kwin.eventsrc/close/soundfile");
    WvString result = root[key].get();
    wvout->print("Result of root[\"%s\"].get() = %s\n",key, result);
    return 0;
}
