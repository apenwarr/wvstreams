/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * Provides a WvEncoder abstraction for Ogg vorbis files.
 */
#include "wvoggvorbis.h"
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <unistd.h>

#define OGG_VORBIS_ENCODER_BUF_SIZE 1024  // 1k samples for Vorbis
#define OGG_VORBIS_DECODER_BUF_SIZE 16384 // at least 8k for Vorbis

/***** WvOggVorbisEncoder *****/

WvOggVorbisEncoder::WvOggVorbisEncoder(
    const BitrateSpec &bitratespec,
    int samplingrate, int channels, long serialno) :
    oggstream(NULL), ovinfo(NULL), ovcomment(NULL),
    ovdsp(NULL), ovblock(NULL), wrote_header(false)
{
    // pick a serial number
    if (serialno == RANDOM_SERIALNO)
    {
        serialno = rand();
    }

    // init ogg bitstream layer
    int retval;
    oggstream = new ogg_stream_state;
    if ((retval = ogg_stream_init(oggstream, serialno)) != 0)
    {
        seterror("error %s during ogg_stream_init", retval);
        return;
    }
    
    // init vorbis codec layer
    ovinfo = new vorbis_info;
    vorbis_info_init(ovinfo);
    
    ovcomment = new vorbis_comment;
    vorbis_comment_init(ovcomment);

    // init vorbis bitrate management
    switch (bitratespec.mode)
    {
        case BitrateSpec::VBR_QUALITY:
            if ((retval = vorbis_encode_init_vbr(ovinfo, channels,
                samplingrate, bitratespec.quality)) < 0)
            {
                seterror("error %s during vorbis_encode_init_vbr", retval);
                return;
            }
            break;
        
        case BitrateSpec::VBR_BITRATE:
            if ((retval = vorbis_encode_init(ovinfo, channels, samplingrate,
                bitratespec.max_bitrate, bitratespec.nominal_bitrate,
                bitratespec.min_bitrate)) < 0)
            {
                seterror("error %s during vorbis_encode_init", retval);
                return;
            }
            break;
    }

    // init vorbis dsp layer
    ovdsp = new vorbis_dsp_state;
    if ((retval = vorbis_analysis_init(ovdsp, ovinfo)) != 0)
    {
        seterror("error %s during vorbis_analysis_init", retval);
        return;
    }
    ovblock = new vorbis_block;
    if ((retval = vorbis_block_init(ovdsp, ovblock)) != 0)
    {
        seterror("error %s during vorbis_block_init", retval);
        return;
    }
}


WvOggVorbisEncoder::~WvOggVorbisEncoder()
{
    // destroy vorbis dsp layer
    if (ovblock)
    {
        vorbis_block_clear(ovblock);
        delete ovblock;
    }
    if (ovdsp)
    {
        vorbis_dsp_clear(ovdsp);
        delete ovdsp;
    }

    // destroy vorbis codec layer
    if (ovcomment)
    {
        vorbis_comment_clear(ovcomment);
        delete ovcomment;
    }
    if (ovinfo)
    {
        vorbis_info_clear(ovinfo);
        delete ovinfo;
    }

    // destroy ogg bitstream layer
    if (oggstream)
    {
        ogg_stream_clear(oggstream);
        delete oggstream;
    }
}

void WvOggVorbisEncoder::add_comment(WvStringParm comment)
{
    if (! comment) return;
    // Ogg Vorbis missing const qualifier in function prototype!
    char *str = const_cast<char *>(comment.cstr());
    vorbis_comment_add(ovcomment, str);
}


void WvOggVorbisEncoder::add_tag(WvStringParm tag, WvStringParm value)
{
    if (! tag || ! value) return;
    // Ogg Vorbis missing const qualifier in function prototype!
    char *tagstr = const_cast<char *>(tag.cstr());
    char *valuestr = const_cast<char *>(value.cstr());
    vorbis_comment_add_tag(ovcomment, tagstr, valuestr);
}


bool WvOggVorbisEncoder::_encode(WvBuffer &inbuf, WvBuffer &outbuf,
    bool flush)
{
    // write header pages if needed
    if (! wrote_header)
    {
        if (! write_header(outbuf))
            return false;
        wrote_header = true;
    }

    // write compressed audio pages
    for (;;)
    {
        // read in more data
        size_t ovsamples = inbuf.used() / sizeof(OggVorbisFloat);
        if (ovsamples == 0)
        {
            // no more data
            if (flush)
                if (! write_stream(outbuf, true))
                    return false;
            return true;
        }
        if (ovsamples > OGG_VORBIS_ENCODER_BUF_SIZE)
            ovsamples = OGG_VORBIS_ENCODER_BUF_SIZE;
                
        OggVorbisFloat **ovbuf = vorbis_analysis_buffer(ovdsp, ovsamples);
        if (ovbuf == NULL)
        {
            seterror("error allocating vorbis analysis buffer");
            return false;
        }
        size_t ovbufsize = ovsamples * sizeof(OggVorbisFloat);
        const unsigned char *indata = inbuf.get(ovbufsize);
        memcpy(ovbuf[0], indata, ovbufsize);
        vorbis_analysis_wrote(ovdsp, ovsamples);

        process_audio(outbuf);
    }
}


