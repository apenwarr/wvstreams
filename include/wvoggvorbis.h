/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Provides a WvEncoder abstraction for Ogg vorbis files.
 * Only monaural audio is supported for now.
 */
#ifndef __WVOGGVORBIS_H
#define __WVOGGVORBIS_H

#include "wvtypedencoder.h"
#include "wvstringlist.h"
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>

/**
 * Encodes PCM audio using the Ogg Vorbis stream format.
 * 
 * Input buffer must contain a sequence of signed 'float' type
 * values in machine order representing normalized PCM audio data.
 * 
 * Outbut buffer will contain part of an Ogg bitstream.
 * 
 */
class WvOggVorbisEncoder :
    public WvTypedEncoder<float, unsigned char>
{
public:
    static const long RANDOM_SERIALNO = 0;

    /**
     * Bitrate specification.
     * 
     * Identifies a particular bitrate control mechanism.
     * Use one of the subclasses to initialize a suitable BitrateSpec.
     * 
     */
    class BitrateSpec
    {
        friend class WvOggVorbisEncoder;
    protected:
        enum Mode { VBR_QUALITY, VBR_BITRATE };
        Mode mode;
        float quality_index;
        long max_bitrate;
        long nominal_bitrate;
        long min_bitrate;
        
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
     * Specifies a variable bitrate based on max, nominal, and min
     * bitrates specified in bits per second.
     */
    class VBRBitrate : public BitrateSpec
    {
    public:
        /**
         * Creates a bitrate specification.
         * "nominal" is the nominal bitrate
         */
        VBRBitrate(long nominal) : BitrateSpec(VBR_BITRATE)
        {
            max_bitrate = -1;
            nominal_bitrate = nominal;
            min_bitrate = -1;
        }
        /**
         * Creates a bitrate specification.
         * "max" is the maximum bitrate
         * "nominal" is the nominal bitrate
         * "min" is the minimum bitrate
         */
        VBRBitrate(long max, long nominal, long min) :
            BitrateSpec(VBR_BITRATE)
        {
            max_bitrate = max;
            nominal_bitrate = nominal;
            min_bitrate = min;
        }
    };
    
    /**
     * Creates an Ogg Vorbis Encoder.
     * 
     * The special constant RANDOM_SERIALNO may be specified as the
     * serial number to let the encoder choose one at random.  The
     * implementation uses the rand() function and assumes that
     * the PRNG was previously seeded with srand().
     * 
     * "bitrate" is the bitrate specification
     * "samplingrate" is the number of samples per second
     * "channels" is number of channels (must be 1 for now),
     *        defaults to 1
     * "serialno" is the Ogg bitstream serial number,
     *        defaults to RANDOM_SERIALNO
     */
    WvOggVorbisEncoder(const BitrateSpec &bitratespec,
        int samplingrate, int channels = 1,
        long serialno = RANDOM_SERIALNO);

    virtual ~WvOggVorbisEncoder();

    /**
     * Adds a comment to the Ogg Vorbis stream.
     * 
     * Do not call after the first invocation of encode().
     * 
     * "comment" is the comment
     */
    void add_comment(WvStringParm comment);
    
    /**
     * Adds a comment to the Ogg Vorbis stream.
     * 
     * Do not call after the first invocation of encode().
     * 
     */
    void add_comment(WVSTRING_FORMAT_DECL)
        { add_comment(WvString(WVSTRING_FORMAT_CALL)); }
    
    /**
     * Adds a tag to the Ogg Vorbis stream.
     * 
     * Ogg Vorbis tags are special comment strings of the form
     * "=" and are typically used to store artist,
     * date, and other simple string encoded metadata.
     * 
     * Do not call after the first invocation of encode().
     * 
     * "tag" is the tag name
     * "value" is the value
     */
    void add_tag(WvStringParm tag, WvStringParm value);

protected:
    virtual bool _typedencode(IBuffer &inbuf, OBuffer &outbuf,
        bool flush);
    virtual bool _typedfinish(OBuffer &outbuf);

private:
    ogg_stream_state *oggstream;
    vorbis_info *ovinfo;
    vorbis_comment *ovcomment;
    vorbis_dsp_state *ovdsp;
    vorbis_block *ovblock;
    bool wrote_header;

    bool write_header(OBuffer &outbuf);
    bool write_stream(OBuffer &outbuf, bool flush = false);
    bool process_audio(OBuffer &outbuf);
};


/**
 * Decodes PCM audio using the Ogg Vorbis stream format.
 * 
 * Inbut buffer must contain part of an Ogg bitstream.
 * 
 * Output buffer will contain a sequence of signed 'float' type
 * values in machine order representing normalized PCM audio data.
 * 
 * If flush == false, then encode() will return true immediately
 * after isheaderok() becomes true without outputting any audio
 * data.  This allows the client to examine the header and to
 * tailor the actual decoding process based on that information.
 * 
 */
class WvOggVorbisDecoder :
    public WvTypedEncoder<unsigned char, float>
{
    WvStringList comment_list;

public:
    /**
     * Creates a new Ogg Vorbis Decoder.
     * 
     * For now, if the input bitstream is stereo, outputs the left
     * channel only.  This behaviour may change later on.
     * 
     */
    WvOggVorbisDecoder();
    virtual ~WvOggVorbisDecoder();

    /**
     * Returns true when the entire stream header has been processed
     * and the comments and vendor fields are valid.
     * 
     * If false and isok(), try decoding more data.
     * 
     * Returns: true when the header has been decoded
     */
    bool isheaderok() const;

    /**
     * Returns the Ogg Vorbis vendor comment string.
     *
     * Returns: the vendor comment
     */
    WvString vendor() const
        { return ovcomment->vendor; }

    /**
     * Returns the Ogg Vorbis list of user comments.
     * 
     * The list is owned by the encoder, do not change.
     * 
     * Returns: the list of comments
     */
    WvStringList &comments()
        { return comment_list; }

    /**
     * Returns the number of channels in the stream.
     *
     * Returns: the number of channels, non-negative
     */
    int channels() const
        { return ovinfo->channels; }

    /**
     * Returns the sampling rate of the stream.
     *
     * Returns: the sampling rate
     */
    int samplingrate() const
        { return ovinfo->rate; }
    
protected:
    virtual bool _typedencode(IBuffer &inbuf, OBuffer &outbuf,
        bool flush);
    virtual bool _typedfinish(OBuffer &outbuf);

private:
    ogg_sync_state *oggsync;
    ogg_stream_state *oggstream;
    ogg_page *oggpage;
    vorbis_info *ovinfo;
    vorbis_comment *ovcomment;
    vorbis_dsp_state *ovdsp;
    vorbis_block *ovblock;
    bool need_serialno;
    int need_headers;

    bool process_page(ogg_page *oggpage, OBuffer &outbuf);
    bool process_packet(ogg_packet *oggpacket, OBuffer &outbuf);
    bool prepare_dsp();
    bool prepare_stream(long serialno);
};

#endif // __WVOGGVORBIS
