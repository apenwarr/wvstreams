/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * Provides a WvEncoder abstraction for the Speex audio packet format.
 * suitable for encoding voice at low bitrates.
 *
 * Only monaural audio is supported for now.
 */
#include "wvspeex.h"
#include <speex.h>
#include <speex_header.h>

/**
 * Returns the SpeexMode to use for a given sampling rate.
 * @param modeid the suggested mode
 * @param samplingrate the sampling rate
 */
static SpeexMode *get_speex_mode(WvSpeex::CodecMode modeid,
    int samplingrate)
{
    // use the suggested mode if it is valid
    if (modeid >= 0 && modeid < SPEEX_NB_MODES)
        return speex_mode_list[modeid];

    // otherwise determine a suitable default
    SpeexMode *mode;
    assert(samplingrate <= 48000 || ! "sampling rate too high");
    assert(samplingrate >= 6000 || ! "sampling rate too low");
    if (samplingrate > 25000)
        mode = & speex_uwb_mode;
    else if (samplingrate > 12500)
        mode = & speex_wb_mode;
    else
        mode = & speex_nb_mode;
    return mode;
}



/***** WvSpeexEncoder *****/

WvSpeexEncoder::WvSpeexEncoder(
    const WvSpeex::BitrateSpec &bitratespec,
    int samplingrate, int channels, WvSpeex::CodecMode modeid,
    int complexity) :
    spxstate(NULL), spxbits(NULL), spxmode(NULL),
    _channels(channels), _samplesperframe(0)
{
    // init encoder
    spxmode = get_speex_mode(modeid, samplingrate);
    spxstate = speex_encoder_init(spxmode);
    if (! spxstate)
    {
        seterror("error during speex_encoder_init");
        return;
    }
    spxbits = new SpeexBits;
    speex_bits_init(spxbits);
    
    // set sampling rate
    speex_encoder_ctl(spxstate, SPEEX_SET_SAMPLING_RATE, &samplingrate);

    // set the complexity
    if (complexity != WvSpeex::DEFAULT_COMPLEXITY)
        speex_encoder_ctl(spxstate, SPEEX_SET_COMPLEXITY, &complexity);
    
    // init bitrate management
    switch (bitratespec.mode)
    {
        case WvSpeex::BitrateSpec::VBR_QUALITY:
        {
            int enable = 1;
            speex_encoder_ctl(spxstate, SPEEX_SET_VBR, &enable);
            float quality = bitratespec.quality_index * 10;
            speex_encoder_ctl(spxstate, SPEEX_SET_VBR_QUALITY,
                &quality);
            break;
        }

        case WvSpeex::BitrateSpec::CBR_QUALITY:
        {
            int quality = int(bitratespec.quality_index * 10);
            speex_encoder_ctl(spxstate, SPEEX_SET_QUALITY,
                &quality);
            break;
        }

        case WvSpeex::BitrateSpec::CBR_BITRATE:
        {
            int bitrate = bitratespec.nominal_bitrate;
            speex_encoder_ctl(spxstate, SPEEX_SET_BITRATE,
                &bitrate);
            break;
        }
    }

    // cache frame size since we use it often
    speex_encoder_ctl(spxstate, SPEEX_GET_FRAME_SIZE,
        &_samplesperframe);
}


WvSpeexEncoder::~WvSpeexEncoder()
{
    speex_encoder_destroy(spxstate);
    speex_bits_destroy(spxbits);
    delete spxbits;
}


bool WvSpeexEncoder::_typedencode(IBuffer &inbuf, OBuffer &outbuf,
    bool flush)
{
    if (! flushspxbits(outbuf))
        return false;
    for (;;)
    {
        size_t avail = inbuf.used();
        if (avail == 0)
            return true;
        if (avail < size_t(_samplesperframe))
            return ! flush; // not enough data
        speex_encode(spxstate, const_cast<float*>(
            inbuf.get(_samplesperframe)), spxbits);
        if (! flushspxbits(outbuf))
            return false;
        if (! flush)
            return true;
    }
}


bool WvSpeexEncoder::_typedfinish(OBuffer &outbuf)
{
    return flushspxbits(outbuf);
}


