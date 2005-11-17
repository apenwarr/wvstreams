#include "wvtest.h"
#include "wvbackslash.h"
#include "wvstream.h"
#include "wvbufstream.h"
#include "wvfile.h"
#include "wvistreamlist.h"
#include "wvencoderstream.h"

#define BSLASH_NUM_INPUT 5

WVTEST_MAIN("backslashtest.cc")
{
    // Test encoding
    {
        WvEncoder *enc = new WvBackslashEncoder();
        WvBufStream *ostream = new WvBufStream();
        WvDynBuf outbuf;
        char *input[BSLASH_NUM_INPUT] = 
            {"encode this!\n", "baroofey\n", "\\", "\nmagoo\b", " "};
        char *desired[BSLASH_NUM_INPUT] =
            {"encode this!\\n", "baroofey\\n", "\\\\", "\\nmagoo\\b", " "};
        WvString result;

        WvEncoderStream *stream = new WvEncoderStream(ostream);
        stream->disassociate_on_close = true;
        stream->auto_flush(true);
        stream->writechain.append(enc, true);

        // start giving stream input and testing ostream for results
        for (int i = 0; i < BSLASH_NUM_INPUT; i++)
        {
            stream->write(input[i], strlen(input[i]));
            ostream->read(outbuf, 1024);
            result = outbuf.getstr();
            if (!WVPASS(result == desired[i]))
                printf("   because [%s] != [%s]\n", result.cstr(), desired[i]);
        }
       
        stream->flush(0);
        WVRELEASE(stream);
        WVRELEASE(ostream);
    }
    
    // Test decoding
    {
        WvEncoder *enc = new WvBackslashDecoder();
        WvBufStream *ostream = new WvBufStream();
        WvDynBuf outbuf;
        char *input[BSLASH_NUM_INPUT] =
            {"encode this!\\n", "baroofey\\n", "\\\\", "\\nmagoo\\b", " "};
        char *desired[BSLASH_NUM_INPUT] = 
            {"encode this!\n", "baroofey\n", "\\", "\nmagoo\b", " "};
        WvString result;

        WvEncoderStream *stream = new WvEncoderStream(ostream);
        stream->disassociate_on_close = true;
        stream->auto_flush(true);
        stream->writechain.append(enc, true);

        // start giving stream input and testing ostream for results
        for (int i = 0; i < BSLASH_NUM_INPUT; i++)
        {
            stream->write(input[i], strlen(input[i]));
            ostream->read(outbuf, 1024);
            result = outbuf.getstr();
            if (!WVPASS(result == desired[i]))
                printf("   because [%s] != [%s]\n", result.cstr(), desired[i]);
        }
       
        stream->flush(0);
        WVRELEASE(stream);
        WVRELEASE(ostream);
    }
}
