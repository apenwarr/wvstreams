/* -*- Mode: C++ -*-
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * WvStreams interface for popt argument processing
 */
#include "wvautoconf.h"
#ifdef WITH_POPT

#include "wvargs.h"

#include <popt.h>

class WvArgsOption
{
    private:
    	    
    	char short_option;
    	const char *long_option;
    	const char *desc;
        	    
    public:
    	    
    	WvArgsOption(char _short_option,
    	    	const char *_long_option,
    	    	const char *_desc)
    	    : short_option(_short_option),
    	    	    long_option(_long_option),
    	    	    desc(_desc)
    	    {}
    	virtual ~WvArgsOption()
    	    {}
        	    
    	virtual void process(WvStringParm arg)
    	    {}
    	    	    	
    	virtual void fill_popt_table(struct poptOption *popt_option, int popt_val)
    	{
    	    memset(popt_option, 0, sizeof(struct poptOption));
    	    	    
    	    popt_option->longName = long_option;
    	    popt_option->shortName = short_option;
    	    popt_option->descrip = desc;
    	    popt_option->val = popt_val;
    	}
};
    	
class WvArgsNoArgOption : public WvArgsOption
{
    	
    public:
    	    
    	WvArgsNoArgOption(char _short_option,
    	    	const char *_long_option,
    	    	const char *_desc)
    	    : WvArgsOption(_short_option, _long_option, _desc)
    	    {}
        	    
    	virtual void fill_popt_table(struct poptOption *popt_option, int popt_val)
    	{
    	    WvArgsOption::fill_popt_table(popt_option, popt_val);
    	    	    
    	    popt_option->argInfo = POPT_ARG_NONE;
    	}
};

class WvArgsSetBoolOption : public WvArgsNoArgOption
{
    	
    private:
    	    
    	bool &flag;
    	    	
    public:
    	    
    	WvArgsSetBoolOption(char _short_option,
    	    	const char *_long_option,
    	    	const char *_desc,
    	    	bool &_flag)
    	    : WvArgsNoArgOption(_short_option, _long_option, _desc),
    	    	    flag(_flag)
    	    {}

    	virtual void process(WvStringParm arg)
    	{
    	    flag = true;
    	}
};
    	
class WvArgsResetBoolOption : public WvArgsNoArgOption
{
    	
    private:
    	    
    	bool &flag;
    	    	
    public:
    	    
    	WvArgsResetBoolOption(char _short_option,
    	    	const char *_long_option,
    	    	const char *_desc,
    	    	bool &_flag)
    	    : WvArgsNoArgOption(_short_option, _long_option, _desc),
    	    	    flag(_flag)
    	    {}

    	virtual void process(WvStringParm arg)
    	{
    	    flag = false;
    	}
};
    	
class WvArgsFlipBoolOption : public WvArgsNoArgOption
{
    	
    private:
    	    
    	bool &flag;
    	    	
    public:
    	    
    	WvArgsFlipBoolOption(char _short_option,
    	    	const char *_long_option,
    	    	const char *_desc,
    	    	bool &_flag)
    	    : WvArgsNoArgOption(_short_option, _long_option, _desc),
    	    	    flag(_flag)
    	    {}

    	virtual void process(WvStringParm arg)
    	{
    	    flag = !flag;
    	}
};
    	
class WvArgsNoArgCallbackOption : public WvArgsNoArgOption
{
    	
    private:
    	
    	WvArgs::NoArgCallback cb;
    	void *ud;
    	    	
    public:
    	    
    	WvArgsNoArgCallbackOption(char _short_option,
    	    	const char *_long_option,
    	    	const char *_desc,
    	    	WvArgs::NoArgCallback _cb,
    	    	void *_ud)
    	    : WvArgsNoArgOption(_short_option, _long_option, _desc),
    	    	    cb(_cb), ud(_ud)
    	    {}

    	virtual void process(WvStringParm arg)
    	{
    	    cb(ud);
    	}
};
    	
class WvArgsArgOption : public WvArgsOption
{
    private:
    	    
     	const char *arg_desc;
        	    
    public:
    	    
    	WvArgsArgOption(char _short_option,
    	    	const char *_long_option,
    	    	const char *_desc,
    	    	const char *_arg_desc)
    	    : WvArgsOption(_short_option, _long_option, _desc),
    	    	    arg_desc(_arg_desc)
    	    {}
        	    
    	virtual void fill_popt_table(struct poptOption *popt_option, int popt_val)
    	{
    	    WvArgsOption::fill_popt_table(popt_option, popt_val);
    	    	    
    	    popt_option->argDescrip = arg_desc;
    	    popt_option->argInfo = POPT_ARG_STRING;
    	}
};