bool WvOggVorbisEncoder::_finish(WvBuffer &outbuf)
{
    // write header pages if needed
    if (! wrote_header)
    {
        if (! write_header(outbuf))
            return false;
        wrote_header = true;
    }

    // write EOF mark
    vorbis_analysis_wrote(ovdsp, 0);
    process_audio(outbuf);
    return true;
}


bool WvOggVorbisEncoder::write_header(WvBuffer &outbuf)
{
    // generate headers
    ogg_packet headers[3];
    int retval;
    if ((retval = vorbis_analysis_headerout(ovdsp, ovcomment,
        & headers[0], & headers[1], & headers[2])) != 0)
    {
        seterror("error %s during vorbis_analysis_headerout", retval);
        return false;
    }
        
    // push headers to ogg stream
    for (int i = 0; i < 3; ++i)
        ogg_stream_packetin(oggstream, & headers[i]); // always succeeds
        
    // flush to ensure next data packet is in its own page
    return write_stream(outbuf, true /*flush*/);
}


bool WvOggVorbisEncoder::write_stream(WvBuffer &outbuf, bool flush)
{
    ogg_page oggpage;
    for (;;)
    {
        if (flush)
        {
            int retval = ogg_stream_flush(oggstream, & oggpage);
            if (retval == 0)
                break; // no remaining data
            else if (retval < 0)
            {
                seterror("error %s during ogg_stream_flush", retval);
                return false;
            }
        }
        else
        {
            int retval = ogg_stream_pageout(oggstream, & oggpage);
            if (retval == 0)
                break; // not enough data
            else if (retval < 0)
            {
                seterror("error %s during ogg_stream_pageout", retval);
                return false;
            }
        }
        outbuf.put(oggpage.header, oggpage.header_len);
        outbuf.put(oggpage.body, oggpage.body_len);
    }
    return true;
}


bool WvOggVorbisEncoder::process_audio(WvBuffer &outbuf)
{
    while (vorbis_analysis_blockout(ovdsp, ovblock) == 1)
    {
        // we got a block!
        int retval = vorbis_analysis(ovblock, NULL);
        if (retval < 0)
        {
            seterror("error %s during vorbis_analysis", retval);
            return false;
        }
        retval = vorbis_bitrate_addblock(ovblock);
        if (retval < 0)
        {
            seterror("error %s during vorbis_bitrate_addblock", retval);
            return false;
        }
    
        // write out packets
        ogg_packet oggpacket;
        while (vorbis_bitrate_flushpacket(ovdsp, & oggpacket) > 0)
        {
            ogg_stream_packetin(oggstream, & oggpacket); // always succeeds
            if (! write_stream(outbuf, false))
                return false;
        }
    }
    return true;
}



/***** WvOggVorbisDecoder *****/

WvOggVorbisDecoder::WvOggVorbisDecoder() :
    oggsync(NULL), oggstream(NULL), ovinfo(NULL), ovcomment(NULL),
    ovdsp(NULL), ovblock(NULL), need_serialno(true), need_headers(3)
{
    int retval;
    
    // init ogg sync layer
    oggsync = new ogg_sync_state;
    if ((retval = ogg_sync_init(oggsync)) != 0)
    {
        seterror("error %s during ogg_sync_init", retval);
        return;
    }
    oggpage = new ogg_page;
}


WvOggVorbisDecoder::~WvOggVorbisDecoder()
{
    // destroy vorbis dsp layer
    if (ovblock)
    {
        vorbis_block_clear(ovblock);
        delete ovblock;
    }
    if (ovdsp)
    {
        vorbis_dsp_clear(ovdsp);
        delete ovdsp;
    }
    
    // destroy vorbis codec layer
    if (ovcomment)
    {
        vorbis_comment_clear(ovcomment);
        delete ovcomment;
    }
    if (ovinfo)
    {
        vorbis_info_clear(ovinfo);
        delete ovinfo;
    }

    // destroy ogg bitstream layer
    if (oggstream)
    {
        ogg_stream_clear(oggstream);
        delete oggstream;
    }
    
    // destroy ogg sync layer
    delete oggpage;
    ogg_sync_clear(oggsync);
    delete oggsync;
}


bool WvOggVorbisDecoder::isheaderok() const
{
    return need_headers == 0;
}


