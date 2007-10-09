/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test program for hex functions...
 */

#include "wvhex.h"
#include "wvstream.h"
#include "wvistreamlist.h"
#include "wvencoderstream.h"
#include <assert.h>

int main(int argc, char **argv)
{
    char obuf[40];
    hexify(obuf, "ABCDE\n\37700", 7);
    obuf[15] = 'Z';
    assert(memcmp(obuf, "41424344450aff\0Z", 16) == 0);

    assert(strcmp(WvHexEncoder().strflushstr("ABCDE\n\377", true),
        "41424344450aff") == 0);

    unhexify(obuf, "41424344450aff\377\0ab");
    obuf[7] = 'Z';
    assert(memcmp(obuf, "ABCDE\n\xffZ", 8) == 0);

    // test using stdin/stdout
    bool encode = true;
    if (argc > 1 && strcmp(argv[1], "-d") == 0)
        encode = false;

    WvEncoder *enc;
    if (encode)
        enc = new WvHexEncoder();
    else
        enc = new WvHexDecoder();

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
