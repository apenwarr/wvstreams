/** Tests Ogg Speex encoding and decoding. */
#include "wvoggspeex.h"
#include "wvstreamlist.h"
#include "wvencoderstream.h"
#include "wvfile.h"
#include "wvpcmutils.h"
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

size_t copy(WvStream *in, WvStream *out, size_t maxbytes = 0)
{
    size_t total = 0;
    char buf[10240];

    WvStreamList slist;
    slist.append(in, false);
    slist.append(out, false);
    while (in->isok() && out->isok())
    {
        size_t bytes = sizeof(buf);
        if (maxbytes != 0 && bytes > maxbytes)
            bytes = maxbytes;
        if (slist.select(-1))
            slist.callback();
        size_t len = in->read(buf, bytes);
        if (len != 0)
        {
            total += len;
            out->write(buf, len);
        }
        if (maxbytes != 0)
        {
            maxbytes -= len;
            if (maxbytes == 0) break;
        }
    }
    return total;
}


WvString tostring(double value)
{
    char num[32];
    sprintf(num, "%f", value);
    return WvString(num);
}


void usage(const char *prog)
{
    wverr->print("Usage %s [options]\n", prog);
    wverr->print(
        "Common options\n"
        "  -i file     : specifies input file\n"
        "  -o file     : specifies output file\n"
        "  -d          : decode Ogg Speex from stdin or input file\n"
        "  -e          : encode Ogg Speex to stdout or output file\n"
        "  -r          : reencode (equivalent to both -d and -e)\n"
        "\n"
        "Encoder options\n"
        "  -b bps      : nominal bitrate\n"
        "  -q quality  : quality range 0.0 (low) to 1.0 (high)\n"
        "  -v          : enables VBR\n"
        "  -C complex  : complexity range 0 to 10\n"
        "  -M modeid   : force codec mode id, 0 = narrow, 1 = wide, 2 = ultrawide\n"
        "  -s rate     : sampling rate\n"
        "  -S serialno : bitstream serial number\n"
        "  -c comment  : adds comment tag to output Ogg Vorbis stream\n"
        "\n"
        "Decoder options\n"
        "  -p          : enable perceptual enhancement post-filter\n"
        "\n"
        "  The -q and the -b options are mutually exclusive\n"
        "  The -c option may be specified multiple times\n");
}


