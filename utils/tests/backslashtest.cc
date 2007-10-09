/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test program for backslash functions...
 */

#include "wvbackslash.h"
#include "wvstream.h"
#include "wvistreamlist.h"
#include "wvencoderstream.h"
#include <assert.h>

int main(int argc, char **argv)
{
    // test using stdin/stdout
    bool encode = true;
    if (argc > 1 && strcmp(argv[1], "-d") == 0)
        encode = false;

    WvEncoder *enc;
    if (encode)
        enc = new WvBackslashEncoder();
    else
        enc = new WvBackslashDecoder();

    WvEncoderStream *stream = new WvEncoderStream((wvout->addRef(), wvout));
    stream->auto_flush(false);
    stream->writechain.append(enc, true);

    WvIStreamList *slist = new WvIStreamList();
    slist->append(stream, false, "stream");
    slist->append(wvin, false, "wvin");
    wvin->autoforward(*stream);
    
    while (wvin->isok() && stream->isok())
    {
        if (slist->select(-1))
            slist->callback();
    }
    stream->flush(0);
    WVRELEASE(stream);
    WVRELEASE(slist);
    
    return 0;
}
