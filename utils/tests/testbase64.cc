/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test program for base64 functions...
 */

#include "wvbase64.h"
#include "wvstream.h"
#include "wvstreamlist.h"
#include "wvencoderstream.h"

int main(int argc, char **argv)
{
    bool encode = true;
    if (argc > 1 && strcmp(argv[1], "-d") == 0)
        encode = false;

    WvEncoder *enc;
    if (encode)
        enc = new WvBase64Encoder();
    else
        enc = new WvBase64Decoder();

    WvStream *wvin = new WvStream(0);
    WvStream *wvout = new WvStream(1);
    WvEncoderStream *stream = new WvEncoderStream(wvout);
    stream->auto_flush(false);
    stream->writechain.append(enc, true);

    WvStreamList *slist = new WvStreamList();
    slist->append(stream, false);
    slist->append(wvin, false);
    wvin->autoforward(*stream);
    
    while (wvin->isok())
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
