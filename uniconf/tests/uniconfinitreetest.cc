// Test client for the UniIniTreeGen

#include "uniconf.h"
#include "uniinitreegen.h"

int main(int argc, char **argv)
{
    WvString mount_dir("~/.kde/share/config");
    UniConfRoot root;
    root.mount(WvString("initree:%s", mount_dir));
    return 0;
}
