/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * Provides a WvEncoder abstraction for the SimpleAudio audio packet format.
 * suitable for encoding voice at low bitrates.
 *
 * Only monaural audio is supported for now.
 */

#include "wvaudioencoder.h"
#include "wvpcmutils.h"


/***** WvSimpleAudioEncoder *****/


WvSimpleAudioEncoder::WvSimpleAudioEncoder(unsigned int channels,
        unsigned int samplerate) :
    _channels(channels), _samplesperframe(samplerate / 1000 * 20)
{
}


bool WvSimpleAudioEncoder::_typedencode(IBuffer &inbuf, OBuffer &outbuf,
    bool flush)
{
    static WvPCMUnnormFloatToSigned16Functor f;

    for (;;)
    {
        // find how much data is available 
        size_t count = inbuf.used();
        if (count == 0) return true;
        if (count < _samplesperframe)
            return ! flush; // not enough data

        // convert this frame
        while (count > 0)
        {
            size_t avail = outbuf.optallocable();
            if (avail == 0) return false;
            if (avail > count) avail = count;
            count -= avail;

            const IType *indata = inbuf.get(avail);
            OType *outdata = outbuf.alloc(avail);
            while (avail-- > 0)
                *(outdata++) = f(*(indata++));
        }

        // return if we're not flushing
        if (! flush)
            return true;
    }
}


bool WvSimpleAudioEncoder::_typedfinish(OBuffer &outbuf)
{
    return true;
}


/***** WvSimpleAudioDecoder *****/


WvSimpleAudioDecoder::WvSimpleAudioDecoder(unsigned int channels,
        unsigned int samplerate) :
    _channels(channels), _samplesperframe(samplerate / 1000 * 20)
{
}


bool WvSimpleAudioDecoder::_typedencode(IBuffer &inbuf, OBuffer &outbuf,
    bool flush)
{
    static WvPCMSigned16ToUnnormFloatFunctor f;
    
    for (;;)
    {
        size_t avail = inbuf.used();
        if (avail == 0) return true;
        if (outbuf.free() < _samplesperframe)
            return false; // not enough room

        size_t skip = 0;
        if (avail > _samplesperframe)
        {
            // packet is too large but try to decode some audio anyhow
            skip = avail - _samplesperframe;
            avail -= skip;
        }

        const IType *indata = inbuf.get(avail);
        inbuf.skip(skip); // skip over bad data
        OType *outdata = outbuf.alloc(_samplesperframe);
        while (avail-- > 0)
            *(outdata++) = f(*(indata++));
        
        if (! flush)
            return true;
    }
}


bool WvSimpleAudioDecoder::_typedfinish(OBuffer &outbuf)
{
    return true;
}
