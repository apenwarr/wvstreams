/*
 * Rotates logs on a daily basis, expiring logs that are older than <keep_for>
 * days
 */
#ifndef __WVLOGROTATOR_H
#define __WVLOGROTATOR_H

#include "wvdailyevent.h"
#include "wvstringlist.h"

class WvLogRotator : public WvDailyEvent
{
    friend class WvLogRotatorTest;
public:
    /**
     * /param _filenames - a space separated list of filenames to manage
     * /param _keep_for - the number of days before expiring old logs
     */
    WvLogRotator(WvStringParm _filenames, int _keep_for = 7);
    
    void set_keep_for(int _keep_for);
private:
    virtual void execute();
    
    WvStringList filenames;
    int keep_for;
};
#endif
