/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * A top-level data encoder class and a few useful encoders.
 */
#ifndef __WVENCODER_H
#define __WVENCODER_H

#include "wvbuffer.h"
#include "wvlinklist.h"
#include "wvstring.h"

/**
 * The base encoder class.
 * <p>
 * Encoders read data from an input buffer, transform it in some
 * way, then write the results to an output buffer.  The resulting
 * data may be of a different size or data type, and may or may
 * not depend on previous data.
 * </p><p>
 * Encoders may or may not possess the following characteristics:
 * <ul>
 * <li>Statefulness: encoding of successive input elements may
 *     depend on previous one</li>
 * <li>Error states: encoding may enter an error state indicated
 *     by <code>isok() == false</code> due to problems detected
 *     in the input, or by the manner in which the encoder has
 *     been user</li>
 * <li>Minimum input block size: data will not be drawn from the
 *     input buffer until enough is available or the encoder
 *     is flushed</li>
 * <li>Minimum output block size: data will not be written to the
 *     output buffer until enough free space is available</li>
 * <li>Synchronization boundaries: data is process or generated
 *     in chunks which can be manipulated independently of any
 *     others, in which case flush() may cause the encoder to
 *     produce such a boundary in its output</li>
 * <li>Recognition of end-of-data mark: a special sequence marks
 *     the end of input, after which the encoder transitions to
 *     <code>isfinished() == true</code></li>
 * <li>Generation of end-of-data mark: a special sequence marks
 *     the end of output when the encoder transitions to
 *     <code>isfinished() == true</code>, usually by an explicit
 *     call to finish()</li>
 * <li>Reset support: the encoder may be reset to its initial
 *     state and thereby recycled at minimum cost</li>
 * </ul>
 * </p><p>
 *
 * Helper functions are provided for encoding data from plain
 * memory buffers and from strings.  Some have no encode(...)
 * equivalent because they cannot incrementally encode from
 * the input, hence they always use the flush option.
 *
 * The 'mem' suffix has been tacked on to these functions to
 * resolve ambiguities dealing with 'char *' that should be
 * promoted to WvString.  For instance, consider the signatures
 * of strflushmem(const void*, size_t) and strflushstr(WvStringParm,
 * bool).
 *
 * Another reason for these suffixes is to simplify overloading
 * the basic methods in subclasses since C++ would require the
 * subclass to redeclare all of the other signatures for
 * an overloaded method.
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
     * Returns true if the encoder has not encountered an error.
     * <p>
     * This should only be used to record permanent failures.
     * Transient errors (eg. bad block, but recoverable) should be
     * detected in a different fashion.
     * </p>
     * @return true if the encoder is ok
     */
    bool isok() const
        { return okay && _isok(); }

    /**
     * Returns true if the encoder can no longer encode data.
     * <p>
     * This will be set when the encoder detects and end-of-data
     * mark in its input, or when finish() is called.
     * </p>
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
     * <p>
     * If flush == true, the input buffer will be drained and the output
     * buffer will contain all of the encoded data including any that
     * might have been buffered internally from previous calls.  Thus it
     * is possible that new data will be written to the output buffer even
     * though the input buffer was empty when encode() was called.  If the
     * buffer could not be fully drained because there was insufficient
     * data, this function returns false and leaves the remaining unflushed
     * data in the buffer.
     * </p><p>
     * If flush == false, the encoder will read and encode as much data
     * as possible (or as it convenient) from the input buffer and store
     * the results in the output buffer.  Partial results may be buffered
     * internally by the encoder to be written to the output buffer later
     * when the encoder is flushed.
     * </p><p>
     * If finish = true, the encode() will be followed up by a call to
     * finish().  The return values will be ANDed together to yield the
     * final result.  Most useful when flush is also true.
     *
     * If a permanent error occurs, then isok() will return false, this
     * function will return false and the input buffer will be left in an
     * undefined state.
     * </p><p>
     * If a recoverable error occurs, the encoder should discard the
     * problematic data from the input buffer and return false from this
     * function, but isok() will remain true.
     * </p><p>
     * A stream might become isfinished() == true if an encoder-
     * specific end-of-data marker was detected in the input.
     * </p>
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
     * <p>
     * The encoder will flush out any internally buffered data
     * and write out whatever end-of-data marking it needs to the
     * supplied output buffer before returning.
     * </p><p>
     * Clients should invoke flush() on the input buffer before
     * finish() if the input buffer was not yet empty.
     * </p><p>
     * It is safe to call this function multiple times.
     * The implementation will simply return isok() and do nothing else.
     * </p>
     * @param outbuf the output buffer
     * @return true on success
     * @see _finish for the actual implementation
     */
    bool finish(WvBuffer &outbuf);

    /**
     * Asks an encoder to reset itself to its initial state at
     * creation time, if supported.
     * <p>
     * This function may be called at any time, even if
     * isok() == false, or isfinished() == true.
     * </p><p>
     * If the behaviour is not supported or an error occurs,
     * then false is returned and afterwards isok() == false.
     * </p>
     * @return true on success
     * @see _reset for the actual implementation
     */
    bool reset();

    /**
     * Flushes data through the encoder from a string to a buffer.
     *
     * @param instr the input string
     * @param outbuf the output buffer
     * @param finish if true, calls finish() on success
     * @return true on success
     */
    bool flushstrbuf(WvStringParm instr, WvBuffer &outbuf,
        bool finish = false);
        
    /**
     * Flushes data through the encoder from a string to a string.
     * <p>
     * The output data is appended to the target string.
     * </p>
     * @param instr the input string
     * @param outstr the output string
     * @param finish if true, calls finish() on success
     * @return true on success
     */
    bool flushstrstr(WvStringParm instr, WvString &outstr,
        bool finish = false);

    /**
     * Encodes data from a buffer to a string.
     * <p>
     * The output data is appended to the target string.
     * </p>
     * @param inbuf the input buffer
     * @param outstr the output string
     * @param flush if true, flushes the encoder
     * @param finish if true, calls finish() on success
     * @return true on success
     */   
    bool encodebufstr(WvBuffer &inbuf, WvString &outstr,
        bool flush = false, bool finish = false);

    /**
     * Flushes data through the encoder from a buffer to a string.
     * <p>
     * The output data is appended to the target string.
     * </p>
     * @param inbuf the input buffer
     * @param outstr the output string
     * @param finish if true, calls finish() on success
     * @return true on success
     */   
    inline bool flushbufstr(WvBuffer &inbuf, WvString &outstr,
        bool finish = false)
        { return encodebufstr(inbuf, outstr, true, finish); }
    
    /**
     * Flushes data through the encoder from a string to a string.
     *
     * @param inbuf the input buffer
     * @param finish if true, calls finish() on success
     * @return the resulting encoded string, does not signal errors
     */   
    WvString strflushstr(WvStringParm instr, bool finish = false);
    
    /**
     * Flushes data through the encoder from a buffer to a string.
     *
     * @param inbuf the input buffer
     * @param finish if true, calls finish() on success
     * @return the resulting encoded string, does not signal errors
     */   
    WvString strflushbuf(WvBuffer &inbuf, bool finish = false);

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
     * <p>
     * The <code>outlen</code> parameter specifies by reference
     * the length of the output buffer.  It is updated in place to
     * reflect the number of bytes copied to the output buffer.
     * If the buffer was too small to hold the data, the overflow
     * bytes will be discarded and false will be returned.
     * </p>
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
     * <p>
     * The <code>outlen</code> parameter specifies by reference
     * the length of the output buffer.  It is updated in place to
     * reflect the number of bytes copied to the output buffer.
     * If the buffer was too small to hold the data, the overflow
     * bytes will be discarded and false will be returned.
     * </p>
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
     * <p>
     * The <code>outlen</code> parameter specifies by reference
     * the length of the output buffer.  It is updated in place to
     * reflect the number of bytes copied to the output buffer.
     * If the buffer was too small to hold the data, the overflow
     * bytes will be discarded and false will be returned.
     * </p>
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
     * <p>
     * The <code>outlen</code> parameter specifies by reference
     * the length of the output buffer.  It is updated in place to
     * reflect the number of bytes copied to the output buffer.
     * If the buffer was too small to hold the data, the overflow
     * bytes will be discarded and false will be returned.
     * </p>
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
     * <p>
     * Not called if any of the following cases are true:
     * <ul>
     * <li>okay == false</li>
     * </ul>
     * </p><p>
     * Most implementations do not need to override this.
     * </p>
     * @return true if the encoder is ok
     * @see setnotok
     */
    virtual bool _isok() const
        { return true; }

    /**
     * Template method implementation of isfinished().
     * <p>
     * Not called if any of the following cases are true:
     * <ul>
     * <li>finished == true</li>
     * </ul>
     * </p><p>
     * Most implementations do not need to override this.
     * </p>
     * @return true if the encoder is finished
     * @see setfinished
     */
    virtual bool _isfinished() const
        { return false; }

    /**
     * Template method implementation of geterror().
     * <p>
     * Not called if any of the following cases are true:
     * <ul>
     * <li>isok() == true</li>
     * <li>errstr is not null</li>
     * </ul>
     * </p><p>
     * Most implementations do not need to override this.
     * </p>
     * @return the error message, or the null string if _isok() == true
     * @see seterror
     */
    virtual WvString _geterror() const
        { return WvString::null; }

    /**
     * Template method implementation of encode().
     * <p>
     * Not called if any of the following cases are true:
     * <ul>
     * <li>okay == false</li>
     * <li>finished == true</li>
     * <li>in.used() == 0 && flush == false</li>
     * </ul>
     * </p><p>
     * All implementations MUST define this.
     * </p><p>
     * If you also override _isok() or _isfinished(), note that they
     * will NOT be consulted when determining whether or not to
     * invoke this function.  This allows finer control over the
     * semantics of isok() and isfinished() with respect to encode().
     * </p>
     * @param inbuf the input buffer
     * @param outbuf the output buffer
     * @param flush if true, flushes the encoder
     * @return true on success
     * @see encode
     */
    virtual bool _encode(WvBuffer &inbuf, WvBuffer &outbuf,
        bool flush) = 0;

    /**
     * Template method implementation of finish().
     * <p>
     * Not called if any of the following cases are true:
     * <ul>
     * <li>okay == false</li>
     * <li>finished == true</li>
     * </ul>
     * </p><p>
     * The encoder is marked finished AFTER this function exits.
     * </p><p>
     * Many implementations do not need to override this.
     * </p><p>
     * If you also override _isok() or _isfinished(), note that they
     * will NOT be consulted when determining whether or not to
     * invoke this function.  This allows finer control over the
     * semantics of isok() and isfinished() with respect to finish().
     * </p>
     * @param outbuf the output buffer
     * @return true on success
     * @see finish
     */
    virtual bool _finish(WvBuffer &outbuf)
        { return true; }

    /**
     * Template method implementation of reset().
     * <p>
     * When this method is invoked, the current local state will
     * be okay == true and finished == false.  If false is returned,
     * then okay will be set to false.
     * </p><p>
     * May set a detailed error message if an error occurs.
     * </p>
     * @return true on success, false on error or if not supported
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
 * A very efficient passthrough encoder that just merges the
 * input buffer into the output buffer.
 * <p>
 * Counts the number of bytes it has processed.
 * </p><p>
 * Supports reset().
 * </p>
 */