bool WvSpeexEncoder::flushspxbits(OBuffer &outbuf)
{
    size_t needed = speex_bits_nbytes(spxbits);
    if (needed == 0)
        return true;

    while (needed != 0)
    {
        size_t avail = outbuf.optallocable();
        if (avail == 0)
            return false;
        if (avail > needed)
            avail = needed;
        speex_bits_write(spxbits, reinterpret_cast<char*>(
            outbuf.alloc(avail)), avail);
        needed -= avail;
    }
    // must reset before the next frame can be created
    speex_bits_reset(spxbits);
    return true;
}


int WvSpeexEncoder::samplingrate() const
{
    int rate;
    speex_encoder_ctl(const_cast<void*>(spxstate),
        SPEEX_GET_SAMPLING_RATE, &rate);
    return rate;
}


WvSpeex::CodecMode WvSpeexEncoder::mode() const
{
    return WvSpeex::CodecMode(spxmode->modeID);
}


bool WvSpeexEncoder::vbr() const
{
    int enabled;
    speex_encoder_ctl(const_cast<void*>(spxstate),
        SPEEX_GET_VBR, &enabled);
    return enabled;
}


int WvSpeexEncoder::nominalbitrate() const
{
    int bitrate;
    speex_encoder_ctl(const_cast<void*>(spxstate),
        SPEEX_GET_BITRATE, &bitrate);
    return bitrate;
}



/***** WvSpeexDecoder *****/

WvSpeexDecoder::WvSpeexDecoder(int samplingrate, int channels,
    WvSpeex::CodecMode modeid) :
    _samplingrate(samplingrate), _channels(channels),
    spxstate(NULL), spxbits(NULL), spxmode(NULL),
    _samplesperframe(0)
{
    // init decoder
    spxmode = get_speex_mode(modeid, samplingrate);
    spxstate = speex_decoder_init(spxmode);
    if (! spxstate)
    {
        seterror("error during speex_decoder_init");
        return;
    }
    spxbits = new SpeexBits;
    speex_bits_init(spxbits);
    
    // set sampling rate
    speex_decoder_ctl(spxstate, SPEEX_SET_SAMPLING_RATE,
        & samplingrate);

    // cache frame size since we use it often
    speex_decoder_ctl(spxstate, SPEEX_GET_FRAME_SIZE,
        &_samplesperframe);
}


WvSpeexDecoder::~WvSpeexDecoder()
{
    speex_decoder_destroy(spxstate);
    speex_bits_destroy(spxbits);
    delete spxbits;
}


WvSpeex::CodecMode WvSpeexDecoder::mode() const
{
    return WvSpeex::CodecMode(spxmode->modeID);
}


bool WvSpeexDecoder::postfilter() const
{
    int enabled;
    speex_decoder_ctl(spxstate, SPEEX_GET_ENH, &enabled);
    return enabled;
}


void WvSpeexDecoder::setpostfilter(bool enable)
{
    int enabled = enable ? 1 : 0;
    speex_decoder_ctl(spxstate, SPEEX_SET_ENH, &enabled);
}


bool WvSpeexDecoder::_typedencode(IBuffer &inbuf, OBuffer &outbuf,
    bool flush)
{
    for (;;)
    {
        size_t avail = inbuf.used();
        if (avail == 0)
            return true;
        if (outbuf.free() < size_t(_samplesperframe))
            return false; // not enough room

        size_t skip = 0;
        if (avail > MAX_BYTES_PER_FRAME)
        {
            // packet is too large but try to decode some audio anyhow
            skip = avail - MAX_BYTES_PER_FRAME;
            avail -= skip;
        }
        speex_bits_read_from(spxbits, const_cast<char*>(
            reinterpret_cast<const char*>(inbuf.get(avail))), avail);
        float *outsamples = outbuf.alloc(_samplesperframe);
        int retval = speex_decode(spxstate, spxbits, outsamples);
        inbuf.skip(skip); // skip over bad data
        if (retval != 0)
            return false; // signal bad data but don't stop on error
        if (! flush)
            return true;
    }
}


bool WvSpeexDecoder::_typedfinish(OBuffer &outbuf)
{
    return true;
}


bool WvSpeexDecoder::missing(OBuffer &outbuf)
{
    if (outbuf.free() < size_t(_samplesperframe))
        return false; // not enough room
    speex_decode(spxstate, NULL, outbuf.alloc(_samplesperframe));
    return true;
}
