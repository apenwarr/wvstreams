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
#include "wvoggspeex.h"
#include <ogg/ogg.h>
#include <speex.h>
#include <speex_header.h>
#include <unistd.h>

#define OGG_SPEEX_DECODER_BUF_SIZE 16384 // at most 16k at once

/**
 * Extracts a little endian integer from a buffer.
 */
static unsigned long int getint_le(WvBuffer &inbuf)
{
    // FIXME: a little sloppy
    return inbuf.getch() | (inbuf.getch() << 8) |
        (inbuf.getch() << 16) | (inbuf.getch() << 24);
}


/**
 * Appends a little endian integer to a buffer.
 */
static void putint_le(WvBuffer &outbuf, unsigned long int value)
{
    // FIXME: a little sloppy
    outbuf.putch(value & 255);
    outbuf.putch((value >> 8) & 255);
    outbuf.putch((value >> 16) & 255);
    outbuf.putch(value >> 24);
}


/***** WvOggSpeexEncoder *****/

WvOggSpeexEncoder::WvOggSpeexEncoder(
    const WvSpeex::BitrateSpec &bitratespec, int samplingrate,
    int channels, WvSpeex::CodecMode mode, int complexity,
    long serialno) :
    speexenc(NULL), packetno(0),
    _vendor("Encoded with Speex"),
    oggstream(NULL), wrote_headers(false),
    framebuf(MAX_BYTES_PER_FRAME)
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

    // init speex encoder
    speexenc = new WvSpeexEncoder(bitratespec, samplingrate, channels,
        mode, complexity);
}


WvOggSpeexEncoder::~WvOggSpeexEncoder()
{
    // destroy speex encoder
    delete speexenc;

    // destroy ogg bitstream layer
    if (oggstream)
    {
        ogg_stream_clear(oggstream);
        delete oggstream;
    }
}


bool WvOggSpeexEncoder::_isok() const
{
    return speexenc ? speexenc->isok() : true;
}


WvString WvOggSpeexEncoder::_geterror() const
{
    return speexenc ? speexenc->geterror() : WvString(WvString::null);
}


void WvOggSpeexEncoder::add_comment(WvStringParm comment)
{
    _comments.append(new WvString(comment), true);
}


void WvOggSpeexEncoder::add_tag(WvStringParm tag, WvStringParm value)
{
    _comments.append(new WvString("%s=%s", tag, value), true);
}


bool WvOggSpeexEncoder::_typedencode(IBuffer &inbuf, OBuffer &outbuf,
    bool flush)
{
    // write header pages if needed
    if (! wrote_headers)
    {
        if (! write_headers(outbuf))
            return false;
        wrote_headers = true;
    }

    // write compressed audio pages
    for (;;)
    {
        // read in more data
        size_t samples = inbuf.used();
        if (samples == 0)
        {
            // no more data
            if (flush)
                if (! write_stream(outbuf, true))
                    return false;
            return true;
        }

        framebuf.zap();
        if (! speexenc->encode(inbuf, framebuf))
            return false;
        size_t bytes = framebuf.used();
        if (bytes == 0)
            return false; // not enough data

        // write out a packet
        ogg_packet oggpacket;
        oggpacket.packet = framebuf.ptr();
        oggpacket.bytes = bytes;
        oggpacket.b_o_s = 0;
        oggpacket.e_o_s = 0;
        oggpacket.granulepos = 0;
        oggpacket.packetno = packetno++;
        ogg_stream_packetin(oggstream, &oggpacket); // always succeeds
        if (! write_stream(outbuf, false))
            return false;
    }
}


bool WvOggSpeexEncoder::_typedfinish(OBuffer &outbuf)
{
    // write header pages if needed
    if (! wrote_headers)
    {
        if (! write_headers(outbuf))
            return false;
        wrote_headers = true;
    }
    return write_eof(outbuf);
}