class WvArgsIntOption : public WvArgsArgOption
{
    private:
    	    
    	int &val;
    	    	
    public:
    	    
     	WvArgsIntOption(char _short_option,
    	    	const char *_long_option,
    	    	const char *_desc,
    	    	const char *_arg_desc,
    	    	int &_val)
    	    : WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
    	    	    val(_val)
    	    {}
        	    
    	virtual void fill_popt_table(struct poptOption *popt_option, int popt_val)
    	{
    	    WvArgsArgOption::fill_popt_table(popt_option, popt_val);
    	    	    
    	    popt_option->arg = (void *)&val;
    	    popt_option->argInfo = POPT_ARG_INT;
    	}
};
    	
class WvArgsLongOption : public WvArgsArgOption
{
    private:
    	    
    	long &val;
    	    	
    public:
    	    
     	WvArgsLongOption(char _short_option,
    	    	const char *_long_option,
    	    	const char *_desc,
    	    	const char *_arg_desc,
    	    	long &_val)
    	    : WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
    	    	    val(_val)
    	    {}
        	    
    	virtual void fill_popt_table(struct poptOption *popt_option, int popt_val)
    	{
    	    WvArgsArgOption::fill_popt_table(popt_option, popt_val);
    	    	    
    	    popt_option->arg = (void *)&val;
    	    popt_option->argInfo = POPT_ARG_LONG;
    	}
};
    	
class WvArgsFloatOption : public WvArgsArgOption
{
    private:
    	    
    	float &val;
    	    	
    public:
    	    
     	WvArgsFloatOption(char _short_option,
    	    	const char *_long_option,
    	    	const char *_desc,
    	    	const char *_arg_desc,
    	    	float &_val)
    	    : WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
    	    	    val(_val)
    	    {}
        	    
    	virtual void fill_popt_table(struct poptOption *popt_option, int popt_val)
    	{
    	    WvArgsArgOption::fill_popt_table(popt_option, popt_val);
    	    	    
    	    popt_option->arg = (void *)&val;
    	    popt_option->argInfo = POPT_ARG_FLOAT;
    	}
};
    	
class WvArgsDoubleOption : public WvArgsArgOption
{
    private:
    	    
    	double &val;
    	    	
    public:
    	    
     	WvArgsDoubleOption(char _short_option,
    	    	const char *_long_option,
    	    	const char *_desc,
    	    	const char *_arg_desc,
     	    	double &_val)
    	    : WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
    	    	    val(_val)
    	    {}
        	    
    	virtual void fill_popt_table(struct poptOption *popt_option, int popt_val)
    	{
    	    WvArgsArgOption::fill_popt_table(popt_option, popt_val);
    	    	    
    	    popt_option->arg = (void *)&val;
    	    popt_option->argInfo = POPT_ARG_DOUBLE;
    	}
};
    	
class WvArgsStringOption : public WvArgsArgOption
{
    private:
    	    
    	WvString &val;
    	    	
    public:
    	    
     	WvArgsStringOption(char _short_option,
    	    	const char *_long_option,
    	    	const char *_desc,
    	    	const char *_arg_desc,
    	    	WvString &_val)
    	    : WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
    	    	    val(_val)
    	    {}
            	
    	virtual void process(WvStringParm arg)
    	{
    	    val = arg;
    	}
};
    	
class WvArgsStringListAppendOption : public WvArgsArgOption
{
    private:
    	    
    	WvStringList &val;
    	    	
    public:
    	    
     	WvArgsStringListAppendOption(char _short_option,
    	    	const char *_long_option,
    	    	const char *_desc,
    	    	const char *_arg_desc,
    	    	WvStringList &_val)
    	    : WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
    	    	    val(_val)
    	    {}
            	
    	virtual void process(WvStringParm arg)
    	{
    	    val.append(arg);
    	}
};
    	
class WvArgsArgCallbackOption : public WvArgsArgOption
{
    private:
    	    
    	WvArgs::ArgCallback cb;
    	void *ud;
    	    	
    public:
    	    
     	WvArgsArgCallbackOption(char _short_option,
    	    	const char *_long_option,
    	    	const char *_desc,
    	    	const char *_arg_desc,
     	    	WvArgs::ArgCallback _cb,
    	    	void *_ud)
    	    : WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
    	    	    cb(_cb), ud(_ud)
    	    {}
            	
    	virtual void process(WvStringParm arg)
    	{
    	    cb(arg, ud);
    	}
};

typedef WvVector<WvArgsOption> WvArgsOptionVector;

WvArgs::WvArgs()
{
    options = new WvArgsOptionVector();
}

WvArgs::~WvArgs()
{
    delete options;
}    	
    	    	    	
