#include "uniconfroot.h"
#include "wvfile.h"

int main()
{
    UniConfRoot cfg("ini:foo2.ini");
    for (int i = 0; i < 100000; i++)
        cfg[i].setmeint(i);
    cfg.commit();
}