class WvPassthroughEncoder : public WvEncoder
{
    size_t total;
    
public:
    WvPassthroughEncoder();
    virtual ~WvPassthroughEncoder() { }

    /**
     * Returns the number of bytes processed so far.
     * @return the number of bytes
     */
    size_t bytes_processed() { return total; }
    
protected:
    virtual bool _encode(WvBuffer &in, WvBuffer &out, bool flush);
    virtual bool _reset(); // supported: resets the count to zero
};


/**
 * An encoder chain owns a list of encoders that are used in sequence
 * to transform data from a source buffer to a target buffer.
 * <p>
 * Supports reset() if all the encoders it contains also support
 * reset().
 * </p>
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

    /**
     * Destroys the encoder chain.
     * <p>
     * Destroys any encoders that were added with auto_free == true.
     * </p>
     */
    virtual ~WvEncoderChain();

    /**
     * Appends an encoder to the tail of the chain.
     *
     * @param enc the encoder
     * @param auto_free if true, takes ownership of the encoder
     */
    void append(WvEncoder *enc, bool auto_free);

    /**
     * Prepends an encoder to the head of the chain.
     *
     * @param enc the encoder
     * @param auto_free if true, takes ownership of the encoder
     */
    void prepend(WvEncoder *enc, bool auto_free);

    /**
     * Unlinks the encoder from the chain.
     * <p>
     * Destroys the encoder if it was added with auto_free == true.
     * </p>
     * @param enc the encoder
     */
    void unlink(WvEncoder *enc);

    /**
     * Clears the encoder chain.
     * <p>
     * Destroys any encoders that were added with auto_free == true.
     * </p>
     */
    void zap();

