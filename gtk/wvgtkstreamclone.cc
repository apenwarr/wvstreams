#include "wvgtkstreamclone.h"

WvGtkStreamClone::WvGtkStreamClone(WvStream *_cloned, int msec_timeout)
    : WvStreamClone(_cloned), msec_timeout(msec_timeout), fd_input_map(65537)
{
    addEvents();
}

void WvGtkStreamClone::addEvents()
{
    bool sure = _build_selectinfo(si, msec_timeout, false, false, false, true);
    if(sure)
        si.msec_timeout = 0;

    IndexConditionPair * p = fd_input_map.find(-1);
    if(p != 0)
    {
        gtk_timeout_remove(p->index);
        fd_input_map.remove(-1);
    }
    if(si.msec_timeout >= 0)
    {
        IndexConditionPair idp;
        idp.index = gtk_timeout_add_full(si.msec_timeout, timeoutFn, 0, this, 0);
        fd_input_map.add(-1, idp);
    }

    for(int fd = 0; fd <= si.max_fd; ++fd)
    {
        GdkInputCondition condition = static_cast <GdkInputCondition> ((FD_ISSET(fd, &si.read) ? GDK_INPUT_READ : 0) | (FD_ISSET(fd, &si.write) ? GDK_INPUT_WRITE : 0) | (FD_ISSET(fd, &si.except) ? GDK_INPUT_EXCEPTION : 0));
        IndexConditionPair * p = fd_input_map.find(fd);
        if(p != 0)
        {
            if(p->condition == condition)
                continue;
            gtk_input_remove(p->index);
            fd_input_map.remove(fd);
        }
        if(condition == 0)
            continue;

        IndexConditionPair icp;
        icp.index = gtk_input_add_full(fd, condition, eventCallback, 0, this, 0);
        icp.condition = condition;
        fd_input_map.add(fd, icp);
    }
    FD_ZERO(&si.read);
    FD_ZERO(&si.write);
    FD_ZERO(&si.except);
}

void WvGtkStreamClone::event()
{
    _process_selectinfo(si, true);
    callback();

    addEvents();
}

WvGtkStreamClone::~WvGtkStreamClone()
{
    WvMap <int, IndexConditionPair>::Iter i(fd_input_map);
    for(i.rewind(); i.next();)
    {
        IndexConditionPair * p = (IndexConditionPair *)i.cur()->data;
        gtk_input_remove(p->index);
    }
    fd_input_map.zap();
}

