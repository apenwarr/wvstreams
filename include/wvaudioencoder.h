/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2003 Net Integration Technologies, Inc.
 *
 * Interface declaring additional methods required for a WvEncoder that
 * supports packetized audio formats.
 */
#ifndef __WVAUDIOENCODER_H
#define __WVAUDIOENCODER_H

#include "wvtypedencoder.h"

/**
 * Abstract base class for encoders for PCM audio.  This interface should be
 * added to a WvEncoder.
 */
class WvAudioEncoder : public WvTypedEncoder<float, unsigned char>
{
public:

    /**
     * Returns the number of channels.
     * Returns: the number of channels
     */
    virtual unsigned int channels() const = 0;

    /**
     * Returns the number of samples per frame.
     * Returns: the frame size
     */
    virtual size_t samplesperframe() const = 0;
};


class WvAudioDecoder : public WvTypedEncoder<unsigned char, float>
{
public:

    /**
     * Returns the number of channels.
     * Returns: the number of channels
     */
    virtual unsigned int channels() const = 0;

    /**
     * Returns the number of samples per frame.
     * Returns: the frame size
     */
    virtual size_t samplesperframe() const = 0;

    /**
     * Synthesizes one audio frame to compensate for a missing packet.
     * "outbuf" is the output buffer
     * Returns: true on success
     * @see encode
     */
    virtual bool missing(OBuffer &outbuf) = 0;
};


class WvSimpleAudioEncoder : public WvAudioEncoder
{
public:

    WvSimpleAudioEncoder(unsigned int channels, unsigned int samplerate);

    virtual unsigned int channels() const { return _channels; }

    virtual size_t samplesperframe() const { return _samplesperframe; }

protected:

    virtual bool _typedencode(IBuffer &inbuf, OBuffer &outbuf, bool flush);

    virtual bool _typedfinish(OBuffer &outbuf);

private:

    unsigned int _channels;
    size_t _samplesperframe;
};


class WvSimpleAudioDecoder : public WvAudioDecoder
{
public:

    WvSimpleAudioDecoder(unsigned int channels, unsigned int samplerate);

    virtual unsigned int channels() const { return _channels; }

    virtual size_t samplesperframe() const { return _samplesperframe; }

    virtual bool missing(OBuffer &outbuf) { return false; }

protected:

    virtual bool _typedencode(IBuffer &inbuf, OBuffer &outbuf, bool flush);

    virtual bool _typedfinish(OBuffer &outbuf);

private:

    unsigned int _channels;
    size_t _samplesperframe;
};


#endif // __WVAUDIOENCODER_H
