/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Provides a WvEncoder abstraction for Ogg vorbis files.
 * Only monaural audio is supported for now.
 */
#ifndef __WVOGGVORBIS_H
#define __WVOGGVORBIS_H

#include "wvencoder.h"
#include "wvfunctorencoder.h"
#include "wvstringlist.h"
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>

typedef float OggVorbisFloat;

/**
 * Encodes PCM audio using the Ogg Vorbis stream format.
 * Input buffer must contain a sequence of signed 'OggVorbisFloat' type
 *   values in machine order representing normalized PCM audio data.
 * Outbut buffer will contain part of an Ogg bitstream.
 */
class WvOggVorbisEncoder : public WvEncoder
{
public:
    static const long RANDOM_SERIALNO = 0;

    /**
     * Bitrate specifications.
     * Use one of the subclasses to initialize a suitable BitrateSpec.
     */
    class BitrateSpec
    {
        friend class WvOggVorbisEncoder;
    protected:
        enum Mode { VBR_QUALITY, VBR_BITRATE };
        Mode mode;
        OggVorbisFloat quality;
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
     * from 0.0 (low) to 1.0 (high).
     */
    class VBRQuality : public BitrateSpec
    {
    public:
        VBRQuality(OggVorbisFloat _quality) : BitrateSpec(VBR_QUALITY)
        {
            quality = _quality;
        }
    };
    /**
     * Specifies a variable bitrate based on max, nominal, and min
     * bitrates specified in bits per second.
     */
    class VBRBitrate : public BitrateSpec
    {
    public:
        VBRBitrate(long _nominal) : BitrateSpec(VBR_BITRATE)
        {
            max_bitrate = -1;
            nominal_bitrate = _nominal;
            min_bitrate = -1;
        }
        VBRBitrate(long _max, long _nominal, long _min) :
            BitrateSpec(VBR_BITRATE)
        {
            max_bitrate = _max;
            nominal_bitrate = _nominal;
            min_bitrate = _min;
        }
    };
    
    /**
     * Creates an Ogg Encoder.
     *   bitrate      - the bitrate specification (see above)
     *   samplingrate - samples per second
     *   channels     - number of channels, must be 1 for now
     *   serialno     - the bitstream serial number, RANDOM_SERIALNO
     *                  selects a random number using rand() assuming
     *                  the PRNG was previously seeded with srand().
     */
    WvOggVorbisEncoder(const BitrateSpec &bitratespec,
        int samplingrate = 44100, int channels = 1,
        long serialno = RANDOM_SERIALNO);
    virtual ~WvOggVorbisEncoder();

    /**
     * Adds a comment to the Ogg Vorbis stream.
     * Only use this before the first invocation of encode().
     */
    void add_comment(WvStringParm comment);
    void add_comment(WVSTRING_FORMAT_DECL) { add_comment(WvString(WVSTRING_FORMAT_CALL)); }
    
    /**
     * Adds a tag to the Ogg Vorbis stream.
     * Ogg Vorbis tags are special comment strings of the form
     * "<tag>=<value>" and are typically used to store artist,
     * date, and other simple metadata.
     * Only use this before the first invocation of encode().
     */
    void add_tag(WvStringParm tag, WvStringParm value);

protected:
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf, bool flush);
    virtual bool _finish(WvBuffer &outbuf);

private:
    ogg_stream_state *oggstream;
    vorbis_info *ovinfo;
    vorbis_comment *ovcomment;
    vorbis_dsp_state *ovdsp;
    vorbis_block *ovblock;
    bool wrote_header;

    bool write_header(WvBuffer &outbuf);
    bool write_stream(WvBuffer &outbuf, bool flush = false);
    bool process_audio(WvBuffer &outbuf);
};


/**
 * Decodes PCM audio using the Ogg Vorbis stream format.
 * Inbut buffer must contain part of an Ogg bitstream.
 * Output buffer will contain a sequence of signed 'OggVorbisFloat' type
 *   values in machine order representing normalized PCM audio data.
 */
class WvOggVorbisDecoder : public WvEncoder
{
    WvStringList comment_list;

public:
    /**
     * Creates a new Ogg Decoder.
     * If the input bitstream is stereo, outputs the left channel only.
     * This behaviour may change later on.
     */
    WvOggVorbisDecoder();
    virtual ~WvOggVorbisDecoder();

    /**
     * Returns true when the entire stream header has been processed
     * and the comments and vendor fields are valid.
     * If false and isok(), try decoding more data.
     */
    bool isheaderok() const;

    /**
     * Returns the Ogg Vorbis vendor comment string.
     */
    WvString vendor() const { return ovcomment->vendor; }

    /**
     * Returns the Ogg Vorbis list of user comments.
     * Note: list is owned by the decoder
     */
    WvStringList &comments() { return comment_list; }

    /**
     * Returns the number of channels in the stream.
     */
    int channels() const { return ovinfo->channels; }

    /**
     * Returns the sampling rate of the stream.
     */
    int samplingrate() const { return ovinfo->rate; }
    
protected:
    /**
     * Notes:
     *   If flush == false, returns true immediately after isheaderok()
     *   becomes true without outputting any audio data.  This allows
     *   the client to examine the header and to tailor the decoding
     *   process based on that information.
     */
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf, bool flush);
    virtual bool _finish(WvBuffer &outbuf);

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

    bool process_page(ogg_page *oggpage, WvBuffer &outbuf);
    bool process_packet(ogg_packet *oggpacket, WvBuffer &outbuf);
    bool prepare_dsp();
    bool prepare_stream(long serialno);
};


/**
 * Data type conversion and renormalization functors.
 */
struct WvSigned16ToOggVorbisFloatFunctor
{
    inline OggVorbisFloat operator()(signed short int pcm) const
    {
        return OggVorbisFloat(pcm) / 32768;
    }
};
struct WvOggVorbisFloatToSigned16Functor
{
    inline signed short int operator()(OggVorbisFloat pcm) const
    {
        return (signed short int)(pcm * 32768);
    }
};


/**
 * Instantiate some useful types for renormalizing data for Ogg Vorbis.
 * Use WvOggVorbisFloatToSigned16 on the decoder output and
 *     WvSigned16ToOggVorbisFloat on the encoder input.
 */
class WvOggVorbisFloatToSigned16 : public WvFunctorEncoder
    <signed short int, OggVorbisFloat, WvOggVorbisFloatToSigned16Functor>
{
public:
    WvOggVorbisFloatToSigned16() :
        WvFunctorEncoder<OT, IT, FT>(FT()) { }
};


class WvSigned16ToOggVorbisFloat : public WvFunctorEncoder
    <OggVorbisFloat, signed short int, WvSigned16ToOggVorbisFloatFunctor>
{
public:
    WvSigned16ToOggVorbisFloat() :
        WvFunctorEncoder<OT, IT, FT>(FT()) { }
};

#endif // __WVOGGVORBIS
