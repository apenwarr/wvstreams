/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * Provides a WvEncoder abstraction for Ogg Speex audio streams
 * suitable for encoding voice at low bitrates.
 *
 * Only monaural audio is supported for now.
 */
#ifndef __WVOGGSPEEX_H
#define __WVOGGSPEEX_H

#include "wvstringlist.h"
#include "wvtypedencoder.h"
#include "wvspeex.h"
#include <ogg/ogg.h>

/**
 * Encodes PCM audio using the Ogg Speex stream format.
 * <p>
 * Input buffer must contain a sequence of signed 'float' type
 * values in machine order representing <em>normalized</em> PCM
 * audio data.
 * </p><p>
 * Outbut buffer will contain part of an Ogg Speex bitstream.
 * </p>
 * @see WvSpeexEncoder
 */
class WvOggSpeexEncoder :
    public WvTypedEncoder<float, unsigned char>
{
    WvSpeexEncoder *speexenc;
    ogg_int64_t packetno;
    WvString _vendor;
    WvStringList _comments;
    ogg_stream_state *oggstream;
    bool wrote_headers;
    
    WvInPlaceBuffer framebuf;

public:
    static const long RANDOM_SERIALNO = 0;

    /**
     * Creates an Ogg Speex Encoder.
     * <p>
     * The special constant RANDOM_SERIALNO may be specified as the
     * serial number to let the encoder choose one at random.  The
     * implementation uses the rand() function and assumes that
     * the PRNG was previously seeded with srand().
     * </p>
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
     * @param serialno the Ogg bitstream serial number
     */
    WvOggSpeexEncoder(const WvSpeex::BitrateSpec &bitratespec,
        int samplingrate, int channels = 1,
        WvSpeex::CodecMode mode = WvSpeex::DEFAULT_MODE,
        int complexity = WvSpeex::DEFAULT_COMPLEXITY,
        long serialno = RANDOM_SERIALNO);

    virtual ~WvOggSpeexEncoder();

    /**
     * Adds a comment to the Ogg Speex stream.
     * <p>
     * Do not call after the first invocation of encode().
     * </p>
     * @param comment the comment
     */
    void add_comment(WvStringParm comment);
    
    /**
     * Adds a comment to the Ogg Speex stream.
     * <p>
     * Do not call after the first invocation of encode().
     * </p>
     */
    void add_comment(WVSTRING_FORMAT_DECL)
        { add_comment(WvString(WVSTRING_FORMAT_CALL)); }
    
    /**
     * Adds a tag to the Ogg Speex stream.
     * <p>
     * Ogg Speex tags are special comment strings of the form
     * "<tag>=<value>" and are typically used to store artist,
     * date, and other simple string encoded metadata.
     * </p><p>
     * Do not call after the first invocation of encode().
     * </p>
     * @param tag the tag name
     * @param value the value
     */
    void add_tag(WvStringParm tag, WvStringParm value);

    /**
     * Returns the sampling rate.
     * @return the sampling rate
     */
    int samplingrate() const
        { return speexenc->samplingrate(); }

    /**
     * Returns the number of channels.
     * @return the number of channels
     */
    int channels() const
        { return speexenc->channels(); }

    /**
     * Returns the number of samples per frame.
     * @return the frame size
     */
    int samplesperframe() const
        { return speexenc->samplesperframe(); }

    /**
     * Returns the current encoding mode.
     * @return the encoding mode
     */
    WvSpeex::CodecMode mode() const
        { return speexenc->mode(); }

    /**
     * Returns true if variable bitrate support has been enabled.
     * @return true if it is enabled
     */
    bool vbr() const
        { return speexenc->vbr(); }

    /**
     * Returns the nominal bitrate.
     * @return the bitrate, or -1 if not specified or not meaningful
     */
    int nominalbitrate() const
        { return speexenc->nominalbitrate(); }

protected:
    virtual bool _typedencode(IBuffer &inbuf, OBuffer &outbuf,
        bool flush);
    virtual bool _typedfinish(OBuffer &outbuf);
    virtual bool _isok() const;
    virtual WvString _geterror() const;

private:
    bool write_headers(OBuffer &outbuf);
    bool write_eof(OBuffer &outbuf);
    bool write_stream(OBuffer &outbuf, bool flush = false);
};


