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
     * Helper functions, encodes strings to buffers, strings to strings,
     * and buffers to strings.  The functions that take strings as input
     * or return them force false to true because incremental processing
     * of strings would require extra state.
     */
    bool encode(WvStringParm in, WvBuffer &out);
    bool encode(WvStringParm in, WvString &out);
    bool encode(WvBuffer &in, WvString &out, bool flush = false);

    inline bool flush(WvStringParm in, WvBuffer &out)
        { return encode(in, out); }
    inline bool flush(WvStringParm in, WvString &out)
        { return encode(in, out); }
    inline bool flush(WvBuffer &in, WvString &out)
        { return encode(in, out, true); }
    
    WvString strencode(WvStringParm in);
    WvString strencode(WvBuffer &in);
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
