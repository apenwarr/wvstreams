/* -*- Mode: C++ -*-
 *   Copyright (C) 2004-2005 Net Integration Technologies, Inc.
 *
 * WvStreams interface for command-line argument processing
 */

#include "wvargs.h"
#include "wvscatterhash.h"

#include <argp.h>


class WvArgsOption
{
public:

    int short_option;
    WvString long_option;
    WvString desc;

    WvArgsOption(int _short_option,
		 WvStringParm _long_option,
		 WvStringParm _desc)
	: short_option(_short_option), long_option(_long_option), desc(_desc)
    {
    }

    virtual ~WvArgsOption()
    {
    }

    virtual WvString process(WvStringParm arg)
    {
	return WvString::null;
    }

    virtual void add_to_argp(WvArgsData &data);
};


DeclareWvList(WvArgsOption);
DeclareWvScatterDict(WvArgsOption, int, short_option);

class WvArgsData
{
public:
    WvArgsData();
    ~WvArgsData();

    argp_option *argp() const;
    void *self() const;

    void add(WvArgsOption *option);
    void remove(char short_option, WvStringParm long_option);
    void zap();

    void add_required_arg();
    void subtract_required_arg();
    const WvStringList &args() const;

    static error_t parser(int key, char *arg, argp_state *state);

    unsigned int flags;

protected:
    friend class WvArgsOption;
    friend class WvArgsArgOption;
    friend class WvArgs;

    void argp_build();
    bool argp_add(const char *name, int key, const char *arg, int flags,
		  const char *doc, int group);
private:
    void argp_init(size_t size = 0);

    bool argp_add(const argp_option &option);
    bool argp_double();

    argp_option *argp_;
    size_t argp_index;		// Last element in the options array
    size_t argp_size;		// Size of the options array

    // I create two data-structures, only one of them actually owning
    // the objects, of course.  The List is for ordered construction
    // of argp_.  The Dict is for constant-time lookups when
    // process()ing options.
    WvArgsOptionList options_list; // An ordered list of WvArgsOptions
    WvArgsOptionDict options_dict; // A constant-time lookup of them

    WvStringList args_;		// Arguments after all options have been parsed
    size_t required_args;       // Number of these mandatory arguments.
    size_t maximum_args;	// Number of maximum arguments.

    int last_no_key;		// Last key for options with no short_option
};


void WvArgsOption::add_to_argp(WvArgsData &data)
{
    data.argp_add(long_option, short_option, 0, 0, desc, 0);
}


class WvArgsNoArgOption : public WvArgsOption
{

public:

    WvArgsNoArgOption(int _short_option,
		      WvStringParm _long_option,
		      WvStringParm _desc)
	: WvArgsOption(_short_option, _long_option, _desc)
    {
    }
};


class WvArgsSetBoolOption : public WvArgsNoArgOption
{

private:

    bool &flag;

public:

    WvArgsSetBoolOption(int _short_option,
			WvStringParm _long_option,
			WvStringParm _desc,
			bool &_flag)
	: WvArgsNoArgOption(_short_option, _long_option, _desc),
	  flag(_flag)
    {
    }

    virtual WvString process(WvStringParm arg)
    {
	flag = true;
	return WvString::null;
    }
};


class WvArgsResetBoolOption : public WvArgsNoArgOption
{

private:

    bool &flag;

public:

    WvArgsResetBoolOption(int _short_option,
			  WvStringParm _long_option,
			  WvStringParm _desc,
			  bool &_flag)
	: WvArgsNoArgOption(_short_option, _long_option, _desc),
	  flag(_flag)
    {
    }

    virtual WvString process(WvStringParm arg)
    {
	flag = false;
	return WvString::null;
    }
};


class WvArgsFlipBoolOption : public WvArgsNoArgOption
{

private:

    bool &flag;

public:

    WvArgsFlipBoolOption(int _short_option,
			 WvStringParm _long_option,
			 WvStringParm _desc,
			 bool &_flag)
	: WvArgsNoArgOption(_short_option, _long_option, _desc),
	  flag(_flag)
    {
    }

    virtual WvString process(WvStringParm arg)
    {
	flag = !flag;
	return WvString::null;
    }
};


class WvArgsIncIntOption : public WvArgsNoArgOption
{
private:
    int &val;

public:
    WvArgsIncIntOption(int _short_option,
		       WvStringParm _long_option,
		       WvStringParm _desc,
		       int &_val)
	: WvArgsNoArgOption(_short_option, _long_option, _desc),
	  val(_val)
    {
    }