int main(int argc, char **argv)
{
    /*** Parse options ***/
    WvStream *in = wvin, *out = wvout;
    bool decoding = false, encoding = false;
    long nominal_bitrate = -1;
    int samplingrate = 8000;
    long serialno = WvOggSpeexEncoder::RANDOM_SERIALNO;
    double quality = -1.0;
    bool bitrate_set = false;
    bool vbr_enabled = false;
    bool postfilter = false;
    int complexity = WvSpeex::DEFAULT_COMPLEXITY;
    WvSpeex::CodecMode mode = WvSpeex::DEFAULT_MODE;
    WvStringList comments;
    const int channels = 1;
    
    srand(time(NULL));
    
    int c;
    while ((c = getopt(argc, argv, "?i:o:b:q:dervM:C:c:s:S:")) >= 0)
    {
        switch (c)
        {
        case '?':
        default:
            usage(argv[0]);
            return 1;

        case 'i':
            if (in != wvin) delete in;
            in = new WvFile(optarg, O_RDONLY);
            break;
        
        case 'o':
            if (out != wvout) delete out;
            out = new WvFile(optarg, O_WRONLY | O_TRUNC | O_CREAT);
            break;

        case 'b':
            nominal_bitrate = atoi(optarg);
            bitrate_set = true;
            break;

        case 'q':
            quality = atof(optarg);
            bitrate_set = true;
            break;

        case 'v':
            vbr_enabled = true;
            break;

        case 'd':
            decoding = true;
            break;

        case 'e':
            encoding = true;
            break;

        case 'r':
            decoding = true;
            encoding = true;
            break;

        case 'M':
            mode = WvSpeex::CodecMode(atoi(optarg));
            break;

        case 'C':
            complexity = atoi(optarg);
            break;

        case 'c':
            comments.append(new WvString(optarg), true);
            break;

        case 's':
            samplingrate = atoi(optarg);
            break;

        case 'S':
            serialno = atoi(optarg);
            break;
        }
    }
    if (! bitrate_set)
        quality = 0.5;

    /*** Initialize encoder and decoder ***/
    WvOggSpeexDecoder *oggdec = NULL;
    WvOggSpeexEncoder *oggenc = NULL;
    WvEncoderStream *iencstream = new WvEncoderStream(in);
    iencstream->disassociate_on_close = true;
    iencstream->auto_flush(false);
    WvEncoderStream *oencstream = new WvEncoderStream(out);
    oencstream->disassociate_on_close = true;
    oencstream->auto_flush(false);

    WvPassthroughEncoder *passin = new WvPassthroughEncoder();
    iencstream->readchain.append(passin, true);
    
    if (decoding)
    {
        oggdec = new WvOggSpeexDecoder();
        oggdec->setpostfilter(postfilter);
        iencstream->readchain.append(oggdec, true);
        iencstream->readchain.append(
            new WvPCMUnnormFloatToSigned16Encoder(), true);
    }
    
    WvPassthroughEncoder *passmid = new WvPassthroughEncoder();
    oencstream->writechain.append(passmid, true);
    
    if (encoding)
    {
        wverr->print("Target Ogg Speex Stream Info:\n");
        wverr->print("  Channels: %s\n", channels);
        wverr->print("  Rate    : %s Hz\n", samplingrate);
        
        WvSpeex::BitrateSpec bitratespec;
        if (vbr_enabled)
        {
            if (quality >= 0.0)
            {
                bitratespec = WvSpeex::VBRQuality(quality);
                wverr->print("  Quality : %s (vbr)\n", tostring(quality));
            }
            else
            {
                wverr->print("VBR requires a quality index\n");
                exit(1);
            }
        }
        else
        {
            if (quality >= 0.0)
            {
                bitratespec = WvSpeex::CBRQuality(quality);
                wverr->print("  Quality : %s (cbr)\n", tostring(quality));
            }
            else if (nominal_bitrate > 0)
            {
                bitratespec = WvSpeex::CBRBitrate(nominal_bitrate);
                wverr->print("  Bitrate : %s bps (cbr)\n",
                    nominal_bitrate);
            }
            else
            {
                wverr->print("Must specify a bitrate or quality index\n");
                exit(1);
            }
        }
    
        oggenc = new WvOggSpeexEncoder(bitratespec,
            samplingrate, channels, mode, complexity, serialno);
        wverr->print("  Bitrate : %s bps (returned by encoder)\n",
            oggenc->nominalbitrate());
        wverr->print("  Mode Id : %s\n", oggenc->mode());
        wverr->print("  Smpl/Frm: %s\n", oggenc->samplesperframe());
        WvStringList::Iter it(comments);
        for (it.rewind(); it.next(); )
        {
            oggenc->add_comment(it());
            wverr->print("  Comment : %s\n", it());
        }
            
        oencstream->writechain.append(
            new WvPCMSigned16ToUnnormFloatEncoder(), true);
        oencstream->writechain.append(oggenc, true);
        wverr->print("\n");
    }
    WvPassthroughEncoder *passout = new WvPassthroughEncoder();
    oencstream->writechain.append(passout, true);

    /*** Print out header of source Ogg Speex file ***/
    time_t elapsed_time = time(NULL);
    if (decoding)
    {
        // Note: A better loop could be written that would just
        //       read the header without actually processing any
        //       audio data if, for instance, we wanted to tailor
        //       the encoder chain to the stream.
        //       See WvOggSpeexDecoder::_encode().
        while (iencstream->isok() && oencstream->isok())
        {
            if (oggdec->isheaderok())
            {
                wverr->print("Source Ogg Vorbis Stream Info:\n");
                wverr->print("  Channels: %s\n", oggdec->channels());
                wverr->print("  Rate    : %s Hz\n",
                    oggdec->samplingrate());
                wverr->print("  Bitrate : %s\n",
                    oggdec->nominalbitrate() > 0 ?
                    WvString("%s bps (nominal)",
                        oggdec->nominalbitrate()) :
                    WvString("unknown"));
                wverr->print("  VBR     : %s\n",
                    oggdec->vbr() ? "enabled" : "disabled");
                wverr->print("  Mode Id : %s\n", oggdec->mode());
                wverr->print("  Smpl/Frm: %s\n",
                    oggdec->samplesperframe());
                wverr->print("  Vendor  : %s\n", oggdec->vendor());
                WvStringList::Iter it(oggdec->comments());
                for (it.rewind(); it.next(); )
                    wverr->print("  Comment : %s\n", it());
                wverr->print("\n");
                break;
            }
            copy(iencstream, oencstream, 1024);
        }
    }

    /*** Stream rest of file ***/
    wverr->print("Working...");
    
    copy(iencstream, oencstream);
    oencstream->finish_read();
    oencstream->finish_write();
    oencstream->flush(0);
    elapsed_time = time(NULL) - elapsed_time;

    wverr->print("\n\nSummary:\n");
    
    wverr->print("  Took : %s seconds\n", elapsed_time);
    wverr->print("  Bytes read      : %s\n",
        passin->bytes_processed());
    wverr->print("  Bytes processed : %s (uncompressed)\n",
        passmid->bytes_processed());
    wverr->print("  Bytes written   : %s\n",
        passout->bytes_processed());
    
    if (passmid->bytes_processed() > 0)
    {
        if (decoding)
        {
            double duration = double(passmid->bytes_processed()) /
                (oggdec->samplingrate() /* * oggdec->channels()*/ * 2);
            int bps = int(passin->bytes_processed() * 8 / duration);
            wverr->print("  Average input bitrate  : %s bps\n", bps);
        }
        if (encoding)
        {
            double duration = double(passmid->bytes_processed()) /
                (samplingrate /* * channels*/ * 2);
            int bps = int(passout->bytes_processed() * 8 / duration);
            wverr->print("  Average output bitrate : %s bps\n", bps);
        }
    }
    
    if (decoding)
        wverr->print("  Decoder state: isok()=%s, finished()=%s, "
            "geterror()=%s\n",
            oggdec->isok(), oggdec->isfinished(), oggdec->geterror());
    if (encoding)
        wverr->print("  Encoder state: isok()=%s, finished()=%s, "
            "geterror()=%s\n",
            oggenc->isok(), oggenc->isfinished(), oggenc->geterror());

    delete iencstream;
    delete oencstream;
    if (in != wvin)
        delete in;
    if (out != wvout)
        delete out;
    return 0;
}
