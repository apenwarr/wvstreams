/* -*- Mode: C++ -*-
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2005 Net Integration Technologies, Inc.
 *
 * WvStreams interface for popt argument processing
 */
#ifndef __WVARGS_H
#define __WVARGS_H

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
        WvString args_desc;
        unsigned int num_required_args;
    
    public:

    	WvArgs();
    	~WvArgs();

    	bool process(int argc, char **argv,
    	    	WvStringList *remaining_args = NULL);
    	    	
    	void print_usage(int argc, char **argv);
    	void print_help(int argc, char **argv);
    	    	
    	void add_set_bool_option(char short_option, WvStringParm long_option,
    	    	WvStringParm desc, bool &val);
    	void add_reset_bool_option(char short_option, WvStringParm long_option,
    	    	WvStringParm desc, bool &val);
    	void add_flip_bool_option(char short_option, WvStringParm long_option,
    	    	WvStringParm desc, bool &val);
    	
    	void add_option(char short_option, WvStringParm long_option,
    	    	WvStringParm desc, WvStringParm arg_desc, int &val);
    	void add_option(char short_option, WvStringParm long_option,
    	    	WvStringParm desc, WvStringParm arg_desc, long &val);
    	void add_option(char short_option, WvStringParm long_option,
    	    	WvStringParm desc, WvStringParm arg_desc, float &val);
    	void add_option(char short_option, WvStringParm long_option,
    	    	WvStringParm desc, WvStringParm arg_desc, double &val);
    	void add_option(char short_option, WvStringParm long_option,
    	    	WvStringParm desc, WvStringParm arg_desc, WvString &val);
    	void add_option(char short_option, WvStringParm long_option,
    	    	WvStringParm desc, WvStringParm arg_desc, WvStringList &val);
    	    	
    	void add_option(char short_option, WvStringParm long_option,
    	    	WvStringParm desc, NoArgCallback cb, void *ud = NULL);
    	void add_option(char short_option, WvStringParm long_option,
    	    	WvStringParm desc, WvStringParm arg_desc,
    	    	ArgCallback cb, void *ud = NULL);
    	
        void add_required_arg(WvStringParm desc);
        void add_optional_arg(WvStringParm desc, bool multiple = false);

        void remove_option(char short_option);
        void remove_option(WvStringParm long_option);
        void remove_all_options();
};

#endif // __WVARGS_H
