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
#include "wvstring.h"

/**
 * The base encoder class.
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
     * Returns true if the encoder can encode more data.
     *
     * This should only be used to record permanent failures.
     * Transient errors (eg. bad block, but recoverable) should be
     * detected in a different fashion.
     *
     * @return true if the encoder is ok
     */
    bool isok() const
        { return okay && _isok(); }

    /**
     * Returns true if the encoder can no longer process data.
     *
     * This will be set when the encoder detects and end-of-data
     * mark in its input, or when finish() is called.
     *
     * @return true if the encoder is finished
     */
    bool isfinished() const
        { return finished || _isfinished(); }

    /**
     * Returns an error message if any is available.
     *
     * @return the error message, or the null string is isok() == true
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
     * as possible (or as it convenient) from the input buffer and store
     * the results in the output buffer.  Partial results may be buffered
     * internally by the encoder to be written to the output buffer later
     * when the encoder is flushed.
     *
     * If finish = true, the encode() will be followed up by a call to
     * finish().  The return values will be ANDed together to yield the
     * final result.  Most useful when flush is also true.
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
     * @param inbuf the input buffer
     * @param outbuf the output buffer
     * @param flush if true, flushes the encoder
     * @param finish if true, calls finish() on success
     * @return true on success
     * @see _encode for the actual implementation
     */
    bool encode(WvBuffer &inbuf, WvBuffer &outbuf, bool flush = false,
        bool finish = false);

    /**
     * Flushes the encoder and optionally finishes it.
     *
     * @param inbuf the input buffer
     * @param outbuf the output buffer
     * @param finish if true, calls finish() on success
     * @return true on success
     */
    inline bool flush(WvBuffer &inbuf, WvBuffer &outbuf,
        bool finish = false)
        { return encode(inbuf, outbuf, true, finish); }

    /**
     * Tells the encoder that NO MORE DATA will ever be encoded.
     *
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
     * @param outbuf the output buffer
     * @return true on success
     * @see _finish for the actual implementation
     */
    bool finish(WvBuffer &outbuf);

    /**
     * Asks an encoder to reset itself to its initial state at
     * creation time, if supported.
     *
     * This function may be called at any time, even if
     * isok() == false, or isfinished() == true.
     *
     * Returns true if the encoder has successfully been reset.
     * If the behaviour is not supported or an error occurs,
     * then afterwards isok() == false.
     *
     * @return true on success
     * @see _reset for the actual implementation
     */
    bool reset();

    /*
     * Helper functions for encoding strings.
     * Some variants have no encode(...) equivalent because they must
     * always flush.
     */
     
    /**
     * Flushes data through the encoder from a string to a buffer.
     *
     * @param instr the input string
     * @param outbuf the output buffer
     * @param finish if true, calls finish() on success
     * @return true on success
     */
    bool flush(WvStringParm instr, WvBuffer &outbuf,
        bool finish = false);
        
    /**
     * Flushes data through the encoder from a string to a string.
     *
     * The output data is appended to the target string.
     *
     * @param instr the input string
     * @param outstr the output string
     * @param finish if true, calls finish() on success
     * @return true on success
     */
    bool flush(WvStringParm instr, WvString &outstr,
        bool finish = false);

    /**
     * Encodes data from a buffer to a string.
     *
     * The output data is appended to the target string.
     *
     * @param inbuf the input buffer
     * @param outstr the output string
     * @param flush if true, flushes the encoder
     * @param finish if true, calls finish() on success
     * @return true on success
     */   
    bool encode(WvBuffer &inbuf, WvString &outstr,
        bool flush = false, bool finish = false);

    /**
     * Flushes data through the encoder from a buffer to a string.
     *
     * The output data is appended to the target string.
     *
     * @param inbuf the input buffer
     * @param outstr the output string
     * @param finish if true, calls finish() on success
     * @return true on success
     */   
    inline bool flush(WvBuffer &inbuf, WvString &outstr,
        bool finish = false)
        { return encode(inbuf, outstr, true, finish); }
    
    /**
     * Flushes data through the encoder from a string to a string.
     *
     * @param inbuf the input buffer
     * @param finish if true, calls finish() on success
     * @return the resulting encoded string, does not signal errors
     */   
    WvString strflush(WvStringParm instr, bool finish = false);
    
    /**
     * Flushes data through the encoder from a buffer to a string.
     *
     * @param inbuf the input buffer
     * @param finish if true, calls finish() on success
     * @return the resulting encoded string, does not signal errors
     */   
    WvString strflush(WvBuffer &inbuf, bool finish = false);

    /*
     * Helper functions for encoding data from plain memory buffers.
     * Some variants have no encode(...) equivalent because they must
     * always flush.
     *
     * The 'mem' suffix has been tacked on to these functions to
     * resolve ambiguities dealing with 'char *' that should be
     * promoted to WvString.  For instance, consider the signatures
     * of strflushmem(const void*, size_t) and strflush(WvStringParm,
     * bool).
     */
     
    /**
     * Flushes data through the encoder from memory to a buffer.
     *
     * @param inmem the input data pointer
     * @param inlen the input data length
     * @param outbuf the output buffer
     * @param finish if true, calls finish() on success
     * @return true on success
     */
    bool flushmembuf(const void *inmem, size_t inlen, WvBuffer &outbuf,
        bool finish = false);
        
    /**
     * Flushes data through the encoder from memory to memory.
     *
     * The <code>outlen</code> parameter specifies by reference
     * the length of the output buffer.  It is updated in place to
     * reflect the number of bytes copied to the output buffer.
     * If the buffer was too small to hold the data, the overflow
     * bytes will be discarded and false will be returned.
     *
     * @param inmem the input data pointer
     * @param inlen the input data length
     * @param outmem the output data pointer
     * @param outlen the output data length, by reference
     * @param finish if true, calls finish() on success
     * @return true on success
     */
    bool flushmemmem(const void *inmem, size_t inlen, void *outmem,
        size_t *outlen, bool finish = false);
        
    /**
     * Encodes data from a buffer to memory.
     *
     * The <code>outlen</code> parameter specifies by reference
     * the length of the output buffer.  It is updated in place to
     * reflect the number of bytes copied to the output buffer.
     * If the buffer was too small to hold the data, the overflow
     * bytes will be discarded and false will be returned.
     *
     * @param inmem the input data pointer
     * @param inlen the input data length
     * @param outmem the output data pointer
     * @param outlen the output data length, by reference
     * @param flush if true, flushes the encoder
     * @param finish if true, calls finish() on success
     * @return true on success
     */
    bool encodebufmem(WvBuffer &inbuf, void *outmem, size_t *outlen,
        bool flush = false, bool finish = false);   
        
    /**
     * Flushes data through the encoder from a buffer to memory.
     *
     * The <code>outlen</code> parameter specifies by reference
     * the length of the output buffer.  It is updated in place to
     * reflect the number of bytes copied to the output buffer.
     * If the buffer was too small to hold the data, the overflow
     * bytes will be discarded and false will be returned.
     *
     * @param inbuf the input buffer
     * @param outmem the output data pointer
     * @param outlen the output data length, by reference
     * @param finish if true, calls finish() on success
     * @return true on success
     */
    inline bool flushbufmem(WvBuffer &inbuf, void *outmem, size_t *outlen,
        bool finish = false)
        { return encodebufmem(inbuf, outmem, outlen, true, finish); }

    /**
     * Flushes data through the encoder from a string to memory.
     *
     * The <code>outlen</code> parameter specifies by reference
     * the length of the output buffer.  It is updated in place to
     * reflect the number of bytes copied to the output buffer.
     * If the buffer was too small to hold the data, the overflow
     * bytes will be discarded and false will be returned.
     *
     * @param instr the input string
     * @param outmem the output data pointer
     * @param outlen the output data length, by reference
     * @param finish if true, calls finish() on success
     * @return true on success
     */
    bool flushstrmem(WvStringParm instr, void *outmem, size_t *outlen,
        bool finish = false);

    /**
     * Flushes data through the encoder from memory to a string.
     *
     * @param inmem the input data pointer
     * @param inlen the input data length
     * @param finish if true, calls finish() on success
     * @return the resulting encoded string, does not signal errors
     */
    WvString strflushmem(const void *inmem, size_t inlen, bool finish = false);

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

    /**
     * Sets an error condition, then setnotok().
     */
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
     * Not called if any of the following cases are true:
     *   okay == false
     *
     * Most implementations do not need to override this.
     * @see setnotok
     */
    virtual bool _isok() const
        { return true; }

    /**
     * Template method implementation of isfinished().
     * Not called if any of the following cases are true:
     *   finished == true
     *
     * Most implementations do not need to override this.
     * @see setfinished
     */
    virtual bool _isfinished() const
        { return false; }

    /**
     * Template method implementation of geterror().
     * Not called if any of the following cases are true:
     *   isok() == true
     *   errstr is not null
     *
     * Most implementations do not need to override this.
     * @see seterror
     */
    virtual WvString _geterror() const
        { return WvString::null; }

    /**
     * Template method implementation of encode().
     * Not called if any of the following cases are true:
     *   okay == false       // NOTE: not same as isok()
     *   finished == true    // NOTE: not same as isfinished()
     *   in.used() == 0 && flush == false
     *
     * All implementations MUST define this.
     *
     * If you also override _isok() or _isfinished(), note that they
     * will NOT be consulted when determining whether or not to
     * invoke this function.  This allows finer control over the
     * semantics of isok() and isfinished() with respect to encode().
     *
     * @see encode
     */
    virtual bool _encode(WvBuffer &in, WvBuffer &out, bool flush) = 0;

    /**
     * Template method implementation of finish().
     * Not called if any of the following cases are true:
     *   okay == false       // NOTE: not same as isok()
     *   finished == true    // NOTE: not same as isfinished()
     * The encoder is marked finished AFTER this function exits.
     *
     * Many implementations do not need to override this.
     *
     * If you also override _isok() or _isfinished(), note that they
     * will NOT be consulted when determining whether or not to
     * invoke this function.  This allows finer control over the
     * semantics of isok() and isfinished() with respect to finish().
     *
     * @see finish
     */
    virtual bool _finish(WvBuffer &out)
        { return true; }

    /**
     * Template method implementation of reset().
     *
     * When this method is invoked, the current local state will
     * be okay == true and finished == false.  If false is returned,
     * then okay will be set to false.
     *
     * Returns false on error or if the behaviour is not supported.
     * May also set a detailed error message if an error occurs.
     * @see reset
     */
    virtual bool _reset()
        { return false; }
};


