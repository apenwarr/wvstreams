/** \file
 * A WvBase64 example.
 */
/** \example wvbase64ex.cc
 * This program encodes strings to Base64 format.
 * Typing CTRL-D will terminate this program.
 */

#include "wvbase64.h"
#include "wvstream.h"
#include "wvstreamlist.h"
#include "wvencoderstream.h"


int main()
{
    WvEncoder *enc;
    enc = new WvBase64Encoder();

    WvEncoderStream *stream = new WvEncoderStream(wvout);
    stream->disassociate_on_close = true;
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
    delete slist;

    return 0;
}
