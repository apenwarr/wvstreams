/* -*- Mode: C++ -*-
 * Worldvisions Tunnel Vision Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * WvStreams interface for popt argument processing
 */

#include "wvargs.h"

#include <popt.h>

class WvArgsOption
{
public:

    char short_option;
    WvString long_option;
    WvString desc;

    WvArgsOption(char _short_option,
		 WvStringParm _long_option,
		 WvStringParm _desc)
	: short_option(_short_option), long_option(_long_option), desc(_desc)
    {
    }

    virtual ~WvArgsOption()
    {
    }

    virtual void process(WvStringParm arg)
    {
    }

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
		      WvStringParm _long_option,
		      WvStringParm _desc)
	: WvArgsOption(_short_option, _long_option, _desc)
    {
    }

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
			WvStringParm _long_option,
			WvStringParm _desc,
			bool &_flag)
	: WvArgsNoArgOption(_short_option, _long_option, _desc),
	  flag(_flag)
    {
    }

    virtual void process(WvStringParm  arg)
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
			  WvStringParm _long_option,
			  WvStringParm _desc,
			  bool &_flag)
	: WvArgsNoArgOption(_short_option, _long_option, _desc),
	  flag(_flag)
    {
    }

    virtual void process(WvStringParm  arg)
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
			 WvStringParm _long_option,
			 WvStringParm _desc,
			 bool &_flag)
	: WvArgsNoArgOption(_short_option, _long_option, _desc),
	  flag(_flag)
    {
    }

    virtual void process(WvStringParm  arg)
    {
	flag = !flag;
    }
};

class WvArgsIncIntOption : public WvArgsNoArgOption
{
private:
    int &val;

public:
    WvArgsIncIntOption(char _short_option,
		       WvStringParm _long_option,
		       WvStringParm _desc,
		       int &_val)
	: WvArgsNoArgOption(_short_option, _long_option, _desc),
	  val(_val)
    {
    }

    virtual void process(WvStringParm  arg)
    {
	val++;
    }
};

class WvArgsNoArgCallbackOption : public WvArgsNoArgOption
{

private:

    WvArgs::NoArgCallback cb;
    void *ud;

public:

    WvArgsNoArgCallbackOption(char _short_option,
			      WvStringParm _long_option,
			      WvStringParm _desc,
			      WvArgs::NoArgCallback _cb,
			      void *_ud)
	: WvArgsNoArgOption(_short_option, _long_option, _desc),
	  cb(_cb), ud(_ud)
    {
    }

    virtual void process(WvStringParm  arg)
    {
	cb(ud);
    }
};

class WvArgsArgOption : public WvArgsOption
{
private:

    WvString arg_desc;

public:

    WvArgsArgOption(char _short_option,
		    WvStringParm _long_option,
		    WvStringParm _desc,
		    WvStringParm _arg_desc)
	: WvArgsOption(_short_option, _long_option, _desc),
	  arg_desc(_arg_desc)
    {
    }

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
		    WvStringParm _long_option,
		    WvStringParm _desc,
		    WvStringParm _arg_desc,
		    int &_val)
	: WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
	  val(_val)
    {
    }

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
		     WvStringParm _long_option,
		     WvStringParm _desc,
		     WvStringParm _arg_desc,
		     long &_val)
	: WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
	  val(_val)
    {
    }

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
		      WvStringParm _long_option,
		      WvStringParm _desc,
		      WvStringParm _arg_desc,
		      float &_val)
	: WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
	  val(_val)
    {
    }

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
		       WvStringParm _long_option,
		       WvStringParm _desc,
		       WvStringParm _arg_desc,
		       double &_val)
	: WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
	  val(_val)
    {
    }

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
		       WvStringParm _long_option,
		       WvStringParm _desc,
		       WvStringParm _arg_desc,
		       WvString &_val)
	: WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
	  val(_val)
    {
    }

    virtual void process(WvStringParm  arg)
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
				 WvStringParm _long_option,
				 WvStringParm _desc,
				 WvStringParm _arg_desc,
				 WvStringList &_val)
	: WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
	  val(_val)
    {
    }

    virtual void process(WvStringParm  arg)
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
			    WvStringParm _long_option,
			    WvStringParm _desc,
			    WvStringParm _arg_desc,
			    WvArgs::ArgCallback _cb,
			    void *_ud)
	: WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
	  cb(_cb), ud(_ud)
    {
    }

    virtual void process(WvStringParm  arg)
    {
	cb(arg, ud);
    }
};