    virtual WvString process(WvStringParm arg)
    {
	val++;
	return WvString::null;
    }
};


class WvArgsNoArgCallbackOption : public WvArgsNoArgOption
{

private:

    WvArgs::NoArgCallback cb;
    void *ud;

public:

    WvArgsNoArgCallbackOption(int _short_option,
			      WvStringParm _long_option,
			      WvStringParm _desc,
			      WvArgs::NoArgCallback _cb,
			      void *_ud)
	: WvArgsNoArgOption(_short_option, _long_option, _desc),
	  cb(_cb), ud(_ud)
    {
    }

    virtual WvString process(WvStringParm arg)
    {
	if (cb(ud))
	    return WvString::null;
	else
	    return WvString("invalid option `%s'", arg);
    }
};


class WvArgsArgOption : public WvArgsOption
{
private:

    WvString arg_desc;

public:

    WvArgsArgOption(int _short_option,
		    WvStringParm _long_option,
		    WvStringParm _desc,
		    WvStringParm _arg_desc)
	: WvArgsOption(_short_option, _long_option, _desc),
	  arg_desc(_arg_desc)
    {
    }

    virtual void add_to_argp(WvArgsData &data)
    {
	data.argp_add(long_option, short_option, arg_desc, 0, desc, 0);
    }
};


class WvArgsIntOption : public WvArgsArgOption
{
private:

    int &val;

public:

    WvArgsIntOption(int _short_option,
		    WvStringParm _long_option,
		    WvStringParm _desc,
		    WvStringParm _arg_desc,
		    int &_val)
	: WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
	  val(_val)
    {
    }

    virtual WvString process(WvStringParm arg)
    {
	char *tailptr = NULL;
	errno = 0;
	long int tmp = strtol(arg, &tailptr, 10);
	if (errno == ERANGE || tmp > INT_MAX || tmp < INT_MIN )
	{
	    // Out of range
	    return WvString("`%s': invalid number.", arg);
	}
	else if (*tailptr)
	{
	    // Invalid number
	    return WvString("`%s': invalid number.", arg);
	}
	else
	{
	    val = tmp;
	    return WvString::null;
	}
    }
};


class WvArgsLongOption : public WvArgsArgOption
{
private:

    long &val;

public:

    WvArgsLongOption(int _short_option,
		     WvStringParm _long_option,
		     WvStringParm _desc,
		     WvStringParm _arg_desc,
		     long &_val)
	: WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
	  val(_val)
    {
    }

    virtual WvString process(WvStringParm arg)
    {
	char *tailptr = NULL;
	errno = 0;
	long int tmp = strtol(arg, &tailptr, 10);
	if (errno == ERANGE)
	{
	    // Out of range
	    return WvString("`%s': invalid number.", arg);
	}
	else if (*tailptr)
	{
	    // Invalid number
	    return WvString("`%s': invalid number.", arg);
	}
	else
	{
	    val = tmp;
	    return WvString::null;
	}
    }
};


class WvArgsFloatOption : public WvArgsArgOption
{
private:

    float &val;

public:

    WvArgsFloatOption(int _short_option,
		      WvStringParm _long_option,
		      WvStringParm _desc,
		      WvStringParm _arg_desc,
		      float &_val)
	: WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
	  val(_val)
    {
    }

    virtual WvString process(WvStringParm arg)
    {
	char *tailptr = NULL;
	errno = 0;
	float tmp = strtof(arg, &tailptr);
	if (errno == ERANGE)
	{
	    // Out of range
	    return WvString("`%s': invalid number.", arg);
	}
	else if (*tailptr)
	{
	    // Invalid number
	    return WvString("`%s': invalid number.", arg);
	}
	else
	{
	    val = tmp;
	    return WvString::null;
	}
    }
};


class WvArgsDoubleOption : public WvArgsArgOption
{
private:

    double &val;

public:

    WvArgsDoubleOption(int _short_option,
		       WvStringParm _long_option,
		       WvStringParm _desc,
		       WvStringParm _arg_desc,
		       double &_val)
	: WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
	  val(_val)
    {
    }

    virtual WvString process(WvStringParm arg)
    {
	char *tailptr = NULL;
	errno = 0;
	double tmp = strtod(arg, &tailptr);
	if (errno == ERANGE)
	{
	    // Out of range
	    return WvString("`%s': invalid number.", arg);
	}
	else if (*tailptr)
	{
	    // Invalid number
	    return WvString("`%s': invalid number.", arg);
	}
	else
	{
	    val = tmp;
	    return WvString::null;
	}
    }
};


