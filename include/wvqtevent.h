/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A Qt object that invokes its callback whenever it receives
 * an event.  This is useful for deferring processing to the
 * Qt event loop.  Use it to avoid problems resulting from the
 * non-reentrant nature of WvStream::execute().
 */
#ifndef __WVQTEVENT_H
#define __WVQTEVENT_H

//#include <QtCore/qobject.h>
#include <Qt/qevent.h>
#include <QtCore/qcoreevent.h>
#include "wvcallback.h"

class WvQtEvent : public QEvent
{
public:
    WvQtEvent(int type, void *_data = 0);

    void * getData();

    void setData(void * _data);

private:
    void * data;
};

#endif // __WVQTEVENT_H
