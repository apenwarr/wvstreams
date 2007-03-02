/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Provides a WvEncoder abstraction for the Speex audio packet format.
 * suitable for encoding voice at low bitrates.
 *
 * Only monaural audio is supported for now.
 */
#ifndef __WVSPEEX_H
#define __WVSPEEX_H

#include "wvaudioencoder.h"

struct SpeexMode;
struct SpeexBits;

namespace WvSpeex
{
    /** The default encoder complexity level. */
    static const int DEFAULT_COMPLEXITY = -1;
    
    /**
     * Describes an encoding algorithm used by the Speex codec.
     * Might also take on values not listed in the enum at this
     * time due to future codec enhancements.
     */
    enum CodecMode
    {
        DEFAULT_MODE = -1,      /*!< Chosen based on the sampling rate */
        NARROWBAND_MODE = 0,    /*!< Narrowband ~8khz, 20ms frames */
        WIDEBAND_MODE = 1,      /*!< Wideband ~16khz, ?ms frames */
        ULTRAWIDEBAND_MODE = 2  /*!< Ultrawideband ~32khz, ?ms frames */
    };

    /**
     * Bitrate specification.
     * 
     * Identifies a particular bitrate control mechanism.
     * Use one of the subclasses to initialize a suitable BitrateSpec.
     * 
     */
    class BitrateSpec
    {
    public:
        // TODO: check whether VBR_BITRATE is a valid mode
        enum Mode { VBR_QUALITY, CBR_QUALITY, CBR_BITRATE };
        Mode mode;
        float quality_index;
        int nominal_bitrate;
        
    protected:
        BitrateSpec(Mode mode) : mode(mode) { }

    public:
        // allow creation of uninitialized objects for later assignment
        BitrateSpec() { }
    };
    
    /**
     * Specifies a variable bitrate based on a quality index ranging
     * from 0.0 (low quality) to 1.0 (high quality).
     */
    class VBRQuality : public BitrateSpec
    {
    public:
        /**
         * Creates a bitrate specification.
         * "quality" is the quality index
         */
        VBRQuality(float quality) : BitrateSpec(VBR_QUALITY)
        {
            quality_index = quality;
        }
    };
    
    /**
     * Specifies a constant bitrate specified in bits per second.
     *
     * The encoder may adjust the bitrate according to internal
     * constraints, but guarantees that it will not select a
     * bitrate larger than that specified here.
     */
    class CBRBitrate : public BitrateSpec
    {
    public:
        /**
         * Creates a bitrate specification.
         * "nominal" is the nominal bitrate
         */
        CBRBitrate(int nominal) : BitrateSpec(CBR_BITRATE)
        {
            nominal_bitrate = nominal;
        }
    };
    
    /**
     * Specifies a constant bitrate based on a quality index ranging
     * from 0.0 (low quality) to 1.0 (high quality).
     */
    class CBRQuality : public BitrateSpec
    {
    public:
        /**
         * Creates a bitrate specification.
         * "bitrate" is the fixed bitrate
         */
        CBRQuality(float quality) : BitrateSpec(CBR_QUALITY)
        {
            quality_index = quality;
        }
    };
}; // namespace



/**
 * Encodes PCM audio using the Speex audio packet format.
 * 
 * Input buffer must contain a sequence of signed 'float' type
 * values in machine order representing unnormalized PCM
 * audio data.
 * 
 * Outbut buffer will contain a sequence of Speex packets.  Each
 * invocation of encode() with flush == false will generate
 * precisely one Speex packet suitable for use with unreliable
 * datagram transmission protocols that guarantee serial packet
 * order on reception.  Each packet contains one frame of exactly
 * 20ms of encoded audio in narrowband mode (sampling rate
 * <= 12.5Khz).
 * 
 * Warning: Never invoke encode() with flush == true unless
 * the input buffer contains exactly zero or one frame of audio.
 * Speex packets do not contain any delimiters therefore it is not
 * possible to locate the boundary between adjacent packets unless
 * they are encapsulated as individual datagrams in some fashion.
 * With flush == true, multiple adjacent generated packets will run
 * together to form one large undecodable lump.
 * 
 * For archival purposes or for streaming, consider using
 * WvOggSpeexEncoder.
 * 
 * For encoding music or other non-speech audio, consider using
 * WvOggVorbisEncoder.
 * 
 */
class WvSpeexEncoder : public WvAudioEncoder
{
    void *spxstate;
    SpeexBits *spxbits;
    SpeexMode *spxmode;
    unsigned int _channels;
    size_t _samplesperframe;
    
public:

    /**
     * Creates a Speex Encoder.
     *
     * "bitrate" is the bitrate specification
     * "samplingrate" is the number of samples per second,
     *        preferably one of 8000, 16000, or 32000
     * "channels" is number of channels (must be 1 for now),
     *        defaults to 1
     * "mode" is the Speex codec mode to use or
     *        WvSpeex::DEFAULT_MODE to select one automatically
     *        based on the sampling rate, this is the default
     * "complexity" is a measure of the amount of CPU
     *        resources that should be allocated to the encoder,
     *        ranges from 0 to 10 or WvSpeex::DEFAULT_COMPLEXITY
     *        the encoder default, this is the default
     */
    WvSpeexEncoder(const WvSpeex::BitrateSpec &bitratespec,
        int samplingrate, unsigned int channels = 1,
        WvSpeex::CodecMode mode = WvSpeex::DEFAULT_MODE,
        int complexity = WvSpeex::DEFAULT_COMPLEXITY);
        
