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
    bool okay; // false iff setnotok() was called
    bool finished; // true iff setfinished()/finish() was called
    WvString errstr;

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
     */
    bool isok() const
        { return okay && _isok(); }

    /**
     * Returns true if finish has been called on this encoder.
     */
    bool isfinished() const
        { return finished || _isfinished(); }

    /**
     * Returns an error message if isok() == false, else the null string.
     */
    WvString geterror() const;

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
     * A stream might become isfinished() == true if an encoder-
     * specific end-of-data marker was detected in the input.
     *
     * Returns true on success or false if an error occurs.
     * Returns isok() if the stream had previously been finished, unless
     * the input buffer is not empty, in which case returns false.
     *
     * See _encode() for the actual implementation.
     */
    bool encode(WvBuffer &in, WvBuffer &out, bool flush = false);

    /**
     * Flushes the encoder (convenience function).
     */
    bool flush(WvBuffer &in, WvBuffer &out)
        { return encode(in, out, true); }

    /**
     * Tells the encoder that NO MORE DATA will ever be encoded.
     * It should flush out any and all internally buffered data
     * and write out whatever end-of-data marking it needs before
     * returning.
     *
     * Clients should invoke flush() on the input buffer before
     * finish() if the input buffer was not yet empty.
     *
     * It is safe to call this function multiple times.
     * The implementation will simply return isok() and do nothing else.
     *
     * Returns true on success or false if an error occurs.
     * Returns isok() if the stream had previously been finished.
     *
     * See _finish() for the actual implementation.
     */
    bool finish(WvBuffer &out);

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

    bool flush(WvStringParm instr, void *outmem, size_t *outlen);

protected:
    /**
     * Sets 'okay' to false explicitly.
     */
    void setnotok()
        { okay = false; }

    /**
     * Sets an error condition, then setnotok().
     */
    void seterror(WvStringParm message)
        { errstr = message; setnotok(); }
    void seterror(WVSTRING_FORMAT_DECL)
        { seterror(WvString(WVSTRING_FORMAT_CALL)); }

    /**
     * Sets 'finished' to true explicitly.
     */
    void setfinished()
        { finished = true; }

protected:
    /**
     * Template method implementation of isok().
     * Not be called if any of the following cases are true:
     *   okay == false
     *
     * Most implementations do not need to override this.
     * See setnotok().
     */
    virtual bool _isok() const
        { return true; }

    /**
     * Template method implementation of isfinished().
     * Not be called if any of the following cases are true:
     *   finished == true
     *
     * Most implementations do not need to override this.
     * See setfinished().
     */
    virtual bool _isfinished() const
        { return false; }

    /**
     * Template method implementation of geterror().
     * Not be called if any of the following cases are true:
     *
     * Most implementations do not need to override this.
     * See seterror().
     */
    virtual WvString _geterror() const
        { return WvString::null; }

    /**
     * Template method implementation of encode().
     * Not be called if any of the following cases are true:
     *   isok() == false
     *   isfinished() == true
     *   in.used() == 0 && flush == false
     *
     * All implementations MUST define this.
     */
    virtual bool _encode(WvBuffer &in, WvBuffer &out, bool flush) = 0;

    /**
     * Template method implementation of finish().
     * Not be called if any of the following cases are true:
     *   isok() == false
     *   isfinished() == true
     * The encoder is marked finished AFTER this function exits.
     *
     * Many implementations do not need to override this.
     */
    virtual bool _finish(WvBuffer &out)
        { return true; }
};


/**
 * An encoder that discards all of its input.
 */
class WvNullEncoder : public WvEncoder
{
protected:
    virtual bool _encode(WvBuffer &in, WvBuffer &out, bool flush);
};


/**
 * A very efficient passthrough encoder that just sends its input
 * to its output without actually copying any data.
 * Counts the number of bytes it has processed.
 */
class WvPassthroughEncoder : public WvEncoder
{
    size_t total;
    
public:
    WvPassthroughEncoder() : total(0) { }

    /**
     * Returns the number of bytes processed so far.
     */
    size_t bytes_processed() { return total; }
    
protected:
    virtual bool _encode(WvBuffer &in, WvBuffer &out, bool flush);
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
     * It is not a good idea to change the list this unless the encoders
     * have been flushed and finished first, but it is supported.
     */
    void append(WvEncoder *enc, bool auto_free);
    void prepend(WvEncoder *enc, bool auto_free);
    void unlink(WvEncoder *enc);
    void zap();

protected:
    /**
     * Returns true if and only if all encoders return true.
     *
     * WvEncoderChain is special in that it may transition from
     * isok() == false to isok() == true if the offending encoders
     * are removed from the list.
     */
    virtual bool _isok() const;
    
    /**
     * Returns false if and only if all encoders return false.
     *
     * WvEncoderChain is special in that it may transition from
     * isfinished() == true to isfinished() == false if the offending
     * encoders are removed from the list, but not if finish() is
     * called.
     */
    virtual bool _isfinished() const;

    /**
     * Returns the first non-null error message found in the list.
     *
     * WvEncoderChain is special in that it may transition from
     * !geterror() = false to !geterror() = true if the offending
     * encoders are removed from the list.
     */
    virtual WvString _geterror() const;
    
    /**
     * Passes the data through the entire chain of encoders.
     *
     * Returns true if and only if all encoders return true.
     */
    virtual bool _encode(WvBuffer &in, WvBuffer &out, bool flush);
    
    /**
     * Invokes finish() on the first encoder in the chain, then
     * flush() on the second encoder if new data was generated,
     * then finish() on the second encoder, and so on until all
     * encoders have been flushed and finished (assuming the first
     * encoder had already been flushed).
     *
     * Returns true if and only if all encoders return true.
     */
    virtual bool _finish(WvBuffer & out);
};

#endif // __WVENCODER_H