typedef WvVector<WvArgsOption> WvArgsOptionVector;

WvArgs::WvArgs()
    : num_required_args(0)
{
    options = new WvArgsOptionVector();
}

WvArgs::~WvArgs()
{
    delete options;
}

static bool create_popt_context(int argc, char **argv,
				WvVector<WvArgsOption> *options,
				WvStringParm args_desc,
				poptContext &popt_context,
				struct poptOption **popt_options)
{
    (*popt_options) = new struct poptOption[options->count() + 2];
    if (!popt_options)
	return false;

    int j;
    for (j=0; ; ++j)
    {
	WvArgsOption *option = (*options)[j];
	if (!option)
	    break;

	if (option->short_option || option->long_option)
	    option->fill_popt_table(&(*popt_options)[j], j+1);
    }

    const struct poptOption extras[2] = {
	POPT_AUTOHELP
	POPT_TABLEEND
    };

    memcpy(&(*popt_options)[j++], &extras[0], sizeof(struct poptOption));
    memcpy(&(*popt_options)[j++], &extras[1], sizeof(struct poptOption));

    popt_context = poptGetContext(argv[0], argc, (const char **)argv,
				  (*popt_options), 0);

    WvString usage_desc;
    if (options->count() > 0)
	usage_desc = "[OPTION...] ";
    usage_desc.append(args_desc);

    poptSetOtherOptionHelp(popt_context, usage_desc.cstr());

    return true;
}

bool WvArgs::process(int argc, char **argv, WvStringList *remaining_args)
{
    poptContext popt_context;
    struct poptOption *popt_options;
    if (!create_popt_context(argc, argv, options, args_desc, popt_context,
			     &popt_options))
	return false;

    bool result = true;

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
	    WvStringParm leftover_arg = poptGetArg(popt_context);
	    if (!leftover_arg) break;
	    remaining_args->append(leftover_arg);
	}

	if (remaining_args->count() < num_required_args)
	{
	    result = false;

	    poptPrintUsage(popt_context, stdout, 0);
	}
    }

    poptFreeContext(popt_context);
    deletev popt_options;

    return result;
}

void WvArgs::print_usage(int argc, char **argv)
{
    poptContext popt_context;
    struct poptOption *popt_options;

    create_popt_context(argc, argv, options, args_desc, popt_context,
			&popt_options);

    poptPrintUsage(popt_context, stdout, 0);

    poptFreeContext(popt_context);
    deletev popt_options;
}

void WvArgs::print_help(int argc, char **argv)
{
    poptContext popt_context;
    struct poptOption *popt_options;
    create_popt_context(argc, argv, options, args_desc, popt_context,
			&popt_options);

    poptPrintHelp(popt_context, stdout, 0);

    poptFreeContext(popt_context);
    deletev popt_options;
}

void WvArgs::add_set_bool_option(char short_option, WvStringParm long_option,
				 WvStringParm desc, bool &val)
{
    remove_option(short_option);
    remove_option(long_option);

    options->append(new WvArgsSetBoolOption(short_option, long_option, desc,
					    val), true);
}


void WvArgs::add_reset_bool_option(char short_option, WvStringParm long_option,
				   WvStringParm desc, bool &val)
{
    remove_option(short_option);
    remove_option(long_option);

    options->append(new WvArgsResetBoolOption(short_option, long_option, desc,
					      val),
		    true);
}


