/* -*- Mode: C++ -*-
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * WvStreams interface for popt argument processing
 */
#ifndef __WVARGS_H
#define __WVARGS_H

#include "wvautoconf.h"
#ifdef WITH_POPT

#include "wvstring.h"
#include "wvstringlist.h"
#include "wvcallback.h"
#include "wvvector.h"

class WvArgsOption;

class WvArgs
{
    public:
    
    	typedef WvCallback<void, void *> NoArgCallback;
	typedef WvCallback<void, WvStringParm, void *> ArgCallback;
    	
    private:
    
    	WvVector<WvArgsOption> *options;
    
    public:

    	WvArgs();
    	~WvArgs();

    	bool process(int argc, char **argv,
    	    	WvStringList *remaining_args = NULL);
    	    	
    	void print_usage();
    	void print_help();
    	    	
    	void add_set_bool_option(char short_option, const char *long_option,
    	    	const char *desc, bool &val);
    	void add_reset_bool_option(char short_option, const char *long_option,
    	    	const char *desc, bool &val);
    	void add_flip_bool_option(char short_option, const char *long_option,
    	    	const char *desc, bool &val);
    	    	
    	void add_option(char short_option, const char *long_option,
    	    	const char *desc, const char *arg_desc, int &val);
    	void add_option(char short_option, const char *long_option,
    	    	const char *desc, const char *arg_desc, long &val);
    	void add_option(char short_option, const char *long_option,
    	    	const char *desc, const char *arg_desc, float &val);
    	void add_option(char short_option, const char *long_option,
    	    	const char *desc, const char *arg_desc, double &val);
    	void add_option(char short_option, const char *long_option,
    	    	const char *desc, const char *arg_desc, WvString &val);
    	void add_option(char short_option, const char *long_option,
    	    	const char *desc, const char *arg_desc, WvStringList &val);
    	    	
    	void add_option(char short_option, const char *long_option,
    	    	const char *desc, NoArgCallback cb, void *ud);
    	void add_option(char short_option, const char *long_option,
    	    	const char *desc, const char *arg_desc,
    	    	ArgCallback cb, void *ud);
 
};

#endif // WITH_POPT

#endif // __WVARGS_H