/**
 * An encoder that discards all of its input.
 */
class WvNullEncoder : public WvEncoder
{
protected:
    virtual bool _encode(WvBuffer &in, WvBuffer &out, bool flush);
    virtual bool _reset(); // supported: does nothing
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
    WvPassthroughEncoder();
    virtual ~WvPassthroughEncoder() { }

    /**
     * Returns the number of bytes processed so far.
     */
    size_t bytes_processed() { return total; }
    
protected:
    virtual bool _encode(WvBuffer &in, WvBuffer &out, bool flush);
    virtual bool _reset(); // supported: resets the count to zero
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
        WvDynamicBuffer out;
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
     * Returns true iff all encoders return isok() == true.
     *
     * WvEncoderChain is special in that it may transition from
     * isok() == false to isok() == true if the offending encoders
     * are removed from the list.
     */
    virtual bool _isok() const;
    
    /**
     * Returns false iff all encoders return isfinished() == false.
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
     * Returns true iff all encoders return true.
     */
    virtual bool _encode(WvBuffer &in, WvBuffer &out, bool flush);
    
    /**
     * Invokes finish() on the first encoder in the chain, then
     * flush() on the second encoder if new data was generated,
     * then finish() on the second encoder, and so on until all
     * encoders have been flushed and finished (assuming the first
     * encoder had already been flushed).
     *
     * Returns true iff all encoders return true.
     */
    virtual bool _finish(WvBuffer & out);

    /**
     * Resets all of the encoders in the chain and discards any
     * pending buffered input.  Preserves the list of encoders.
     *
     * Returns true iff all encoders return true.
     */
    virtual bool _reset();
};

#endif // __WVENCODER_H