protected:
    /**
     * Returns true if the encoder has not encountered an error.
     * <p>
     * WvEncoderChain is special in that it may transition from
     * isok() == false to isok() == true if the offending encoders
     * are removed from the list.
     * </p>
     * @return true iff all encoders return isok() == true
     * @see WvEncoder::_isok
     */
    virtual bool _isok() const;
    
    /**
     * Returns true if the encoder can no longer encode data.
     * <p>
     * WvEncoderChain is special in that it may transition from
     * isfinished() == true to isfinished() == false if the offending
     * encoders are removed from the list, but not if finish() is
     * called.
     * </p>
     * @return false iff all encoders return isfinished() == false
     */
    virtual bool _isfinished() const;

    /**
     * Returns the error message, if any.
     * <p>
     * WvEncoderChain is special in that it may transition from
     * !geterror() = false to !geterror() = true if the offending
     * encoders are removed from the list.
     * </p>
     * @return the first non-null error message in the chain
     */
    virtual WvString _geterror() const;
    
    /**
     * Passes the data through the entire chain of encoders.
     *
     * @return true iff all encoders return true.
     */
    virtual bool _encode(WvBuffer &in, WvBuffer &out, bool flush);
    
    /**
     * Finishes the chain of encoders.
     * <p>
     * Invokes finish() on the first encoder in the chain, then
     * flush() on the second encoder if new data was generated,
     * then finish() on the second encoder, and so on until all
     * encoders have been flushed and finished (assuming the first
     * encoder had already been flushed).
     * <p>
     * @return true iff all encoders return true.
     */
    virtual bool _finish(WvBuffer & out);

    /**
     * Resets the chain of encoders.
     * <p>
     * Resets all of the encoders in the chain and discards any
     * pending buffered input.  Preserves the list of encoders.
     * </p>
     * @return true iff all encoders return true.
     */
    virtual bool _reset();
};

#endif // __WVENCODER_H
