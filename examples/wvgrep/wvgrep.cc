/* -*- Mode: C++ -*-
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2004 Net Integration Technologies, Inc.
 *
 * A clone of grep(1) that is written entirely in WvStreams
 *
 */

#include "wvstring.h"
#include "wvstringlist.h"
#include "wvargs.h"
#include "wvregex.h"
#include "wvfile.h"


#define VERSION "0.1.0"

static void output_filename(WvStringParm filename,
    	char suffix, bool display_nulls)
{
    wvout->print(!!filename? filename: WvFastString("(standard input)"));
    if (display_nulls)
    	wvout->write("\0", 1);
    else
    	wvout->write(&suffix, 1);
}


static int match(const WvRegex &regex, WvStringParm filename, WvStream *file,
    	bool invert_match, bool display_filename, bool display_line_number,
    	bool display_nulls, bool display_nothing, bool end_on_first_match)
{
    int count = 0;
    int lineno = 0;
    while (file->isok())
    {
    	const char *line = file->blocking_getline(-1);
    	if (line == NULL) break;
    	++lineno;
    	
    	bool result = regex.match(line);
    	if (invert_match) result = !result;
    	
    	if (result)
    	{
    	    ++count;
    	    if (end_on_first_match) return count;
    	}
    	
    	if (!result || display_nothing) continue;
    	
    	if (display_filename)
    	    output_filename(filename, ':', display_nulls);
    	if (display_line_number)
    	    wvout->print("%s:", lineno);
    	wvout->print("%s\n", line);
    }
    return count;
}


int main(int argc, char **argv)
{
    WvArgs args;

    args.set_version("wvgrep (WvStreams grep) " VERSION "\n");
    args.set_email("<" PACKAGE_BUGREPORT ">");

    bool opt_count = false;
    args.add_set_bool_option('c', "count", WvString::null, opt_count);
    
    bool opt_extended_regexp = false;
    args.add_set_bool_option('E', "extended-regexp", WvString::null, opt_extended_regexp);
    
    WvString opt_regexp;
    args.add_option('e', "regexp", WvString::null, WvString::null, opt_regexp);
  
    bool opt_basic_regexp = false;
    args.add_set_bool_option('G', "basic-regexp", WvString::null, opt_basic_regexp);
    
    bool opt_with_filename = false;
    args.add_set_bool_option('H', "with-filename", WvString::null, opt_with_filename);
    
    bool opt_no_filename = false;
    args.add_set_bool_option('h', "no-filename", WvString::null, opt_no_filename);
    
    bool opt_ignore_case = false;
    args.add_set_bool_option('i', "ignore-case", WvString::null, opt_ignore_case);
    args.add_set_bool_option('y', WvString::null, "Synonym for -i", opt_ignore_case);
    
    bool opt_files_without_match = false;
    args.add_set_bool_option('L', "files-without-match", WvString::null, opt_files_without_match);
    
    bool opt_files_with_matches = false;
    args.add_set_bool_option('l', "files-with-matches", WvString::null, opt_files_with_matches);
    
    bool opt_line_number = false;
    args.add_set_bool_option('n', "line-number", WvString::null, opt_line_number);
    
    bool opt_quiet = false;
    args.add_set_bool_option('q', "quiet", WvString::null, opt_quiet);
    args.add_set_bool_option(0, "silent", "Synonym for --quiet", opt_quiet);
    
    bool opt_no_messages = false;
    args.add_set_bool_option('s', "no-message", WvString::null, opt_no_messages);
    
    bool opt_invert_match = false;
    args.add_set_bool_option('v', "invert-match", WvString::null, opt_invert_match);
    
    bool opt_line_regexp = false;
    args.add_set_bool_option('x', "line-regexp", WvString::null, opt_line_regexp);
    
    bool opt_null = false;
    args.add_set_bool_option('Z', "null", WvString::null, opt_null);

    args.add_required_arg("PATTERN");
    args.add_optional_arg("FILE", true);

    args.set_help_header("Search for PATTERN in each FILE or standard input.");
    args.set_help_footer("With no FILE, this program reads standard input.");

    WvStringList remaining_args;    
    args.process(argc, argv, &remaining_args);

    if (!opt_regexp && !remaining_args.isempty())
    	opt_regexp = remaining_args.popstr();

    int cflags = WvFastString(argv[0]) == "egrep"?
    	    WvRegex::EXTENDED: WvRegex::BASIC;
    if (opt_extended_regexp) cflags = WvRegex::EXTENDED;
    if (opt_basic_regexp) cflags = WvRegex::BASIC;
    if (opt_ignore_case) cflags |= WvRegex::ICASE;
    
    WvString regex_str;
    if (opt_line_regexp)
    	regex_str = WvString("^%s$", opt_regexp);
    else regex_str = opt_regexp;
    
    WvRegex regex(regex_str, cflags);
    if (!regex.isok())
    {
    	WvString errstr = regex.errstr();
    	wverr->print("%s: Invalid regular expression", argv[0]); 
   	if (!!errstr) wverr->print(errstr);
   	wverr->write("\n", 1);
   	return 2;
    }
    
    bool display_filename = remaining_args.count() >= 2;
    if (opt_with_filename) display_filename = true;
    if (opt_no_filename) display_filename = false;
    
    if (remaining_args.isempty())
    	remaining_args.append(WvString::null);
    
    bool found_match = false;
    WvStringList::Iter filename(remaining_args);
    for (filename.rewind(); filename.next(); )
    {
    	WvStream *file;
    	if (!!*filename)
	    file = new WvFile(*filename, O_RDONLY);
	else
	    file = wvcon;

    	if (!file->isok())
    	{
    	    if (!opt_no_messages)
    	    	wverr->print("%s: %s: %s\n", argv[0],
    	    	    	*filename, file->errstr());
    	    if (!!*filename) WVRELEASE(file);
    	    continue;
    	}
    	    
    	int count = match(regex, *filename, file,
    	    	opt_invert_match, display_filename, opt_line_number, opt_null,
    	    	opt_count || opt_files_without_match || opt_files_with_matches,
    	    	opt_quiet);
    	
    	if (!!*filename) WVRELEASE(file);
    	
    	if (opt_files_with_matches || opt_files_without_match)
    	{
    	    bool display = opt_files_with_matches? count>0: count==0;
    	    if (display)
    	    	output_filename(*filename, '\n', opt_null);
    	}
    	else if(opt_count)
    	{
    	    if (display_filename)
    	    	output_filename(*filename, ':', opt_null);
     	    wvout->print("%s\n", count);
    	}
    	    	
    	found_match = found_match || count > 0;
    	if (opt_quiet && found_match) break;
    }

    return found_match? 0: 1;
}
