/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 */

/** \file
 * An stream wrapper for encoders.
 */
#ifndef __WVENCODERSTREAM_H
#define __WVENCODERSTREAM_H

#include "wvstream.h"
#include "wvstreamclone.h"
#include "wvencoder.h"

/**
 * WvEncoderStream chains a series of encoders on the input and
 * output ports of the underlying stream to effect on-the-fly data
 * transformations.
 * <p>
 * Notice that the value of WvEncoderStream's auto_flush flag becomes
 * important when working with encoders that treat explicit buffer
 * flushing in a special manner.  For instance, on flush() the Gzip
 * encoder synchronizes its output.  Were auto_flush to remain true,
 * each incremental write to the stream would cause the Gzip encoder
 * to flush its dictionary thereby resulting in poor compression.
 * </p>
 * @see WvStream::auto_flush(bool)
 */
class WvEncoderStream : public WvStreamClone
{
    bool is_closing;
    bool is_eof;
    WvDynamicBuffer readinbuf;
    WvDynamicBuffer readoutbuf;
    WvDynamicBuffer writeinbuf;
    WvDynamicBuffer writeoutbuf;

public:
    /**
     * Encoder chain through which input data is passed.
     */
    WvEncoderChain readchain;

    /**
     * Encoder chain through which output data is passed.
     */
    WvEncoderChain writechain;

    /**
     * Controls the minimum number of unencoded bytes the encoder
     * should try to read at once from the underlying stream,
     * to optimize performance of block-oriented protocols.
     * This is not the same as queuemin() which guarantees how much
     * encoded input must be generated before select() returns true.
     * if 0, the encoder will only whatever is specified in uread()
     */
    size_t min_readsize;

    /**
     * Creates an encoder stream.
     *
     * @param cloned the stream to wrap
     */
    WvEncoderStream(WvStream *cloned);
    virtual ~WvEncoderStream();

    /**
     * Safely shuts down the stream.
     * <p>
     * Does the following in sequence:
     * <ul>
     * <li>Flushes and finishes the read chain</li>
     * <li>Flushes and finishes the write chain</li>
     * <li>Flushes the stream output buffers</li>
     * <li>Closes the underlying stream</li>
     * </ul>
     * </p>
     */
    virtual void close();

    /**
     * Flushes the read chain through to the stream's input buffer.
     * <p>
     * The regular stream flush() only operates on the write chain.
     * </p>
     * @return true if the encoder chain returned true
     */
    bool flush_read();

    /**
     * Flushes the write chain through to the stream's output buffer.
     * <p>
     * The regular stream flush() invokes this, then attempts to
     * synchronously push the buffered data to the stream, which
     * may not always be desirable since it can be quite costly.
     * </p><p>
     * To simply cause the write chain to be flushed but allow
     * WvStreams to drain the encoded output buffer at its leisure,
     * use this function.
     * </p>
     * @return true if the encoder chain returned true
     */
    bool flush_write();

    /**
     * Calls flush() then finish() on the read chain of encoders.
     *
     * @return true if the encoder chain returned true
     */
    bool finish_read();

    /**
     * Calls flush() then finish() on the write chain of encoders.
     * <p>
     * Does not flush() the stream.
     * </p>
     * @return true if the encoder chain returned true.
     */
    bool finish_write();

    /**
     * Defines isok() semantics for encoders.
     * <p>
     * Returns false on error or after all data has been read from
     * the internal buffers and readchain.isfinished() or
     * ! writechain.isok().
     * </p>
     * @return true if it is still possible to read and write data
     */
    virtual bool isok() const;

protected:
    bool pre_select(SelectInfo &si);
    virtual size_t uread(void *buf, size_t size);
    virtual size_t uwrite(const void *buf, size_t size);
    virtual void flush_internal(time_t msec_timeout);

private:
    void checkreadisok();
    void checkwriteisok();
    
    // pulls a chunk of specified size from the underlying stream
    void pull(size_t size);

    // pushes a chunk to the underlying stream
    bool push(bool flush, bool finish);
};

#endif // __WVENCODERSTREAM_H