class WvArgsStringOption : public WvArgsArgOption
{
private:

    WvString &val;

public:

    WvArgsStringOption(int _short_option,
		       WvStringParm _long_option,
		       WvStringParm _desc,
		       WvStringParm _arg_desc,
		       WvString &_val)
	: WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
	  val(_val)
    {
    }

    virtual WvString process(WvStringParm arg)
    {
	val = arg;
	return WvString::null;
    }
};


class WvArgsStringListAppendOption : public WvArgsArgOption
{
private:

    WvStringList &val;

public:

    WvArgsStringListAppendOption(int _short_option,
				 WvStringParm _long_option,
				 WvStringParm _desc,
				 WvStringParm _arg_desc,
				 WvStringList &_val)
	: WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
	  val(_val)
    {
    }

    virtual WvString process(WvStringParm arg)
    {
	val.append(arg);
	return WvString::null;
    }
};


class WvArgsArgCallbackOption : public WvArgsArgOption
{
private:

    WvArgs::ArgCallback cb;
    void *ud;

public:

    WvArgsArgCallbackOption(int _short_option,
			    WvStringParm _long_option,
			    WvStringParm _desc,
			    WvStringParm _arg_desc,
			    WvArgs::ArgCallback _cb,
			    void *_ud)
	: WvArgsArgOption(_short_option, _long_option, _desc, _arg_desc),
	  cb(_cb), ud(_ud)
    {
    }

    virtual WvString process(WvStringParm arg)
    {
	if (cb(arg, ud))
	    return WvString::null;
	else
	    return WvString("invalid option: `%s'", arg);
    }
};


WvArgsData::WvArgsData()
    : flags(0), argp_(NULL), argp_index(0), argp_size(0),
      required_args(0), maximum_args(0), last_no_key(-1)
{
}


WvArgsData::~WvArgsData()
{
    if (argp_)
	free(argp_);
}


argp_option *WvArgsData::argp() const
{
    return argp_;
}


void *WvArgsData::self() const
{
    return (void *)this;
}


void WvArgsData::add(WvArgsOption *option)
{
    if (!option)
	return;

    if (!option->short_option)
	option->short_option = last_no_key--;

    options_list.append(option, true);
    options_dict.add(option, false);
}


// This method removes both short_option and long_option from the
// options_* structures.  Completely.
void WvArgsData::remove(char short_option, WvStringParm long_option)
{
    // First, look through options_list, and remove them from
    // options_dict once we find them.
    WvArgsOptionList::Iter i(options_list);
    for (i.rewind(); i.next(); )
    {
	bool matches_short = false;
	bool matches_long = false;

	if (short_option != '\0' && i->short_option == short_option)
	    matches_short = true;
	if (!long_option.isnull() && i->long_option == long_option)
	    matches_long = true;

	if (matches_short && matches_long
	    || matches_short && i->long_option.isnull()
	    || matches_long && i->short_option == '\0')
	{
	    // Delete this item from the data-structures
	    options_dict.remove(i.ptr());
	    i.xunlink();
	    if (argp_)
	    {
		free(argp_);
		argp_ = NULL;
	    }
	}
	else if (matches_short)
	{
	    // Update the short description and change how it's filed
	    // in the dictionary.
	    i->short_option = '\0';
	    options_dict.remove(i.ptr());
	    options_dict.add(i.ptr(), false);
	}
	else if (matches_long)
	{
	    // Update the long description only
	    i->long_option = WvString::null;
	}
    }
}


void WvArgsData::zap()
{
    options_dict.zap();
    options_list.zap();

    if (argp_)
    {
	free(argp_);
	argp_ = NULL;
    }
}


void WvArgsData::argp_init(size_t size)
{
    argp_size = size;
    if (argp_size < 1)
	argp_size = 1;

    // I'm sorry to use malloc(), but this argp is a C library
    argp_ = (argp_option *)malloc(argp_size * sizeof(argp_option));
    // Terminate the empty array
    memset(argp_, 0, sizeof(argp_option));
}


void WvArgsData::argp_build()
{
    if (!argp_)
	argp_init(options_list.count() + 2);

    WvArgsOptionList::Iter i(options_list);
    for (i.rewind(); i.next(); )
	i->add_to_argp(*this);
}


