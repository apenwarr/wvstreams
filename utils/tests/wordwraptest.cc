/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Simple test for WvWordWrapEncoder.
 */
#include "wvwordwrap.h"
#include "wvstream.h"
#include "wvistreamlist.h"
#include "wvencoderstream.h"

int main(int argc, char **argv)
{
    int maxwidth = 80;
    if (argc > 1)
        maxwidth = atoi(argv[1]);

    WvEncoder *enc = new WvWordWrapEncoder(maxwidth);

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