/**
 * Decodes PCM audio using the Ogg Speex stream format.
 * <p>
 * Inbut buffer must contain part of an Ogg Speex bitstream.
 * </p><p>
 * Output buffer will contain a sequence of signed 'float' type
 * values in machine order representing <em>normalized</em> PCM
 * audio data.
 * </p><p>
 * If flush == false, then encode() will return true immediately
 * after isheaderok() becomes true without outputting any audio
 * data.  This allows the client to examine the header and to
 * tailor the actual decoding process based on that information.
 * </p>
 * @see WvSpeexDecoder
 */
class WvOggSpeexDecoder :
    public WvTypedEncoder<unsigned char, float>
{
    WvSpeexDecoder *speexdec;
    bool forcepostfilter;
    WvString _vendor;
    WvStringList _comments;
    bool _vbr; // extracted from header
    int _nominalbitrate; // extracted from header

public:
    /**
     * Creates an Ogg Speex Decoder.
     * <p>
     * For now, if the input bitstream is stereo, outputs the left
     * channel only.  This behaviour may change later on.
     * </p>
     */
    WvOggSpeexDecoder();
    virtual ~WvOggSpeexDecoder();

    /**
     * Returns true when the entire stream header has been processed
     * and the comments and vendor fields are valid.
     * <p>
     * If false and isok(), try decoding more data.
     * </p>
     * @return true when the header has been decoded
     */
    bool isheaderok() const;

    /**
     * Returns the Ogg Speex vendor comment string.
     *
     * @return the vendor comment
     */
    WvString vendor() const
        { return _vendor; }

    /**
     * Returns the Ogg Speex list of user comments.
     * <p>
     * The list is owned by the encoder, do not change.
     * </p>
     * @return the list of comments
     */
    WvStringList &comments()
        { return _comments; }

    /**
     * Returns the number of channels in the stream.
     * Does not return useful information unless isheaderok() == true.
     * @return the number of channels, non-negative
     */
    int channels() const;

    /**
     * Returns the sampling rate of the stream.
     * Does not return useful information unless isheaderok() == true.
     * @return the sampling rate
     */
    int samplingrate() const;
        
    /**
     * Returns the number of samples per frame.
     * Does not return useful information unless isheaderok() == true.
     * @return the frame size
     */
    int samplesperframe() const;

    /**
     * Returns the current encoding mode.
     * Does not return useful information unless isheaderok() == true.
     * @return the encoding mode
     */
    WvSpeex::CodecMode mode() const;
    
    /**
     * Returns true if variable bitrate support has been enabled.
     * Does not return useful information unless isheaderok() == true.
     * @return true if it is enabled
     */
    bool vbr() const;

    /**
     * Returns the nominal bitrate.
     * Does not return useful information unless isheaderok() == true.
     * @return the bitrate, or -1 if not specified or not meaningful
     */
    int nominalbitrate() const;
    
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
    virtual bool _isok() const;
    virtual WvString _geterror() const;

private:
    ogg_sync_state *oggsync;
    ogg_stream_state *oggstream;
    ogg_page *oggpage;
    bool need_serialno;
    int need_headers;

    bool process_page(ogg_page *oggpage, OBuffer &outbuf);
    bool process_packet(ogg_packet *oggpacket, OBuffer &outbuf);
    bool prepare_stream(long serialno);

    bool process_speex_header(ogg_packet *header);
    bool process_comment_header(ogg_packet *header);
};

#endif // __WVOGGSPEEX_H
