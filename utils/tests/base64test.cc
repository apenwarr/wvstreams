/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Test program for base64 functions...
 */

#include "wvbase64.h"
#include "wvstream.h"
#include "wvistreamlist.h"
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

    WvEncoderStream *stream = new WvEncoderStream(wvout);
    stream->disassociate_on_close = true;
    stream->auto_flush(false);
    stream->writechain.append(enc, true);

    WvIStreamList *slist = new WvIStreamList();
    slist->append(stream, false);
    slist->append(wvin, false);
    wvin->autoforward(*stream);
    
    while (wvin->isok() && stream->isok())
    {
        if (slist->select(-1))
            slist->callback();
    }
    stream->flush(0);
    RELEASE(stream);
    RELEASE(slist);
    
    return 0;
}
