#include <stdio.h>
#include "wvstring.h"
#include "wvlog.h"

int main()
{
    WvString s("WvString world!\n");
    WvLog log("mytest", WvLog::Info);
    log(s);
}