void WvArgs::add_flip_bool_option(char short_option, WvStringParm long_option,
				  WvStringParm desc, bool &val)
{
    remove_option(short_option);
    remove_option(long_option);

    options->append(new WvArgsFlipBoolOption(short_option, long_option, desc,
					     val),
		    true);
}


void WvArgs::add_option(char short_option, WvStringParm long_option,
			WvStringParm desc, NoArgCallback cb, void *ud)
{
    remove_option(short_option);
    remove_option(long_option);

    options->append(new WvArgsNoArgCallbackOption(short_option, long_option,
						  desc, cb, ud),
		    true);
}

void WvArgs::add_option(char short_option, WvStringParm long_option,
			WvStringParm desc, WvStringParm arg_desc, int &val)
{
    remove_option(short_option);
    remove_option(long_option);

    options->append(new WvArgsIntOption(short_option, long_option, desc,
					arg_desc, val),
		    true);
}

void WvArgs::add_option(char short_option, WvStringParm long_option,
			WvStringParm desc, WvStringParm arg_desc, long &val)
{
    remove_option(short_option);
    remove_option(long_option);

    options->append(new WvArgsLongOption(short_option, long_option, desc,
					 arg_desc, val),
		    true);
}

void WvArgs::add_option(char short_option, WvStringParm long_option,
			WvStringParm desc, WvStringParm arg_desc, float &val)
{
    remove_option(short_option);
    remove_option(long_option);

    options->append(new WvArgsFloatOption(short_option, long_option, desc,
					  arg_desc, val),
		    true);
}

void WvArgs::add_option(char short_option, WvStringParm long_option,
			WvStringParm desc, WvStringParm arg_desc, double &val)
{
    remove_option(short_option);
    remove_option(long_option);

    options->append(new WvArgsDoubleOption(short_option, long_option, desc,
					   arg_desc, val),
		    true);
}

void WvArgs::add_option(char short_option, WvStringParm long_option,
			WvStringParm desc, WvStringParm arg_desc,
			WvString &val)
{
    remove_option(short_option);
    remove_option(long_option);

    options->append(new WvArgsStringOption(short_option, long_option, desc,
					   arg_desc, val),
		    true);
}

void WvArgs::add_option(char short_option, WvStringParm long_option,
			WvStringParm desc, WvStringParm arg_desc,
			WvStringList &val)
{
    remove_option(short_option);
    remove_option(long_option);

    options->append(new WvArgsStringListAppendOption(short_option, long_option,
						     desc, arg_desc, val),
		    true);
}

void WvArgs::add_option(char short_option, WvStringParm long_option,
			WvStringParm desc, WvStringParm arg_desc,
			ArgCallback cb, void *ud)
{
    remove_option(short_option);
    remove_option(long_option);

    options->append(new WvArgsArgCallbackOption(short_option, long_option,
						desc, arg_desc, cb, ud),
		    true);
}

void WvArgs::remove_option(char short_option)
{
    if (short_option == 0)
	return;

    WvArgsOptionVector::Iter i(*options);
    for (i.rewind(); i.next(); )
    {
	if (i->short_option == short_option)
	    i->short_option = 0;
    }
}

void WvArgs::remove_option(WvStringParm long_option)
{
    if (long_option == NULL)
	return;

    WvArgsOptionVector::Iter i(*options);
    for (i.rewind(); i.next(); )
    {
	if (i->long_option && (long_option == i->long_option))
	    i->long_option = WvString::null;
    }
}

void WvArgs::remove_all_options()
{
    delete options;
    options = new WvArgsOptionVector();
}

void WvArgs::add_required_arg(WvStringParm desc)
{
    num_required_args++;
    add_optional_arg(desc);
}

void WvArgs::add_optional_arg(WvStringParm desc, bool multiple)
{
    // an optional arg is a required arg without the requirement :-)
    if (args_desc.len() > 0)
	args_desc.append(" ");
    args_desc.append("[%s]", desc);
    if (multiple)
	args_desc.append("...");
}