bool WvArgsData::argp_add(const argp_option &option)
{
    if (argp_index >= (argp_size - 1))
    {
	if (!argp_double())
	    return false;
    }

    // Make a copy of the option that we're building.
    memcpy(argp_ + argp_index, &option, sizeof(argp_option));
    // Terminate the array.
    ++argp_index;
    memset(argp_ + argp_index, 0, sizeof(argp_option));
    return true;
}


bool WvArgsData::argp_add(const char *name, int key, const char *arg,
			  int flags, const char *doc, int group)
{
    if (argp_index >= (argp_size - 1))
    {
	if (!argp_double())
	    return false;
    }

    // Set the elements.
    argp_option *option = argp_ + argp_index;
    option->name = name;
    option->key = key;
    option->arg = arg;
    option->flags = flags;
    option->doc = doc;
    option->group = group;
    // Terminate the array.
    ++argp_index;
    memset(argp_ + argp_index, 0, sizeof(argp_option));
    return true;
}


bool WvArgsData::argp_double()
{
    // We won't be able to fit the next entry into the array
    void *tmp = realloc(argp_, 2 * argp_size * sizeof(argp_option));
    if (!tmp)
	return false;

    argp_ = (argp_option *)tmp;
    argp_size *= 2;
    return true;
}


void WvArgsData::add_required_arg()
{
    ++required_args;
}


void WvArgsData::subtract_required_arg()
{
    --required_args;
}


const WvStringList &WvArgsData::args() const
{
    return args_;
}


error_t WvArgsData::parser(int key, char *arg, struct argp_state *state)
{
    WvArgsData *data = (WvArgsData *)state->input;

    switch (key)
    {
    case ARGP_KEY_ARG:
	if (state->arg_num >= data->maximum_args)
	{
	    // Too many arguments
	    argp_usage(state);
	}
	data->args_.append(arg);
	break;

    case ARGP_KEY_NO_ARGS:
    case ARGP_KEY_END:
	if (state->arg_num < data->required_args)
	{
	    // Too few arguments
	    argp_usage(state);
	}
	break;

    default:
	WvArgsOption *option = data->options_dict[key];
	if (option)
	{
	    WvString error = option->process(arg);
	    if (!error.isnull())
	    {
		argp_failure(state, argp_err_exit_status, 0,
			     "%s", error.cstr());
		return EINVAL;
	    }
	}
	else
	    return ARGP_ERR_UNKNOWN;
    }

    return 0;
}


WvArgs::WvArgs()
    : data(new WvArgsData())
{
}


WvArgs::~WvArgs()
{
    if (data)
	delete data;
}


bool WvArgs::process(int argc, char **argv, WvStringList *remaining_args)
{
    if (!data->argp())
	data->argp_build();

    // Setup --help headers and footers
    WvString prog_doc;
    if (header && footer)
	prog_doc = WvString("%s\v%s", header, footer);
    else if (header)
	prog_doc = WvString("%s", header);
    else if (footer)
	prog_doc = WvString(" \v%s", footer);

    // Setup the constant version number and e-mail address
    argp_program_version = version;
    argp_program_bug_address = email;

    struct argp argp = { data->argp(), &WvArgsData::parser, args_doc, prog_doc,
			 0, 0, 0 };

    bool error = argp_parse(&argp, argc, argv, data->flags, 0, data->self());

    if (remaining_args)
    {
	remaining_args->zap();
	WvStringList::Iter i(data->args());
	for (i.rewind(); i.next(); )
	    remaining_args->add(new WvString(*i), true);
    }

    return !error;
}


void WvArgs::set_version(WvStringParm version)
{
    this->version = version;
}


void WvArgs::set_email(WvStringParm email)
{
    this->email = email;
}


void WvArgs::set_help_header(WvStringParm header)
{
    this->header = header;
}


void WvArgs::set_help_footer(WvStringParm footer)
{
    this->footer = footer;
}


void WvArgs::print_usage(int argc, char **argv)
{
    struct argp argp = { data->argp(), 0, 0, 0, 0, 0, 0 };
    argp_help(&argp, stdout, ARGP_HELP_STD_USAGE, argv[0]);
}


void WvArgs::print_help(int argc, char **argv)
{
    struct argp argp = { data->argp(), 0, 0, 0, 0, 0, 0 };
    argp_help(&argp, stdout, ARGP_HELP_STD_HELP, argv[0]);
}

void WvArgs::add_set_bool_option(char short_option, WvStringParm long_option,
				 WvStringParm desc, bool &val)
{
    data->remove(short_option, long_option);
    data->add(new WvArgsSetBoolOption(short_option, long_option, desc, val));
}


