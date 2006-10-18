/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * WvStreamClone simply forwards all requests to the "cloned" stream.
 * 
 * NOTE: this file is a pain to maintain, because many of these functions
 * are almost (but not quite) exactly like the ones in WvStream.  If
 * WvStream changes, you need to change this too.
 * 
 * See wvstreamclone.h.
 */
#include "wvstreamclone.h"
#include "wvmoniker.h"

#ifdef _MSC_VER
#pragma warning(disable : 4073)
#pragma init_seg(lib)
#endif

static IWvStream *creator(WvStringParm s, IObject *obj, void *)
{
    if (!obj)
	obj = wvcreate<IWvStream>(s);
    return new WvStreamClone(mutate<IWvStream>(obj));
}

static WvMoniker<IWvStream> reg("clone", creator);


WvStreamClone::WvStreamClone(IWvStream *_cloned) 
    : cloned(0), disassociate_on_close(false),
      my_type("WvStreamClone:(none)")
{
    setclone(_cloned);
    // the sub-stream will force its own values, if it really wants.
    force_select(false, false, false);
}


WvStreamClone::~WvStreamClone()
{
    //fprintf(stderr, "%p destroying: clone is %p\n", this, cloned);
    close();
    WVRELEASE(cloned);
}


void WvStreamClone::noread()
{
    // unlike nowrite(), it is safe to call cloned->noread() immediately.
    // That will pass the shutdown(SHUT_RD) on to the deepest stream right
    // away, but won't close anything until all the inbufs are empty.
    if (cloned)
	cloned->noread();
    WvStream::noread();
}


void WvStreamClone::nowrite()
{
    // this sets stop_write.  We call cloned->nowrite() in flush_internal()
    // when our outbuf is flushed (because until then, we *do* want to be
    // able to write to the clone).
    if (cloned && !outbuf.used())
	cloned->nowrite();
    WvStream::nowrite();
}


void WvStreamClone::close()
{
    // fprintf(stderr, "%p closing substream %p\n", this, cloned);
    if (cloned)
	cloned->setclosecallback(0); // prevent recursion!
    WvStream::close();
    if (disassociate_on_close)
        setclone(NULL);
    if (cloned)
	cloned->close();
}


bool WvStreamClone::flush_internal(time_t msec_timeout)
{
    if (cloned)
    {
	if (stop_write && !outbuf.used())
	    cloned->nowrite();
        return cloned->flush(msec_timeout);
    }
    else
	return true;
}


size_t WvStreamClone::uread(void *buf, size_t size)
{
    // we use cloned->read() here, not uread(), since we want the _clone_
    // to own the input buffer, not the main stream.
    if (cloned)
    {
	size_t len = 0;
	if (cloned->isok())
	    len = cloned->read(buf, size);
	if (len == 0 && !cloned->isok())
	    close();
	return len;
    }
    else
	return 0;
}


size_t WvStreamClone::uwrite(const void *buf, size_t size)
{
    // we use cloned->write() here, not uwrite(), since we want the _clone_
    // to own the output buffer, not the main stream.
    if (cloned)
	return cloned->write(buf, size);
    else
	return 0;
}


bool WvStreamClone::isok() const
{
    if (geterr())
	return false;
    if (!cloned)
	return false;
    return WvStream::isok();
    
    // don't do this: cloned's closecallback will close us when needed.
    // return cloned->isok();
}


int WvStreamClone::geterr() const
{
    if (WvStream::geterr())
	return WvStream::geterr();
    if (cloned)
	return cloned->geterr();
    return EIO;
}


WvString WvStreamClone::errstr() const
{
    if (WvStream::geterr())
	return WvStream::errstr();
    if (cloned)
	return cloned->errstr();
    return "No child stream!";
}


void WvStreamClone::close_callback(WvStream &s)
{
    if (cloned == &s)
    {
	//fprintf(stderr, "streamclone-closecb: %d/%d/%d/%d/%d\n",
	//	stop_read, stop_write, outbuf.used(), inbuf.used(), closed);
	nowrite();
	noread();
	// close();
	//fprintf(stderr, "streamclone-closecb2: %d/%d/%d/%d/%d\n",
	//	stop_read, stop_write, outbuf.used(), inbuf.used(), closed);
    }
}


void WvStreamClone::setclone(IWvStream *newclone)
{
    if (cloned)
	cloned->setclosecallback(0);
    cloned = newclone;
    closed = stop_read = stop_write = false;
    if (cloned)
	cloned->setclosecallback(IWvStreamCallback(this, &WvStreamClone::close_callback));
    
    if (newclone != NULL)
        my_type = WvString("WvStreamClone:%s", newclone->wstype());
    else
        my_type = "WvStreamClone:(none)";
}


bool WvStreamClone::pre_select(SelectInfo &si)
{
    SelectRequest oldwant;
    bool result = WvStream::pre_select(si);
    if (cloned && cloned->isok())
    {
	oldwant = si.wants;
	
	if (!si.inherit_request)
	{
	    si.wants.readable |= readcb;
	    si.wants.writable |= writecb;
	    si.wants.isexception |= exceptcb;
	    // si.wants |= cloned->force; // why would this be necessary?
	}
	
	if (outbuf.used() || autoclose_time)
	    si.wants.writable = true;

	result = result || cloned->pre_select(si);
	
	si.wants = oldwant;
    }

    return result;
}


bool WvStreamClone::post_select(SelectInfo &si)
{
    SelectRequest oldwant;
    // This currently always returns false, but we prolly should
    // still have it here in case it ever becomes useful
    bool result = WvStream::post_select(si);
    bool val, want_write;
    
    if (cloned && cloned->should_flush())
	flush(0);

    if (cloned && cloned->isok())
    {
	oldwant = si.wants;
	if (!si.inherit_request)
	{
	    si.wants.readable |= readcb;
	    si.wants.writable |= writecb;
	    si.wants.isexception |= exceptcb;
	    // si.wants |= cloned->force; // why would this be needed?
	}

	val = cloned->post_select(si);
	want_write = si.wants.writable;
	si.wants = oldwant;
	
	// return result if they're looking for writable and we still
	// have data in outbuf - the writable is for flushing, not for you!
	if (want_write && outbuf.used())
	    return result;
	else if (val && si.wants.readable && read_requires_writable
		 && !read_requires_writable->select(0, false, true))
	    return result;
	else if (val && si.wants.writable && write_requires_readable
		 && !write_requires_readable->select(0, true, false))
	    return result;
	else
	    return val || result;
    }
    
    return result;
}


const WvAddr *WvStreamClone::src() const
{
    if (cloned)
	return cloned->src();
    return NULL;
}


void WvStreamClone::execute()
{
    WvStream::execute();
    if (cloned) cloned->callback();
}