bool WvOggSpeexEncoder::write_headers(OBuffer &outbuf)
{
    // generate stream header
    ogg_packet header;
    SpeexHeader spxheader;
    SpeexMode *spxmode = speex_mode_list[mode()];
    speex_init_header(&spxheader, samplingrate(), channels(), spxmode);
    spxheader.vbr = vbr();
    spxheader.bitrate = nominalbitrate();
    spxheader.frames_per_packet = 1;
    
    int size;
    header.packet = (unsigned char*)speex_header_to_packet(
        &spxheader, &size);
    header.bytes = size;
    header.b_o_s = 1;
    header.e_o_s = 0;
    header.granulepos = 0;
    header.packetno = packetno++;
    ogg_stream_packetin(oggstream, &header);

    // generate comment header
    WvDynamicBuffer cbuf;
    putint_le(cbuf, _vendor.len());
    cbuf.putstr(_vendor);
    putint_le(cbuf, _comments.count());
    WvStringList::Iter it(_comments);
    for (it.rewind(); it.next(); )
    {
        putint_le(cbuf, it->len());
        cbuf.putstr(*it);
    }
    header.bytes = cbuf.used();
    header.packet = const_cast<unsigned char *>(cbuf.get(header.bytes));
    header.b_o_s = 0;
    header.e_o_s = 0;
    header.granulepos = 0;
    header.packetno = packetno++;
    ogg_stream_packetin(oggstream, &header);
        
    // flush to ensure next data packet is in its own page
    return write_stream(outbuf, true /*flush*/);
}


bool WvOggSpeexEncoder::write_eof(OBuffer &outbuf)
{
    ogg_packet oggpacket;
    oggpacket.packet = (unsigned char*)"";
    oggpacket.bytes = 0;
    oggpacket.b_o_s = 0;
    oggpacket.e_o_s = 1;
    oggpacket.granulepos = 0;
    oggpacket.packetno = packetno++;
    ogg_stream_packetin(oggstream, &oggpacket);
    return write_stream(outbuf, true /*flush*/);
}


bool WvOggSpeexEncoder::write_stream(OBuffer &outbuf, bool flush)
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



/***** WvOggSpeexDecoder *****/

WvOggSpeexDecoder::WvOggSpeexDecoder() :
    speexdec(NULL), forcepostfilter(false),
    _vbr(false), _nominalbitrate(-1),
    oggsync(NULL), oggstream(NULL),
    need_serialno(true), need_headers(2)
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


WvOggSpeexDecoder::~WvOggSpeexDecoder()
{
    // destroy speex decoder
    delete speexdec;

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


bool WvOggSpeexDecoder::_isok() const
{
    return speexdec ? speexdec->isok() : true;
}


WvString WvOggSpeexDecoder::_geterror() const
{
    return speexdec ? speexdec->geterror() : WvString(WvString::null);
}


bool WvOggSpeexDecoder::isheaderok() const
{
    return need_headers == 0;
}


bool WvOggSpeexDecoder::_typedencode(IBuffer &inbuf, OBuffer &outbuf,
    bool flush)
{
    bool checkheaderok = ! isheaderok() && ! flush;
    for (;;)
    {
        // extract packets from the bitstream
        if (oggstream)
        {
            ogg_packet oggpacket;
            while (ogg_stream_packetout(oggstream, & oggpacket) > 0)
            {
                if (! process_packet(& oggpacket, outbuf))
                    return false;
            }

            // detect end of stream
            if (oggstream->e_o_s)
            {
                setfinished();
                return true;
            }
        }

        // get more pages
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
            if (oggbufsize > OGG_SPEEX_DECODER_BUF_SIZE)
                oggbufsize = OGG_SPEEX_DECODER_BUF_SIZE;
                
            char *oggbuf = ogg_sync_buffer(oggsync, oggbufsize);
            if (oggbuf == NULL)
            {
                seterror("error allocating ogg sync buffer");
                return false;
            }
            inbuf.move(oggbuf, oggbufsize);
            ogg_sync_wrote(oggsync, oggbufsize);
        }
        // we got a page!
        if (! process_page(oggpage, outbuf))
            return false;
        
        // return immediately after we see the header if not flushing
        // guarantee no data has been decoded yet since Ogg Speex
        // spec says that the audio data must begin on a fresh page
        // following the headers
        if (checkheaderok && isheaderok())
            return true;
    }
}