bool WvArgs::process(int argc, char **argv, WvStringList *remaining_args = NULL)
{
    struct poptOption *popt_options =
    	    new struct poptOption[options->count() + 2];
    if (!popt_options)
    	return false;
    
    int j;
    for (j=0; ; ++j)
    {
    	WvArgsOption *option = (*options)[j];
    	if (!option) break;
    	
    	option->fill_popt_table(&popt_options[j], j+1);
    }
    
    const struct poptOption extras[2] = {
    	POPT_AUTOHELP
    	POPT_TABLEEND
    };
    memcpy(&popt_options[j++], &extras[0], sizeof(struct poptOption));
    memcpy(&popt_options[j++], &extras[1], sizeof(struct poptOption));
    
    bool result = true;
    
    poptContext popt_context = poptGetContext("WvStreams",
    	    argc, (const char **)argv, popt_options, 0);
    for (;;)
    {
    	int opt = poptGetNextOpt(popt_context);
    	if (opt == -1)
    	    break;
    	else if (opt == POPT_ERROR_NOARG
    	    	|| opt == POPT_ERROR_BADOPT
    	    	|| opt == POPT_ERROR_OPTSTOODEEP
            	|| opt == POPT_ERROR_BADQUOTE
            	|| opt == POPT_ERROR_BADNUMBER
            	|| opt == POPT_ERROR_OVERFLOW)
        {
       	    result = false;
       	    
       	    printf("%s: %s\n\n",
       	    	poptBadOption(popt_context, POPT_BADOPTION_NOALIAS),
       	    	poptStrerror(opt));

    	    poptPrintUsage(popt_context, stdout, 0);
    	    
    	    printf("\nFor detailed options, %s --help\n", argv[0]);

     	    break;
       	}
    	else
    	    (*options)[opt-1]->process(poptGetOptArg(popt_context));
    }
    if (result && remaining_args)
    {
    	for (;;)
    	{
    	    const char *leftover_arg = poptGetArg(popt_context);
    	    if (!leftover_arg) break;
    	    remaining_args->append(leftover_arg);
    	}
    }
    poptFreeContext(popt_context);
    
    deletev popt_options;
    
    return result;
}

void WvArgs::add_set_bool_option(char short_option, const char *long_option,
    	const char *desc, bool &val)
{
    options->append(new WvArgsSetBoolOption(short_option, long_option, desc,
    	    val), true);
}

void WvArgs::add_reset_bool_option(char short_option, const char *long_option,
    	const char *desc, bool &val)
{
    options->append(new WvArgsResetBoolOption(short_option, long_option, desc,
    	    val), true);
}

void WvArgs::add_flip_bool_option(char short_option, const char *long_option,
    	const char *desc, bool &val)
{
    options->append(new WvArgsFlipBoolOption(short_option, long_option, desc,
    	    val), true);
}

void WvArgs::add_option(char short_option, const char *long_option,
    	const char *desc, NoArgCallback cb, void *ud)
{
    options->append(new WvArgsNoArgCallbackOption(short_option, long_option, desc,
    	    cb, ud), true);
}

void WvArgs::add_option(char short_option, const char *long_option,
    	const char *desc, const char *arg_desc, int &val)
{
    options->append(new WvArgsIntOption(short_option, long_option, desc,
    	    arg_desc, val), true);
}

void WvArgs::add_option(char short_option, const char *long_option,
    	const char *desc, const char *arg_desc, long &val)
{
    options->append(new WvArgsLongOption(short_option, long_option, desc,
    	    arg_desc, val), true);
}

void WvArgs::add_option(char short_option, const char *long_option,
    	const char *desc, const char *arg_desc, float &val)
{
    options->append(new WvArgsFloatOption(short_option, long_option, desc,
    	    arg_desc, val), true);
}

void WvArgs::add_option(char short_option, const char *long_option,
    	const char *desc, const char *arg_desc, double &val)
{
    options->append(new WvArgsDoubleOption(short_option, long_option, desc,
    	    arg_desc, val), true);
}

void WvArgs::add_option(char short_option, const char *long_option,
    	const char *desc, const char *arg_desc, WvString &val)
{
    options->append(new WvArgsStringOption(short_option, long_option, desc,
    	    arg_desc, val), true);
}

void WvArgs::add_option(char short_option, const char *long_option,
    	const char *desc, const char *arg_desc, WvStringList &val)
{
    options->append(new WvArgsStringListAppendOption(short_option, long_option, desc,
    	    arg_desc, val), true);
}

void WvArgs::add_option(char short_option, const char *long_option,
    	const char *desc, const char *arg_desc, ArgCallback cb, void *ud)
{
    options->append(new WvArgsArgCallbackOption(short_option, long_option, desc,
    	    arg_desc, cb, ud), true);
}

#endif // WITH_POPT
