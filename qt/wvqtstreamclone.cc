/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Wraps another WvStream and attaches it to the normal Qt
 * event loop.  If you are using this object to manage all of your
 * streams, then you do not need to have a normal WvStreams
 * select()/callback() loop in your application at all.
 *
 * However, should you leave the Qt event loop and wish to continue
 * using this WvStream, call qt_detach() first, then run a normal
 * WvStreams event loop.  If you do not do this, events may be
 * lost!!  You may resume the Qt event loop at any time after the
 * WvStreams event loop has exited by calling qt_attach().
 *
 * Note: You do not need to add all of the WvStreams used in a Qt
 *       application to a single WvStreamList wrapped by a
 *       WvQtStreamClone so long as each top-level stream is wrapped
 *       by a WvQtStreamClone to take care of calling select()
 *       and callback() from within the Qt event loop.
 */
#include "wvqtstreamclone.moc"

// number of slots used by the separate chaining hashtable
// note: can store more than this number of elements in the table
#define NUM_SLOTS 41 // must be prime

WvQtStreamClone::WvQtStreamClone(WvStream* _cloned, int msec_timeout) :
    WvStreamClone(_cloned), msec_timeout(msec_timeout),
    pending_callback(false), first_time(true), select_in_progress(false),
    last_max_fd(-1),
    notify_readable(NUM_SLOTS),
    notify_writable(NUM_SLOTS),
    notify_exception(NUM_SLOTS)
{
    notify_readable.setAutoDelete(true);
    notify_writable.setAutoDelete(true);
    notify_exception.setAutoDelete(true);
    qt_attach();
}


WvQtStreamClone::~WvQtStreamClone()
{
}


void WvQtStreamClone::pre_poll()
{
    // prepare lists of file descriptors
    bool sure = _build_selectinfo(si, msec_timeout, 
				  false, false, false, true);
    if (sure)
    {
        pending_callback = true;
        si.msec_timeout = 0;
    }

    // set up a timer to wake us up to poll again (for alarms)
    // we don't try to catch the timer signal; we use it only to force
    // Qt's event loop to restart so our hook gets called again
    select_timer.stop();
    if (si.msec_timeout >= 0)
        select_timer.start(si.msec_timeout, true /*singleshot*/);

    // set up necessary QSocketNotifiers, unfortunately there is no
    // better way to iterate over the set of file descriptors
    for (int fd = 0; fd <= si.max_fd; ++fd)
    {
        if (FD_ISSET(fd, &si.read))
        {
            QSocketNotifier *n = notify_readable.find(fd);
            if (! n)
            {
                n = new QSocketNotifier(fd, QSocketNotifier::Read);
                notify_readable.insert(fd, n);
                QObject::connect(n, SIGNAL(activated(int)),
                    this, SLOT(fd_readable(int)));
            }
        } else
            notify_readable.remove(fd);
        
        if (FD_ISSET(fd, &si.write))
        {
            QSocketNotifier *n = notify_writable.find(fd);
            if (! n)
            {
                n = new QSocketNotifier(fd, QSocketNotifier::Write);
                notify_writable.insert(fd, n);
                QObject::connect(n, SIGNAL(activated(int)),
                    this, SLOT(fd_writable(int)));
            }
        } else
            notify_writable.remove(fd);
        
        if (FD_ISSET(fd, &si.except))
        {
            QSocketNotifier *n = notify_exception.find(fd);
            if (! n)
            {
                n = new QSocketNotifier(fd, QSocketNotifier::Exception);
                notify_exception.insert(fd, n);
                QObject::connect(n, SIGNAL(activated(int)),
                    this, SLOT(fd_exception(int)));
            }
        } else
            notify_exception.remove(fd);
    }

    // remove stale notifiers
    for (int fd = si.max_fd + 1; fd <= last_max_fd; ++fd)
    {
        notify_readable.remove(fd);
        notify_writable.remove(fd);
        notify_exception.remove(fd);
    }
    last_max_fd = si.max_fd;

    // clear select lists
    FD_ZERO(&si.read);
    FD_ZERO(&si.write);
    FD_ZERO(&si.except);
}


void WvQtStreamClone::post_poll()
{
    // cleanup and invoke callbacks
    bool sure = _process_selectinfo(si, true);
    if (sure || pending_callback)
    {
        pending_callback = false;
        callback();
    }
}


void WvQtStreamClone::set_timeout(int msec_timeout)
{
    this->msec_timeout = msec_timeout;
}


void WvQtStreamClone::qt_begin_event_loop_hook()
{
    // select not done yet?
    if (select_in_progress) return;

    // finish the last polling stage
    if (! first_time)
        post_poll();
    else
        first_time = false;
    // start the next polling stage
    pre_poll();
    select_in_progress = true;
}


void WvQtStreamClone::qt_detach()
{
    // finish the last polling stage
    if (! first_time)
    {
        select_in_progress = false;
        post_poll();
        last_max_fd = -1;
        first_time = true;
    }
    // remove any remaining Qt objects
    select_timer.stop();
    notify_readable.clear();
    notify_writable.clear();
    notify_exception.clear();
    QObject::disconnect(qApp, SIGNAL(guiThreadAwake()),
        this, SLOT(qt_begin_event_loop_hook()));
    QObject::disconnect(& select_timer, SIGNAL(timeout()),
        this, SLOT(select_timer_expired()));
}


void WvQtStreamClone::qt_attach()
{
    // hook into the Qt event loop before each iteration
    QObject::connect(qApp, SIGNAL(guiThreadAwake()),
        this, SLOT(qt_begin_event_loop_hook()));
    QObject::connect(& select_timer, SIGNAL(timeout()),
        this, SLOT(select_timer_expired()));
}


void WvQtStreamClone::select_timer_expired()
{
    select_in_progress = false;
}


void WvQtStreamClone::fd_readable(int fd)
{
    FD_SET(fd, &si.read);
    pending_callback = true;
    select_in_progress = false;
}


void WvQtStreamClone::fd_writable(int fd)
{
    FD_SET(fd, &si.write);
    pending_callback = true;
    select_in_progress = false;
}


void WvQtStreamClone::fd_exception(int fd)
{
    FD_SET(fd, &si.except);
    pending_callback = true;
    select_in_progress = false;
}

void WvQtStreamClone::execute()
{
    WvStreamClone::execute();
}