bool WvOggVorbisDecoder::_encode(WvBuffer &inbuf, WvBuffer &outbuf,
    bool flush)
{
    bool checkheaderok = ! isheaderok() && ! flush;
    for (;;)
    {
        //while (ogg_sync_pageout(oggsync, oggpage) != 1)
        while (ogg_sync_pageseek(oggsync, oggpage) <= 0)
        {
            // read in more data
            size_t oggbufsize = inbuf.used();
            if (oggbufsize == 0)
            {
                // no more data
                if (flush && oggsync->fill != 0)
                    return false;
                return true;
            }
            if (oggbufsize > OGG_VORBIS_DECODER_BUF_SIZE)
                oggbufsize = OGG_VORBIS_DECODER_BUF_SIZE;
                
            char *oggbuf = ogg_sync_buffer(oggsync, oggbufsize);
            if (oggbuf == NULL)
            {
                seterror("error allocating ogg sync buffer");
                return false;
            }
            const unsigned char *indata = inbuf.get(oggbufsize);
            memcpy(oggbuf, indata, oggbufsize);
            ogg_sync_wrote(oggsync, oggbufsize);
        }
        // we got a page!
        if (! process_page(oggpage, outbuf))
            return false;
            
        // detect end of stream
        if (oggstream->e_o_s)
        {
            setfinished();
            return true;
        }
        
        // return immediately after we see the header if not flushing
        // guarantee no data has been decoded yet since Ogg Vorbis
        // spec says that the audio data must begin on a fresh page
        // following the headers
        if (checkheaderok && isheaderok())
            return true;
    }
}


bool WvOggVorbisDecoder::_finish(WvBuffer &outbuf)
{
    if (! isheaderok())
    {
        seterror("failed to detect a vorbis stream");
        return false;
    }
    return true;
}


bool WvOggVorbisDecoder::process_page(ogg_page *oggpage,
    WvBuffer &outbuf)
{       
    if (need_serialno)
    {
        // attach to the first bitstream we find
        long serialno = ogg_page_serialno(oggpage);
        if (! prepare_stream(serialno))
            return false;
        need_serialno = false;
    }
    // submit the page to the bitstream
    if (ogg_stream_pagein(oggstream, oggpage) != 0)
    {
        // this page was bad, or did not match the stream's
        // serial number exactly, skip it
        return true;
    }
    // extract packets from the bitstream
    ogg_packet oggpacket;
    while (ogg_stream_packetout(oggstream, & oggpacket) > 0)
    {
        if (! process_packet(& oggpacket, outbuf))
            return false;
    }
    return true;
}


bool WvOggVorbisDecoder::process_packet(ogg_packet *oggpacket,
    WvBuffer &outbuf)
{
    if (need_headers > 0)
    {
        // read headers
        int result = vorbis_synthesis_headerin(ovinfo,
            ovcomment, oggpacket);
        if (result != 0)
        {
            seterror("error %s reading vorbis headers "
                "(not a vorbis stream?)", result);
            return false;
        }
        if (--need_headers == 0)
            return prepare_dsp();
    }
    else
    {
        // process a block of Vorbis data
        if (vorbis_synthesis(ovblock, oggpacket) != 0)
        {
            // bad data? skip it!
            return true;
        }
        vorbis_synthesis_blockin(ovdsp, ovblock);
    
        // synthesize PCM audio
        for (;;) {
            OggVorbisFloat **pcm;
            long samples = vorbis_synthesis_pcmout(ovdsp, &pcm);
            if (samples == 0) break;
            
            size_t numbytes = samples * sizeof(OggVorbisFloat);
            unsigned char *out = outbuf.alloc(numbytes);
            memcpy(out, pcm[0], numbytes);
            vorbis_synthesis_read(ovdsp, samples);
        }
    }
    return true;
}


bool WvOggVorbisDecoder::prepare_dsp()
{
    // extract comments
    for (int i = 0; i < ovcomment->comments; ++i)
    {
        char *c = ovcomment->user_comments[i];
        if (c)
            comment_list.append(new WvString(c), true);
    }

    // prepare OggVorbis dsp state
    int retval;
    ovdsp = new vorbis_dsp_state;
    if ((retval = vorbis_synthesis_init(ovdsp, ovinfo)) != 0)
    {
        seterror("error %s during vorbis_synthesis_init", retval);
        return false;
    }
    ovblock = new vorbis_block;
    if ((retval = vorbis_block_init(ovdsp, ovblock)) != 0)
    {
        seterror("error %s during vorbis_block_init", retval);
        return false;
    }
    return true;
}


bool WvOggVorbisDecoder::prepare_stream(long serialno)
{
    // init ogg bitstream layer
    oggstream = new ogg_stream_state;
    int retval;
    if ((retval = ogg_stream_init(oggstream, serialno)) != 0)
    {
        seterror("error %s during ogg_stream_init", retval);
        return false;
    }

    // init vorbis codec layer
    ovinfo = new vorbis_info;
    vorbis_info_init(ovinfo); // cannot fail
    ovcomment = new vorbis_comment;
    vorbis_comment_init(ovcomment); // cannot fail
    return true;
}
