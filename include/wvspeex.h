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
#ifndef __WVSPEEX_H
#define __WVSPEEX_H

#include "wvaudioencoder.h"

struct SpeexMode;
struct SpeexBits;

namespace WvSpeex
{
    /**
     * The default encoder complexity level.
     */
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
     * <p>
     * Identifies a particular bitrate control mechanism.
     * Use one of the subclasses to initialize a suitable BitrateSpec.
     * </p>
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
         * @param quality the quality index
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
         * @param nominal the nominal bitrate
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
         * @param bitrate the fixed bitrate
         */
        CBRQuality(float quality) : BitrateSpec(CBR_QUALITY)
        {
            quality_index = quality;
        }
    };
}; // namespace



/**
 * Encodes PCM audio using the Speex audio packet format.
 * <p>
 * Input buffer must contain a sequence of signed 'float' type
 * values in machine order representing <em>unnormalized</em> PCM
 * audio data.
 * </p><p>
 * Outbut buffer will contain a sequence of Speex packets.  Each
 * invocation of encode() with flush == false will generate
 * precisely one Speex packet suitable for use with unreliable
 * datagram transmission protocols that guarantee serial packet
 * order on reception.  Each packet contains one frame of exactly
 * 20ms of encoded audio in narrowband mode (sampling rate
 * <= 12.5Khz).
 * </p><p>
 * <b>Warning:</b> Never invoke encode() with flush == true unless
 * the input buffer contains exactly zero or one frame of audio.
 * Speex packets do not contain any delimiters therefore it is not
 * possible to locate the boundary between adjacent packets unless
 * they are encapsulated as individual datagrams in some fashion.
 * With flush == true, multiple adjacent generated packets will run
 * together to form one large undecodable lump.
 * </p><p>
 * For archival purposes or for streaming, consider using
 * WvOggSpeexEncoder.
 * </p><p>
 * For encoding music or other non-speech audio, consider using
 * WvOggVorbisEncoder.
 * </p>
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
     * @param bitrate the bitrate specification
     * @param samplingrate the number of samples per second,
     *        preferably one of 8000, 16000, or 32000
     * @param channels number of channels (must be 1 for now),
     *        defaults to 1
     * @param mode the Speex codec mode to use or
     *        WvSpeex::DEFAULT_MODE to select one automatically
     *        based on the sampling rate, this is the default
     * @param complexity a measure of the amount of CPU
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
     * @return the sampling rate
     */
    int samplingrate() const;
    
    /**
     * Returns the number of channels.
     * @return the number of channels
     */
    virtual unsigned int channels() const
        { return _channels; }

    /**
     * Returns the number of samples per frame.
     * @return the frame size
     */
    virtual size_t samplesperframe() const
        { return _samplesperframe; }

    /**
     * Returns the current encoding mode.
     * @return the encoding mode
     */
    WvSpeex::CodecMode mode() const;

    /**
     * Returns true if variable bitrate support has been enabled.
     * @return true if it is enabled
     */
    bool vbr() const;

    /**
     * Returns the nominal bitrate.
     * @return the bitrate, or -1 if not specified or not meaningful
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
 * <p>
 * Inbut buffer must contain a sequence of Speex packets.
 * </p><p>
 * Output buffer will contain a sequence of signed 'float' type
 * values in machine order representing <em>unnormalized</em> PCM
 * audio data.
 * </p><p>
 * Missing audio due to lost or damaged packets may be filled in
 * by making predictions (guesses) based on residual energy
 * information from previous ones.  The number of lost or damaged
 * packets must be known in order to calculate how much new audio
 * must be synthesized.  This technique works well to conceal
 * occasional dropouts but not long strings of lost packets.
 * Still, Speech is still surprizingly recognizable with average
 * packet losses of up to 25% to 50%!
 * </p><p>
 * <b>Warning:</b> Never invoke encode() unless the input buffer
 * contains exactly zero or one Speex packets. Speex packets
 * do not contain any delimiters therefore it is not possible to
 * locate the boundary between adjacent packets unless they are
 * encapsulated as individual datagrams in some fashion.
 * Multiple adjacent packets cannot be decoded at once.
 * </p><p>
 * For archival purposes or for streaming, consider using
 * WvOggSpeexDecoder.
 * </p><p>
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
     * <p>
     * For now, if the input bitstream is stereo, outputs the left
     * channel only.  This behaviour may change later on.
     * </p>
     * @param samplingrate the number of samples per second,
     *        preferably one of 8000, 16000, or 32000
     * @param channels number of channels (must be 1 for now),
     *        defaults to 1
     * @param mode the Speex codec mode to use or
     *        WvSpeex::DEFAULT_MODE to select one automatically
     *        based on the sampling rate, this is the default
     */
    WvSpeexDecoder(int samplingrate, unsigned int channels = 1,
        WvSpeex::CodecMode mode = WvSpeex::DEFAULT_MODE);

    virtual ~WvSpeexDecoder();
    
    /**
     * Synthesizes one audio frame to compensate for a missing packet.
     * @param outbuf the output buffer
     * @return true on success
     * @see encode
     */
    virtual bool missing(OBuffer &outbuf);

    /**
     * Returns the number of channels in the stream.
     * @return the number of channels, non-negative
     */
    virtual unsigned int channels() const
        { return _channels; }

    /**
     * Returns the sampling rate of the stream.
     * @return the sampling rate
     */
    int samplingrate() const
        { return _samplingrate; }

    /**
     * Returns the number of samples per frame.
     * @return the frame size
     */
    virtual size_t samplesperframe() const
        { return _samplesperframe; }

    /**
     * Returns the current encoding mode.
     * @return the encoding mode
     */
    WvSpeex::CodecMode mode() const;
    
    /**
     * Determines if the perceptual enhancement post-filter is enabled.
     * @return true if it is enabled
     */
    bool postfilter() const;

    /**
     * Enables or disables the perceptual enhancement post-filter.
     * @param enable true or false
     */
    void setpostfilter(bool enable);

protected:
    virtual bool _typedencode(IBuffer &inbuf, OBuffer &outbuf,
        bool flush);
    virtual bool _typedfinish(OBuffer &outbuf);
};

#endif // __WVSPEEX_H
