/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * A top-level data encoder class.  You put data in, and when it comes out
 * it's encoded.  It might be a different size or not, and future data might
 * or might not depend on previous data, depending on the encoder type.
 * 
 * Decoders are also derived from WvEncoder, since they're
 * really just encoders too.
 */
#ifndef __WVENCODER_H
#define __WVENCODER_H

#include "wvbuffer.h"
#include "wvlinklist.h"

/**
 * A base encoder class.
 */
class WvEncoder
{
public:
    /**
     * Creates a new WvEncoder.
     */
    WvEncoder();

    /**
     * Destroys the encoder.  Unflushed data is lost.
     */
    virtual ~WvEncoder();
    
    /**
     * Returns true if the encoder can encode new data.
     * This should only be used to record permanent failures.
     * Transient errors (eg. bad block, but recoverable) should be
     * detected in a different fashion.
     *
     * The default implementation always returns true.
     */
    virtual bool isok() const;

    /**
     * Reads data from the input buffer, encodes it, and writes the result
     * to the output buffer.
     *
     * If flush == true, the input buffer will be drained and the output
     * buffer will contain all of the encoded data including any that
     * might have been buffered internally from previous calls.  Thus it
     * is possible that new data will be written to the output buffer even
     * though the input buffer was empty when encode() was called.  If the
     * buffer could not be fully drained because there was insufficient
     * data, this function returns false and leaves the remaining unflushed
     * data in the buffer.
     *
     * If flush == false, the encoder will read and encode as much data
     * as possible (or as it convenient) from the input buffer and store the
     * results in the output buffer.  Partial results may be buffered
     * internally by the encoder to be written to the output buffer later
     * when the encoder is flushed.
     *
     * If a permanent error occurs, then isok() will return false, this
     * function will return false and the input buffer will be left in an
     * undefined state.
     *
     * If a recoverable error occurs, the encoder should discard the
     * problematic data from the input buffer and return false from this
     * function, but isok() will remain true.
     *
     * Returns true on success or false if an error occurs.
     */
    virtual bool encode(WvBuffer &in, WvBuffer &out, bool flush = false) = 0;

    /**
     * Flushes the encoder (convenience function).
     */
    bool flush(WvBuffer &in, WvBuffer &out)
        { return encode(in, out, true); }

    /**
     * Helper functions for encoding strings.
     * Some variants have no encode(...) equivalent because they must
     * always flush.
     */
    bool flush(WvStringParm instr, WvBuffer &outbuf);
    bool flush(WvStringParm instr, WvString &outstr);
    bool encode(WvBuffer &inbuf, WvString &outstr, bool flush = false);

    inline bool flush(WvBuffer &inbuf, WvString &outstr)
        { return encode(inbuf, outstr, true); }
    
    WvString strflush(WvStringParm instr, bool ignore_errors = true);
    WvString strflush(WvBuffer &inbuf, bool ignore_errors = true);

    /**
     * Helper functions for encoder data from plain memory buffers.
     * Some variants have no encode(...) equivalent because they must
     * always flush.  The size_t pointer at by the outlen parameter
     * will be updated to reflect the actual number of bytes that
     * were stored into the output buffer.  If the outbuf buffer
     * is not large enough, the overflow bytes will be discarded
     * and false will be returned.
     */
    bool flush(const void *inmem, size_t inlen, WvBuffer &outbuf);
    bool flush(const void *inmem, size_t inlen, void *outmem,
        size_t *outlen);
    bool encode(WvBuffer &inbuf, void *outmem, size_t *outlen,
        bool flush = false);
        
    inline bool flush(WvBuffer &inbuf, void *outmem, size_t *outlen)
        { return encode(inbuf, outmem, outlen, true); }

    /**
     * Helper functions for other interesting cases.
     */
    bool flush(WvStringParm instr, void *outmem, size_t *outlen);
};


/**
 * An encoder that discards all of its input.
 */
class WvNullEncoder : public WvEncoder
{
public:
    virtual bool encode(WvBuffer &in, WvBuffer &out, bool flush);
};


/**
 * A very efficient null encoder that just sends its input to its output
 * without actually copying any data.
 */
class WvPassthroughEncoder : public WvEncoder
{
public:
    virtual bool encode(WvBuffer &in, WvBuffer &out, bool flush);
};


/**
 * An encoder chain owns a list of encoders that are used in sequence
 * to transform data from a source buffer to a target buffer.
 */
class WvEncoderChain : public WvEncoder
{
    class WvEncoderChainElem
    {
    public:
        WvEncoder *enc;
        WvBuffer out;
        bool auto_free;

        WvEncoderChainElem(WvEncoder *enc, bool auto_free) :
            enc(enc), auto_free(auto_free) { }
        ~WvEncoderChainElem() { if (auto_free) delete enc; }
    };
    DeclareWvList3(WvEncoderChainElem, WvEncoderChainElemListBase, );

    WvEncoderChainElemListBase encoders;
    WvPassthroughEncoder passthrough;
public:
    /**
     * Creates an initially empty chain of encoders.
     */
    WvEncoderChain();
    virtual ~WvEncoderChain();

    /**
     * Manipulate the list of encoders.
     * Probably should not do this unless the encoders have been flushed.
     */
    void append(WvEncoder *enc, bool auto_free);
    void prepend(WvEncoder *enc, bool auto_free);
    void unlink(WvEncoder *enc);

    /**
     * Returns true if and only if all encoders return true.
     */
    virtual bool isok() const;

    /**
     * Passes the data through the entire chain of encoders.
     * Returns true if and only if all encoders return true.
     */
    virtual bool encode(WvBuffer &in, WvBuffer &out, bool flush);
};

#endif // __WVENCODER_H
