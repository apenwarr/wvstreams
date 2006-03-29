/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 *
 * A Qt object that invokes its callback whenever it receives
 * an event.  This is useful for deferring processing to the
 * Qt event loop.  Use it to avoid problems resulting from the
 * non-reentrant nature of WvStream::execute().
 */

#include <QtCore/qobject.h>
#include <Qt/qevent.h>
#include <QtCore/qcoreevent.h>
#include "wvcallback.h"
#include "wvqtevent.h"
//#include "wvqtevent.moc"

WvQtEvent::WvQtEvent(int type, void *_data) : 
    QEvent(QEvent::Type(QEvent::User + type)) {
 data = _data;
}

void * WvQtEvent::getData() {
    return data;
}

void WvQtEvent::setData(void * _data) {
    data = _data;
}
