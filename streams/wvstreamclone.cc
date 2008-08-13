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

static IWvStream *creator(WvStringParm s, IObject *_obj)
{
    return new WvStreamClone(wvcreate<IWvStream>(s, _obj));
}

static IWvStream *objcreator(WvStringParm s, IObject *_obj)
{
    // no real need to wrap it
#if MUTATE_ISNT_BROKEN
    return mutate<IWvStream>(_obj);
#else
    // HACK: we assume the object is safely of type IWvStream because
    // xplc's mutate<> function seems not to be working for some reason.
    return (IWvStream *)_obj;
#endif
}

static WvMoniker<IWvStream> clonereg("clone", creator);
static WvMoniker<IWvStream> objreg("obj", objcreator);
static WvMoniker<IWvStream> objreg2("", objcreator);


WvStreamClone::WvStreamClone(IWvStream *_cloned) 
    : cloned(NULL),
      my_type("WvStreamClone:(none)")
{
    setclone(_cloned);
    // the sub-stream will force its own values, if it really wants.
    force_select(false, false, false);
}


WvStreamClone::~WvStreamClone()
{
    //fprintf(stderr, "%p destroying: clone is %p\n", this, cloned);
    setclone(NULL);
    close();
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


void WvStreamClone::close_callback()
{
    //fprintf(stderr, "streamclone-closecb: %d/%d/%d/%d/%d\n",
    //	stop_read, stop_write, outbuf.used(), inbuf.used(), closed);
    nowrite();
    noread();
    // close();
    //fprintf(stderr, "streamclone-closecb2: %d/%d/%d/%d/%d\n",
    //	stop_read, stop_write, outbuf.used(), inbuf.used(), closed);
}


void WvStreamClone::setclone(IWvStream *newclone)
{
    if (cloned)
	cloned->setclosecallback(0);
    WVRELEASE(cloned);
    cloned = newclone;
    closed = stop_read = stop_write = false;
    if (cloned)
	cloned->setclosecallback(wv::bind(&WvStreamClone::close_callback,
					  this));
    
    if (newclone != NULL)
        my_type = WvString("WvStreamClone:%s", newclone->wstype());
    else
        my_type = "WvStreamClone:(none)";
}


void WvStreamClone::pre_select(SelectInfo &si)
{
    SelectRequest oldwant = si.wants;
    WvStream::pre_select(si);

    if (cloned && cloned->isok())
    {
	if (!si.inherit_request)
	{
	    si.wants.readable |= static_cast<bool>(readcb);
	    si.wants.writable |= static_cast<bool>(writecb);
	    si.wants.isexception |= static_cast<bool>(exceptcb);
	}
	
	if (outbuf.used() || autoclose_time)
	    si.wants.writable = true;

	cloned->pre_select(si);
	si.wants = oldwant;
    }
}


bool WvStreamClone::post_select(SelectInfo &si)
{
    SelectRequest oldwant = si.wants;
    // This currently always returns false, but we prolly should
    // still have it here in case it ever becomes useful
    bool result = WvStream::post_select(si);
    bool val, want_write;
    
    if (cloned && cloned->should_flush())
	flush(0);

    if (cloned && cloned->isok())
    {
	if (!si.inherit_request)
	{
	    si.wants.readable |= static_cast<bool>(readcb);
	    si.wants.writable |= static_cast<bool>(writecb);
	    si.wants.isexception |= static_cast<bool>(exceptcb);
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

WvString WvStreamClone::getattr(WvStringParm name) const
{
    WvString ret = WvStream::getattr(name);
    if (ret.isnull() && cloned)
	return cloned->getattr(name);

    return ret;
}