    virtual ~WvSpeexEncoder();
    
    /**
     * Returns the sampling rate.
     * Returns: the sampling rate
     */
    int samplingrate() const;
    
    /**
     * Returns the number of channels.
     * Returns: the number of channels
     */
    virtual unsigned int channels() const
        { return _channels; }

    /**
     * Returns the number of samples per frame.
     * Returns: the frame size
     */
    virtual size_t samplesperframe() const
        { return _samplesperframe; }

    /**
     * Returns the current encoding mode.
     * Returns: the encoding mode
     */
    WvSpeex::CodecMode mode() const;

    /**
     * Returns true if variable bitrate support has been enabled.
     * Returns: true if it is enabled
     */
    bool vbr() const;

    /**
     * Returns the nominal bitrate.
     * Returns: the bitrate, or -1 if not specified or not meaningful
     */
    int nominalbitrate() const;

protected:
    virtual bool _typedencode(IBuffer &inbuf, OBuffer &outbuf,
        bool flush);
    virtual bool _typedfinish(OBuffer &outbuf);

private:
    bool flushspxbits(OBuffer &outbuf);
};



/**
 * Decodes PCM audio using the Speex audio packet format.
 * 
 * Inbut buffer must contain a sequence of Speex packets.
 * 
 * Output buffer will contain a sequence of signed 'float' type
 * values in machine order representing unnormalized PCM
 * audio data.
 * 
 * Missing audio due to lost or damaged packets may be filled in
 * by making predictions (guesses) based on residual energy
 * information from previous ones.  The number of lost or damaged
 * packets must be known in order to calculate how much new audio
 * must be synthesized.  This technique works well to conceal
 * occasional dropouts but not long strings of lost packets.
 * Still, Speech is still surprizingly recognizable with average
 * packet losses of up to 25% to 50%!
 * 
 * Warning: Never invoke encode() unless the input buffer
 * contains exactly zero or one Speex packets. Speex packets
 * do not contain any delimiters therefore it is not possible to
 * locate the boundary between adjacent packets unless they are
 * encapsulated as individual datagrams in some fashion.
 * Multiple adjacent packets cannot be decoded at once.
 * 
 * For archival purposes or for streaming, consider using
 * WvOggSpeexDecoder.
 * 
 * For encoding music or other non-speech audio, consider using
 * WvOggVorbisDecoder.
 */
class WvSpeexDecoder : public WvAudioDecoder
{
    int _samplingrate;
    unsigned int _channels;
    
    void *spxstate;
    SpeexBits *spxbits;
    SpeexMode *spxmode;
    size_t _samplesperframe;

public:
    /**
     * Creates a Speex Decoder.
     * 
     * For now, if the input bitstream is stereo, outputs the left
     * channel only.  This behaviour may change later on.
     * 
     * "samplingrate" is the number of samples per second,
     *        preferably one of 8000, 16000, or 32000
     * "channels" is number of channels (must be 1 for now),
     *        defaults to 1
     * "mode" is the Speex codec mode to use or
     *        WvSpeex::DEFAULT_MODE to select one automatically
     *        based on the sampling rate, this is the default
     */
    WvSpeexDecoder(int samplingrate, unsigned int channels = 1,
        WvSpeex::CodecMode mode = WvSpeex::DEFAULT_MODE);

    virtual ~WvSpeexDecoder();
    
    /**
     * Synthesizes one audio frame to compensate for a missing packet.
     * "outbuf" is the output buffer
     * Returns: true on success
     * @see encode
     */
    virtual bool missing(OBuffer &outbuf);

    /**
     * Returns the number of channels in the stream.
     * Returns: the number of channels, non-negative
     */
    virtual unsigned int channels() const
        { return _channels; }

    /**
     * Returns the sampling rate of the stream.
     * Returns: the sampling rate
     */
    int samplingrate() const
        { return _samplingrate; }

    /**
     * Returns the number of samples per frame.
     * Returns: the frame size
     */
    virtual size_t samplesperframe() const
        { return _samplesperframe; }

    /**
     * Returns the current encoding mode.
     * Returns: the encoding mode
     */
    WvSpeex::CodecMode mode() const;
    
    /**
     * Determines if the perceptual enhancement post-filter is enabled.
     * Returns: true if it is enabled
     */
    bool postfilter() const;

    /**
     * Enables or disables the perceptual enhancement post-filter.
     * "enable" is true or false
     */
    void setpostfilter(bool enable);

protected:
    virtual bool _typedencode(IBuffer &inbuf, OBuffer &outbuf,
        bool flush);
    virtual bool _typedfinish(OBuffer &outbuf);
};

#endif // __WVSPEEX_H
