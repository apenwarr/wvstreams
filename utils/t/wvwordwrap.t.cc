#include "wvtest.h"
#include "wvwordwrap.h"
#include "wvstream.h"
#include "wvbufstream.h"
#include "wvencoderstream.h"

#define WRAP_NUM_INPUT 6

WVTEST_MAIN("wordwraptest.cc")
{    
    int maxwidth = 4;
    WvEncoder *enc = new WvWordWrapEncoder(maxwidth);
    WvBufStream *ostream = new WvBufStream();
    WvDynBuf outbuf;
    const char *input[WRAP_NUM_INPUT] = {"aaaaaa\n", "abra cadabra\n", 
        "cabo-oose\n", "ja ba ba doo ba\n", "rabarabaraba\n", "\nboo\nboo\n"};
    const char *desired[WRAP_NUM_INPUT] = {"aaaa\naa\n", "abra\ncada\nbra\n", 
        "cabo\n-oos\ne\n", "ja\nba\nba\ndoo\nba\n", "raba\nraba\nraba\n", 
        "\nboo\nboo\n"};
    WvString result;
    
    WvEncoderStream *stream = new WvEncoderStream(ostream);
    stream->auto_flush(false);
    stream->writechain.append(enc, true);
    
    // start giving stream input and testing ostream for results
    for (int i = 0; i < WRAP_NUM_INPUT; i++)
    {
        stream->write(input[i], strlen(input[i]));
        ostream->read(outbuf, 1024);
        result = outbuf.getstr();
        WVPASSEQ(result, desired[i]);
    }

    stream->flush(0);
    WVRELEASE(stream);
}
