/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Routines to generate a stack backtrace automatically when a program
 * crashes.
 */
#include "wvcrash.h"
#include <signal.h>
#include <fcntl.h>
#include <string.h>

// FIXME: this file mostly only works in Linux
#ifdef __linux

# include <execinfo.h>
#include <unistd.h>

static const char *argv0 = "UNKNOWN";
static const char *desc = NULL;

// write a string 'str' to fd
static void wr(int fd, const char *str)
{
    write(fd, str, strlen(str));
}


// convert 'num' to a string and write it to fd.
static void wrn(int fd, int num)
{
    int tmp;
    char c;
    
    if (num < 0)
    {
	wr(fd, "-");
	num = -num;
    } 
    else if (num == 0)
    {
	wr(fd, "0");
	return;
    }
    
    tmp = 0;
    while (num > 0)
    {
	tmp *= 10;
	tmp += num%10;
	num /= 10;
    }
    
    while (tmp > 0)
    {
	c = '0' + (tmp%10);
	write(fd, &c, 1);
	tmp /= 10;
    }
}


void wvcrash_real(int sig, int fd)
{
    static void *trace[64];
    static char *signame = strsignal(sig);
    
    wr(fd, argv0);
    if (desc)
    {
	wr(fd, " (");
	wr(fd, desc);
	wr(fd, ")");
    }
    wr(fd, " dying on signal ");
    wrn(fd, sig);
    if (signame)
    {
	wr(fd, " (");
	wr(fd, signame);
	wr(fd, ")");
    }
    wr(fd, "\n\nBacktrace:\n");
    backtrace_symbols_fd(trace,
		 backtrace(trace, sizeof(trace)/sizeof(trace[0])), fd);
    
    // we want to create a coredump, and the kernel seems to not want to do
    // that if we send ourselves the same signal that we're already in.
    // Whatever... just send a different one :)
    if (sig == SIGABRT)
	sig = SIGBUS;
    else if (sig != 0)
	sig = SIGABRT;
    
    signal(sig, SIG_DFL);
    raise(sig);
}


// Hint: we can't do anything really difficult here, because the program is
// probably really confused.  So we should try to limit this to straight
// kernel syscalls (ie. don't fiddle with FILE* or streams or lists, just
// use straight file descriptors.)
// 
// We fork a subprogram to do the fancy stuff like sending email.
// 
void wvcrash(int sig)
{
    int fds[2];
    pid_t pid;
    
    signal(sig, SIG_DFL);
    wr(2, "\n\nwvcrash: crashing!\n");
    
    if (pipe(fds))
	wvcrash_real(sig, 2); // just use stderr instead
    else
    {
	pid = fork();
	if (pid < 0)
	    wvcrash_real(sig, 2); // just use stderr instead
	else if (pid == 0) // child
	{
	    close(fds[1]);
	    dup2(fds[0], 0); // make stdin read from pipe
	    fcntl(0, F_SETFD, 0);
	    
	    execlp("wvcrash", "wvcrash", NULL);
	    
	    // if we get here, we couldn't exec wvcrash
	    wr(2, "wvcrash: can't exec wvcrash binary "
	       "- writing to wvcrash.txt!\n");
	    execlp("dd", "dd", "of=wvcrash.txt", NULL);
	    
	    wr(2, "wvcrash: can't exec dd to write to wvcrash.txt!\n");
	    _exit(127);
	}
	else if (pid > 0) // parent
	{
	    close(fds[0]);
	    wvcrash_real(sig, fds[1]);
	}
    }
    
    // child (usually)
    _exit(126);
}


void wvcrash_setup(const char *_argv0, const char *_desc)
{
    argv0 = _argv0;
    desc = _desc;
    signal(SIGSEGV, wvcrash);
    signal(SIGBUS,  wvcrash);
    signal(SIGABRT, wvcrash);
    signal(SIGFPE,  wvcrash);
    signal(SIGILL,  wvcrash);
}

#elif defined(_WIN32)

#include <Windows.h>
#include <stdio.h>
#include <dbghelp.h>

static void exception_desc(FILE *file, unsigned exception,
        unsigned data1, unsigned data2)
{

    switch (exception)
    {
        case 0xC0000005:
        {
            switch (data1)
            {
                case 0:
                    fprintf(file,
                            "invalid memory read from address 0x%08X",
                            data2);
                    break;
                case 1:
                    fprintf(file,
                            "invalid memory write to address 0x%08X",
                            data2);
                    break;
                default:
                    fprintf(file,
                            "invalid memory access (unknown type %d) at address 0x%08X",
                            data1, data2);
                    break;
            }
        }
        break;

        case 0xC0000094:
            fprintf(file, "integer division by zero");
            break;

        default:
            fprintf(file, "unknown exception (data1=0x%08X, data2=0x%08X)");
            break;
    }
}

static LONG WINAPI ExceptionFilter( struct _EXCEPTION_POINTERS * pExceptionPointers )
{
    struct ExceptionInfo
    {
        unsigned exception;
        unsigned unknown[2];
        void *ip;
        unsigned more_unknown;
        unsigned data1;
        unsigned data2;
    };
    ExceptionInfo *info = *(ExceptionInfo **)pExceptionPointers;

    // handle a special exception.  Number 3 = forced breakpoint
    // having __asm int 3; in code will cause windows to ask if
    // you want to debug the application nicely.
    if (info->exception==0x80000003)
    {
        fprintf(stderr, "Preparing to debug!\n");
        return EXCEPTION_CONTINUE_SEARCH;
    }
    
    HANDLE hProcess = GetCurrentProcess();
    SymInitialize(hProcess, NULL, TRUE);
    DWORD disp = 0;
    PIMAGEHLP_LINE64 line = new IMAGEHLP_LINE64;
    memset(line, 0, sizeof(IMAGEHLP_LINE64));
    line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    if (!SymGetLineFromAddr64(hProcess, (DWORD64)info->ip, &disp, line))
    {
        delete line;
        line = 0;
    }
    
    fprintf(stderr, "--------------------------------------------------------\n");
    fprintf(stderr, "Exception 0x%08X:\n  ", info->exception);
    exception_desc(stderr, info->exception, info->data1, info->data2);
    fprintf(stderr, "\n  at instruction 0x%08X\n", info->ip);
    if (line)
        fprintf(stderr, "  source file: %s line %d\n", line->FileName, line->LineNumber);
    else
        fprintf(stderr, "  could not determine the source file and line number.\n");
    fprintf(stderr, "--------------------------------------------------------\n");

    if (line)
        delete line;
    SymCleanup(hProcess);
                
    return EXCEPTION_EXECUTE_HANDLER;
}

static bool have_global_exception_handler = false;
void setup_console_crash()
{
    if (!have_global_exception_handler)
    {
        SetUnhandledExceptionFilter(ExceptionFilter);
        have_global_exception_handler = true;
    }
}

void wvcrash(int sig) {}
void wvcrash_setup(const char *_argv0, const char *_desc) {}

#else // Not Linux

void wvcrash(int sig) {}
void wvcrash_setup(const char *_argv0, const char *_desc) {}

#endif // Not Linux