void WvArgs::add_reset_bool_option(char short_option, WvStringParm long_option,
				   WvStringParm desc, bool &val)
{
    data->remove(short_option, long_option);
    data->add(new WvArgsResetBoolOption(short_option, long_option, desc, val));
}


void WvArgs::add_flip_bool_option(char short_option, WvStringParm long_option,
				  WvStringParm desc, bool &val)
{
    data->remove(short_option, long_option);
    data->add(new WvArgsFlipBoolOption(short_option, long_option, desc, val));
}


void WvArgs::add_option(char short_option, WvStringParm long_option,
			WvStringParm desc, NoArgCallback cb, void *ud)
{
    data->remove(short_option, long_option);
    data->add(new WvArgsNoArgCallbackOption(short_option, long_option, desc,
					    cb, ud));
}

void WvArgs::add_option(char short_option, WvStringParm long_option,
			WvStringParm desc, WvStringParm arg_desc, int &val)
{
    data->remove(short_option, long_option);
    data->add(new WvArgsIntOption(short_option, long_option, desc, arg_desc,
				  val));
}

void WvArgs::add_option(char short_option, WvStringParm long_option,
			WvStringParm desc, WvStringParm arg_desc, long &val)
{
    data->remove(short_option, long_option);
    data->add(new WvArgsLongOption(short_option, long_option, desc, arg_desc,
				   val));
}

void WvArgs::add_option(char short_option, WvStringParm long_option,
			WvStringParm desc, WvStringParm arg_desc, float &val)
{
    data->remove(short_option, long_option);
    data->add(new WvArgsFloatOption(short_option, long_option, desc, arg_desc,
				    val));
}

void WvArgs::add_option(char short_option, WvStringParm long_option,
			WvStringParm desc, WvStringParm arg_desc, double &val)
{
    data->remove(short_option, long_option);
    data->add(new WvArgsDoubleOption(short_option, long_option, desc,
				     arg_desc, val));
}

void WvArgs::add_option(char short_option, WvStringParm long_option,
			WvStringParm desc, WvStringParm arg_desc,
			WvString &val)
{
    data->remove(short_option, long_option);
    data->add(new WvArgsStringOption(short_option, long_option, desc,
				     arg_desc, val));
}

void WvArgs::add_option(char short_option, WvStringParm long_option,
			WvStringParm desc, WvStringParm arg_desc,
			WvStringList &val)
{
    data->remove(short_option, long_option);
    data->add(new WvArgsStringListAppendOption(short_option, long_option,
					       desc, arg_desc, val));
}

void WvArgs::add_option(char short_option, WvStringParm long_option,
			WvStringParm desc, WvStringParm arg_desc,
			ArgCallback cb, void *ud)
{
    data->remove(short_option, long_option);
    data->add(new WvArgsArgCallbackOption(short_option, long_option, desc,
					  arg_desc, cb, ud));
}


void WvArgs::remove_option(char short_option)
{
    data->remove(short_option, WvString::null);
}


void WvArgs::remove_option(WvStringParm long_option)
{
    data->remove(0, long_option);
}


void WvArgs::remove_all_options()
{
    data->zap();
}


static inline void add_arg_helper(WvArgs *args, WvStringParm desc)
{
}

void WvArgs::add_required_arg(WvStringParm desc, bool multiple)
{
    data->add_required_arg();
    if (!!args_doc)
        args_doc.append(" ", multiple);
    args_doc.append(desc, multiple);
    if (data->maximum_args < LONG_MAX)
	++(data->maximum_args);
}


void WvArgs::add_optional_arg(WvStringParm desc, bool multiple)
{
    // an optional arg is a required arg without the requirement :-)
    add_required_arg(WvString("[%s]", desc));
    data->subtract_required_arg();
    if (multiple)
    {
	args_doc.append("...");
	data->maximum_args = LONG_MAX;
    }
}


bool WvArgs::get_flag(const flags_t flag) const
{
    switch (flag)
    {
    case NO_EXIT_ON_ERRORS:
	return data->flags & ARGP_NO_EXIT;
    default:
	return false;
    }
}


void WvArgs::set_flag(const flags_t flag, const bool value)
{
    printf("set_flag(%d, %d)\n", flag, value);
    unsigned int mask;
    switch (flag)
    {
    case NO_EXIT_ON_ERRORS:
	mask = ARGP_NO_EXIT;
	break;
    default:
	return;
    }

    if (value)
	data->flags |= mask;
    else
	data->flags &= ~mask;

    printf("set_flag(%d, %d) = %d\n", flag, value, data->flags);
}
