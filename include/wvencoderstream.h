/*
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvEncoderStream chains a series of encoders on the input and
 * output ports of the underlying stream to effect on-the-fly data
 * transformations.
 *
 * Notice that the WvEncoderStream's auto_flush flag takes on significant
 * importance when working with encoders that treat explicit buffer
 * flushing in a special manner, such as the Gzip encoder.  For Gzip,
 * if this flag were true, each incremental write call would cause the
 * encoder to flush out small poorly compressed chunks.
 *
 * For such streams, disable the flag with auto_flush(false).
 */
#ifndef __WVENCODERSTREAM_H
#define __WVENCODERSTREAM_H

#include "wvstream.h"
#include "wvstreamclone.h"
#include "wvencoder.h"

class WvEncoderStream : public WvStreamClone
{
    bool is_closing;
    WvDynamicBuffer readinbuf;
    WvDynamicBuffer readoutbuf;
    WvDynamicBuffer writeinbuf;
    WvDynamicBuffer writeoutbuf;
public:
    // encoder chains for reading and writing respectively
    WvEncoderChain readchain;
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

    WvEncoderStream(WvStream *_cloned);
    virtual ~WvEncoderStream();

    /**
     * Safely shuts down the stream.
     *   flushes and finishes the read chain;
     *   then flushes and finishes the write chain;
     *   then flushes the stream output buffers;
     *   then closes the stream
     */
    virtual void close();

    /**
     * Flushes the read chain through to the stream's input buffer.
     * The regular flush() only operates on the write chain.
     * Returns true iff the encoder chain returned true.
     */
    bool flush_read();

    /**
     * Flushes the write chain through to the stream's output buffer.
     * The regular flush() invokes this, then attempts to
     * synchronously push the buffered data to the stream, which
     * may not always be desirable since it can be quite costly.
     *
     * To simply cause the write chain to be flushed but allow
     * WvStreams to drain the output buffer at its leisure, use
     * this function.
     * Returns true iff the encoder chain returned true.
     */
    bool flush_write();

    /**
     * Calls flush() then finish() on the read chain of encoders.
     * Returns true iff the encoder chain returned true.
     */
    bool finish_read();

    /**
     * Calls flush() then finish() on the write chain of encoders.
     * Does not synchronously flush() the output buffer.
     * Returns true iff the encoder chain returned true.
     */
    bool finish_write();

    /**
     * Refine isok() semantics for encoders.
     * Goes false on error or after all data has been read from
     * the internal buffers and readchain.isfinished().
     * Note: readchain is finished automatically when cloned
     *       stream encounters an error
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