bool WvOggSpeexDecoder::_typedfinish(OBuffer &outbuf)
{
    if (! isheaderok())
    {
        seterror("failed to detect an Ogg Speex stream");
        return false;
    }
    return true;
}


bool WvOggSpeexDecoder::process_page(ogg_page *oggpage,
    OBuffer &outbuf)
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
    return true;
}


bool WvOggSpeexDecoder::process_packet(ogg_packet *oggpacket,
    OBuffer &outbuf)
{
    if (need_headers > 0)
    {
        // output headers
        bool success = need_headers == 2 ?
            process_speex_header(oggpacket) :
            process_comment_header(oggpacket);
        if (! success)
            return false;
        need_headers -= 1;
        return true;
    }

    // decode audio
    WvConstInPlaceBuffer buf(oggpacket->packet, oggpacket->bytes);
    return speexdec->flush(buf, outbuf);
}


bool WvOggSpeexDecoder::process_speex_header(ogg_packet *header)
{
    if (! header->b_o_s)
    {
        seterror("missing speex header at beginning of stream");
        return false;
    }
    SpeexHeader *spxheader = speex_packet_to_header(
        (char*)header->packet, header->bytes);
    if (! spxheader)
    {
        seterror("invalid speex header");
        return false;
    }
    if (spxheader->mode < 0 || spxheader->mode >= SPEEX_NB_MODES)
    {
        seterror("header contains an unrecognized or invalid codec mode");
        return false;
    }
    _vbr = spxheader->vbr;
    _nominalbitrate = spxheader->bitrate;
    
    // create the decoder
    speexdec = new WvSpeexDecoder(spxheader->rate, spxheader->nb_channels,
        WvSpeex::CodecMode(spxheader->mode));
    return true;
}


bool WvOggSpeexDecoder::process_comment_header(ogg_packet *header)
{
    if (! header->b_o_s && header->bytes >= 8)
    {
        WvConstInPlaceBuffer cbuf(header->packet, header->bytes);
        unsigned long int length = getint_le(cbuf);
        if (length <= cbuf.used() - 4)
        {
            _vendor = WvString(reinterpret_cast<const char*>(
                cbuf.get(length))).unique();
            unsigned long int count = getint_le(cbuf);
            while (count * 4 < cbuf.used())
            {
                length = getint_le(cbuf);
                if (length > cbuf.used())
                    break;
                WvString comment(reinterpret_cast<const char*>(
                    cbuf.get(length)));
                _comments.append(new WvString(comment.unique()), true);
                count -= 1;
            }
            if (count == 0)
                return true;
        }
    }
    seterror("invalid comment header");
    return false;
}


bool WvOggSpeexDecoder::prepare_stream(long serialno)
{
    // init ogg bitstream layer
    oggstream = new ogg_stream_state;
    int retval;
    if ((retval = ogg_stream_init(oggstream, serialno)) != 0)
    {
        seterror("error %s during ogg_stream_init", retval);
        return false;
    }
    return true;
}


int WvOggSpeexDecoder::channels() const
{
    return speexdec ? speexdec->channels() : 0;
}


int WvOggSpeexDecoder::samplingrate() const
{
    return speexdec ? speexdec->samplingrate() : 0;
}
 

int WvOggSpeexDecoder::samplesperframe() const
{
    return speexdec ? speexdec->samplesperframe() : 0;
}


WvSpeex::CodecMode WvOggSpeexDecoder::mode() const
{
    return speexdec ? speexdec->mode() : WvSpeex::NARROWBAND_MODE;
}


bool WvOggSpeexDecoder::vbr() const
{
    return _vbr;
}


int WvOggSpeexDecoder::nominalbitrate() const
{
    return _nominalbitrate;
}


bool WvOggSpeexDecoder::postfilter() const
{
    return speexdec ? speexdec->postfilter() : forcepostfilter;
}


void WvOggSpeexDecoder::setpostfilter(bool enable)
{
    forcepostfilter = enable;
    if (speexdec)
        speexdec->setpostfilter(enable);
}
