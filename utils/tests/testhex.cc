/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test program for hex functions...
 */

#include "wvhex.h"
#include "wvstream.h"
#include "wvstreamlist.h"
#include "wvencoderstream.h"
#include <assert.h>

int main(int argc, char **argv)
{
    char obuf[40];
    hexify(obuf, "ABCDE\n\37700", 7);
    obuf[15] = 'Z';
    assert(memcmp(obuf, "41424344450aff\0Z", 16) == 0);

    assert(strcmp(WvHexEncoder().strflush("ABCDE\n\377", true),
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

    WvStream *wvin = new WvStream(0);
    WvStream *wvout = new WvStream(1);
    WvEncoderStream *stream = new WvEncoderStream(wvout);
    stream->auto_flush(false);
    stream->writechain.append(enc, true);

    WvStreamList *slist = new WvStreamList();
    slist->append(stream, false);
    slist->append(wvin, false);
    wvin->autoforward(*stream);
    
    while (wvin->isok() && stream->isok())
    {
        if (slist->select(-1))
            slist->callback();
    }
    stream->flush(0);
    delete stream;
    delete wvin;
    delete slist;
    
    return 0;
}
