#ifndef __WVGTKSTREAMCLONE_H
#define __WVGTKSTREAMCLONE_H

#include "wvstreamclone.h"
#include "wvhashtable.h"
#include <gtk/gtk.h>
#include <glib.h>

class WvGtkStreamClone : public WvStreamClone
{
    public:
        friend void eventCallback(gpointer owner, int fd, GdkInputCondition condition)
        {
            ((WvGtkStreamClone *)owner)->event();
        }
        friend gint timeoutFn(gpointer owner)
        {
            ((WvGtkStreamClone *)owner)->event();
            return FALSE;
        }
        WvGtkStreamClone::WvGtkStreamClone(WvStream *_cloned, int msec_timeout);
        WvGtkStreamClone::~WvGtkStreamClone();

    private:
        typedef struct
        {
            int index;
            GdkInputCondition condition;
        } IndexConditionPair;

        void addEvents();
        void event();
		
        SelectInfo si;
        int msec_timeout;
        WvMap <int, IndexConditionPair> fd_input_map;
};

#endif
