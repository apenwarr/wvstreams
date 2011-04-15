#include "wvunixdgsocket.h"
#include <sys/stat.h>
#ifdef MACOS
#include <sys/types.h>
#endif

WvUnixDGSocket::WvUnixDGSocket(WvStringParm filename, bool _server, int perms)
    : socketfile(filename)
{
//    log(WvLog::Debug2, "Starting up %s!\n", filename);
    server = _server;
    backoff = 10;

    bufsize = 0;

    // open a datagram unix domain socket
    setfd(socket(PF_UNIX, SOCK_DGRAM, 0));

    // if we don't have a file desciptor, something is wrong.
    if (getfd() < 0)
    {
        seterr("No Socket available.");
        return;
    }

    // set non-blocking mode
    fcntl(getfd(), F_SETFL, O_RDWR|O_NONBLOCK);

    WvUnixAddr uaddr(socketfile);

    // Let this file be reusable, since we're going to own this anyway
    // The business with the int x is just Unix stupidities.. *sigh*
    int x = 1;
    setsockopt(getfd(), SOL_SOCKET, SO_REUSEADDR, &x, sizeof(x));

    if (server)
    {
        // Fix it so that there can't be another process on this file
        unlink(socketfile);

        // Actually bind to the address we set up above.
        sockaddr *addr = uaddr.sockaddr();
        if (bind(getfd(), (sockaddr *)addr, uaddr.sockaddr_len()))
        {
            seterr("Bind to %s failed: %s", socketfile, strerror(errno));
            close();
        }
        delete addr;

        chmod(socketfile, perms);
    }
    else
    {
        // we're the client, so we connect to someone else's socket
        sockaddr *addr = uaddr.sockaddr();
        if (connect(getfd(), (sockaddr *)addr, uaddr.sockaddr_len()))
        {
            seterr("Connect to %s failed: %s",
                   socketfile, strerror(errno));
            close();
        }
        delete addr;
    }

    drain();
}

WvUnixDGSocket::~WvUnixDGSocket()
{
//    log(WvLog::Debug2, "Destroying: %s\n", socketfile);
    close();
    if (server)
        unlink(socketfile);
}

size_t WvUnixDGSocket::uwrite(const void *buf, size_t count)
{
    size_t ret = bufs.isempty() ? WvFDStream::uwrite(buf, count) : 0;

    if (ret < count)
    {
        WvDynBuf *b = new WvDynBuf;
        b->put(buf, count);
        bufs.append(b, true);
	bufsize += count;
    }

    return count;
}

void WvUnixDGSocket::pre_select(SelectInfo &si)
{
    SelectRequest oldwant = si.wants;
    if (!bufs.isempty())
    {
        // stupid unix domain sockets seem to return true when selecting
        // for write EVEN IF write() RETURNS -EAGAIN!  Just shoot me.
        // 
        // To deal with this, we set an alarm() in post_select() if we
        // couldn't write everything we wanted.  While the alarm is set,
        // we don't try to flush our output buffer.
        if (alarm_remaining() <= 0)
            si.wants.writable = true;
        else if (si.msec_timeout < 0
                 || si.msec_timeout > alarm_remaining())
            si.msec_timeout = alarm_remaining();
    }

    WvFDStream::pre_select(si);

    si.wants = oldwant;
}

bool WvUnixDGSocket::post_select(SelectInfo &si)
{
    SelectRequest oldwant = si.wants;
    if (!bufs.isempty())
        si.wants.writable = true;

    bool sure = WvFDStream::post_select(si);

    si.wants = oldwant;

    if (sure)
    {
        // try flushing previous bufs
        WvBufList::Iter i(bufs);
        for (i.rewind(); i.next(); )
        {
            int used = i->used();
            int retval = WvFDStream::uwrite(i->get(used), used);
            if (retval < used)
            {
                i->unget(used);
                alarm(backoff *= 2);
                if (backoff > 1000)
                    backoff = 1000;
                break; // can't continue
            }
            else
            {
		bufsize -= used;
                i.xunlink(); // done with that one
                backoff = 10;
            }
        }
    }

    return sure;
}


