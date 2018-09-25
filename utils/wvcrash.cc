/*
 * Worldvisions Weaver Software:
 *   Copyright (C) 1997-2002 Net Integration Technologies, Inc.
 * 
 * Routines to generate a stack backtrace automatically when a program
 * crashes.
 */
#include "wvcrash.h"
#include "wvtask.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#ifndef _WIN32
# include <sys/wait.h>
# include <sys/syscall.h>
#endif

#ifndef WVCRASH_USE_SIGALTSTACK
# define WVCRASH_USE_SIGALTSTACK 1
#endif

// FIXME: this file mostly only works in Linux
#ifdef __linux

# include <execinfo.h>
#include <unistd.h>

#ifdef __USE_GNU
static const char *argv0 = program_invocation_short_name;
#else
static const char *argv0 = "UNKNOWN";
#endif // __USE_GNU

#if WVCRASH_USE_SIGALTSTACK
static const size_t altstack_size = 1048576; // wvstreams can be a pig
static char altstack[altstack_size];
#endif

// Reserve enough buffer for a screenful of programme.
static const int buffer_size = 2048 + wvcrash_ring_buffer_size;

static char desc[buffer_size];

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


// convert 'addr' to hex and write it to fd.
static void wra(int fd, const void *addr)
{
    const unsigned int ptrbitsshift = (sizeof(ptrdiff_t) << 3) - 4;
    char digits[] = "0123456789ABCDEF";

    write(fd, "0x", 2);
    for (int shift=ptrbitsshift; shift>=0; shift-=4)
        write(fd, &digits[(((ptrdiff_t)addr)>>shift)&0xF], 1);
}


static void wvcrash_real(int sig, int fd, pid_t pid)
{
    static void *trace[64];
    static char *signame = strsignal(sig);
    
    wr(fd, argv0);
    if (desc[0])
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
	wr(fd, ")\n");
    }

    // Write out the PID and PPID.
    static char pid_str[32];
    wr(fd, "\nProcess ID: ");
    snprintf(pid_str, sizeof(pid_str), "%d", getpid());
    pid_str[31] = '\0';
    wr(fd, pid_str);
    wr(fd, "\nParent's process ID: ");
    snprintf(pid_str, sizeof(pid_str), "%d", getppid());
    pid_str[31] = '\0';
    wr(fd, pid_str);
    wr(fd, "\n");

#if WVCRASH_USE_SIGALTSTACK
#pragma GCC diagnostic ignored "-Wframe-address"
    // Determine if this has likely been a stack overflow
    const void *last_real_stack_frame;
    for (;;)
    {
        last_real_stack_frame = __builtin_frame_address(0);
        if (last_real_stack_frame == NULL
                || last_real_stack_frame < &altstack[0]
                || last_real_stack_frame >= &altstack[altstack_size])
            break;
        last_real_stack_frame = __builtin_frame_address(1);
        if (last_real_stack_frame == NULL
                || last_real_stack_frame < &altstack[0]
                || last_real_stack_frame >= &altstack[altstack_size])
            break;
        last_real_stack_frame = __builtin_frame_address(2);
        if (last_real_stack_frame == NULL
                || last_real_stack_frame < &altstack[0]
                || last_real_stack_frame >= &altstack[altstack_size])
            break;
        last_real_stack_frame = __builtin_frame_address(3);
        if (last_real_stack_frame == NULL
                || last_real_stack_frame < &altstack[0]
                || last_real_stack_frame >= &altstack[altstack_size])
            break;
        last_real_stack_frame = __builtin_frame_address(4);
        if (last_real_stack_frame == NULL
                || last_real_stack_frame < &altstack[0]
                || last_real_stack_frame >= &altstack[altstack_size])
            break;
        last_real_stack_frame = __builtin_frame_address(5);
        if (last_real_stack_frame == NULL
                || last_real_stack_frame < &altstack[0]
                || last_real_stack_frame >= &altstack[altstack_size])
            break;
        last_real_stack_frame = NULL;
        break;
    }
    if (last_real_stack_frame != NULL)
    {
        wr(fd, "\nLast real stack frame: ");
        wra(fd, last_real_stack_frame);
        const void *top_of_stack = WvTaskMan::current_top_of_stack();
        wr(fd, "\nTop of stack: ");
        wra(fd, top_of_stack);
        size_t stack_size = size_t(top_of_stack) - size_t(last_real_stack_frame);
        wr(fd, "\nStack size: ");
        wrn(fd, int(stack_size));
        size_t stack_size_limit = WvTaskMan::current_stacksize_limit();
        if (stack_size_limit > 0)
        {
            wr(fd, "\nStack size rlimit: ");
            wrn(fd, int(stack_size_limit));
            if (stack_size > stack_size_limit)
                wr(fd, "  DEFINITE STACK OVERFLOW");
            else if (stack_size + 16384 > stack_size_limit)
                wr(fd, "  PROBABLE STACK OVERFLOW");
        }
        wr(fd, "\n");
    }
#endif
                

    // Write out the contents of the ring buffer
    {
        const char *ring;
        bool first = true;
        while ((ring = wvcrash_ring_buffer_get()) != NULL)
        {
            if (first)
            {
                first = false;
                wr(fd, "\nRing buffer:\n");
            }
            wr(fd, ring);
        }
    }
    
    // Write out the assertion message, as logged by __assert*_fail(), if any.
    {
	const char *assert_msg = wvcrash_read_assert();
	if (assert_msg && assert_msg[0])
	{
	    wr(fd, "\nAssert:\n");
	    wr(fd, assert_msg);
	}
    }

    // Write out the note, if any.
    {
	const char *will_msg = wvcrash_read_will();
	if (will_msg && will_msg[0])
	{
	    wr(fd, "\nLast Will and Testament:\n");
	    wr(fd, will_msg);
	    wr(fd, "\n");
	}
    }

    if (WvCrashInfo::in_stream_state != WvCrashInfo::UNUSED
	&& WvCrashInfo::in_stream)
    {
	const char *state = NULL;
	switch (WvCrashInfo::in_stream_state)
	{
	case WvCrashInfo::UNUSED:
	    // Can't possibly get here.
	    break;
	case WvCrashInfo::PRE_SELECT:
	    state = "\nStream in pre_select: ";
	    break;
	case WvCrashInfo::POST_SELECT:
	    state = "\nStream in post_select: ";
	    break;
	case WvCrashInfo::EXECUTE:
	    state = "\nStream in execute: ";
	    break;
	}

	if (state)
	{
	    static char ptr_str[32];
	    snprintf(ptr_str, sizeof(ptr_str), "%p", WvCrashInfo::in_stream);
	    ptr_str[sizeof(ptr_str) - 1] = '\0';

	    wr(fd, state);
	    wr(fd, WvCrashInfo::in_stream_id && WvCrashInfo::in_stream_id[0]
	       ? WvCrashInfo::in_stream_id : "unknown stream");
	    wr(fd, " (");
	    wr(fd, ptr_str);
	    wr(fd, ")\n");
	}
    }

    wr(fd, "\nBacktrace:\n");
    backtrace_symbols_fd(trace,
		 backtrace(trace, sizeof(trace)/sizeof(trace[0])), fd);
    
    if (pid > 0)
    {
        // Wait up to 10 seconds for child to write wvcrash file in case there
        // is limited space availible on the device; wvcrash file is more
        // useful than core dump
        int i;
        struct timespec ts = { 0, 100*1000*1000 };
        close(fd);
        for (i=0; i < 100; ++i)
        {
            if (waitpid(pid, NULL, WNOHANG) == pid)
                break;
            nanosleep(&ts, NULL);
        }
    }

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
    
    // close some fds, just in case the reason we're crashing is fd
    // exhaustion!  Otherwise we won't be able to create our pipe to a
    // subprocess.  Probably only closing two fds is possible, but the
    // subproc could get confused if all the fds are non-close-on-exec and
    // it needs to open a few files.
    // 
    // Don't close fd 0, 1, or 2, however, since those might be useful to
    // the child wvcrash script.  Also, let's skip 3 and 4, in case someone
    // uses them for something.  But don't close fd numbers that are *too*
    // big; if someone ulimits the number of fds we can use, and *that's*
    // why we're crashing, there's no guarantee that high fd numbers are in
    // use even if we've run out.
    for (int count = 5; count < 15; count++)
	close(count);
    
    if (pipe(fds))
	wvcrash_real(sig, 2, 0); // just use stderr instead
    else
    {
	pid = fork();
	if (pid < 0)
	    wvcrash_real(sig, 2, 0); // just use stderr instead
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
	    wvcrash_real(sig, fds[1], pid);
	}
    }
    
    // child (usually)
    _exit(126);
}


static void wvcrash_setup_alt_stack()
{
#if WVCRASH_USE_SIGALTSTACK
    stack_t ss;
    
    ss.ss_sp = altstack;
    ss.ss_flags = 0;
    ss.ss_size = altstack_size;
    
    if (ss.ss_sp == NULL || sigaltstack(&ss, NULL))
        fprintf(stderr, "Failed to setup sigaltstack for wvcrash: %s\n",
                strerror(errno)); 
#endif //WVCRASH_USE_SIGALTSTACK
}

void wvcrash_add_signal(int sig)
{
#if WVCRASH_USE_SIGALTSTACK
    struct sigaction act;
    
    memset(&act,0,sizeof(act));
    act.sa_handler = wvcrash;
    sigfillset(&act.sa_mask);
    act.sa_flags = SA_ONSTACK | SA_RESTART;
    
    if (sigaction(sig, &act, NULL))
        fprintf(stderr, "Failed to setup wvcrash handler for signal %d: %s\n",
                sig, strerror(errno));
#else //!WVCRASH_USE_SIGALTSTACK
    signal(sig, wvcrash);
#endif //WVCRASH_USE_SIGALTSTACK
}

// Secret symbol for initialising the will and assert buffers
extern void __wvcrash_init_buffers(const char *program_name);

void wvcrash_setup(const char *_argv0, const char *_desc)
{
    if (_argv0)
	argv0 = basename(_argv0);
    __wvcrash_init_buffers(argv0);
    if (_desc)
    {
	strncpy(desc, _desc, buffer_size);
	desc[buffer_size - 1] = '\0';
    }
    else
	desc[0] = '\0';
    
    wvcrash_setup_alt_stack();
    
    wvcrash_add_signal(SIGSEGV);
    wvcrash_add_signal(SIGBUS);
    wvcrash_add_signal(SIGABRT);
    wvcrash_add_signal(SIGFPE);
    wvcrash_add_signal(SIGILL);
}

#elif defined(_WIN32)

#include <windows.h>
#include <stdio.h>
#include <imagehlp.h>

inline char* last_part(char* in)
{
    int len = strlen(in);
    char* tmp = in+len;
    while (tmp > in)
    {
        if (*tmp == '/' || *tmp == '\\')
            return tmp+1;
        tmp--;
    }
    return in;
}


/**
 * Call this with a thread context to get a nice callstack.  You can get a 
 * thread context either from an exception or by using this code:
 *   CONTEXT ctx;
 *   memset(&ctx, 0, sizeof(CONTEXT));
 *   ctx.ContextFlags = CONTEXT_FULL;
 *   GetThreadContext(hThread, &ctx);
 */
int backtrace(CONTEXT &ctx)
{
    HANDLE hProcess = (HANDLE)GetCurrentProcess();
    HANDLE hThread = (HANDLE)GetCurrentThread();

    SymInitialize(hProcess, NULL, TRUE);

    STACKFRAME sf;
    memset(&sf, 0, sizeof(STACKFRAME));

    sf.AddrPC.Offset = ctx.Eip;
    sf.AddrPC.Mode = AddrModeFlat;
    sf.AddrFrame.Offset = ctx.Ebp;
    sf.AddrFrame.Mode = AddrModeFlat;
    sf.AddrStack.Offset = ctx.Esp;
    sf.AddrStack.Mode = AddrModeFlat;

    fprintf(stderr, "Generating stack trace......\n");
    fprintf(stderr, "%3s  %16s:%-10s %32s:%3s %s\n", "Num", "Module", "Addr", "Filename", "Line", "Function Name");
    int i = 0;
    while (StackWalk(IMAGE_FILE_MACHINE_I386,
        hProcess,
        hThread,
        &sf,
        &ctx,
        NULL,
        SymFunctionTableAccess,
        SymGetModuleBase,
        NULL))
    {
        if (sf.AddrPC.Offset == 0)
            break;

        // info about module
        IMAGEHLP_MODULE modinfo;
        memset(&modinfo, 0, sizeof(IMAGEHLP_MODULE));
        modinfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE);
        SymGetModuleInfo(hProcess, sf.AddrPC.Offset, &modinfo);

        // get some symbols
        BYTE buffer[1024];
        DWORD disp = 0;
        memset(buffer, 0, sizeof(buffer));
        PIMAGEHLP_SYMBOL sym = (PIMAGEHLP_SYMBOL)buffer;
        sym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        sym->MaxNameLength = sizeof(buffer) - sizeof(IMAGEHLP_SYMBOL) + 1;
        SymGetSymFromAddr(hProcess, sf.AddrPC.Offset, &disp, sym);

        // line numbers anyone?
        IMAGEHLP_LINE line;
        SymSetOptions(SYMOPT_LOAD_LINES);
        DWORD disp2 = 0;
        memset(&line, 0, sizeof(IMAGEHLP_LINE));
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE);
        SymGetLineFromAddr(hProcess, sf.AddrPC.Offset, &disp2, &line);

        // output some info now then
        fprintf(stderr, "%3d. %16s:0x%08X %32s:%-3d %s\n",
                ++i,
                modinfo.LoadedImageName[0]?modinfo.LoadedImageName:"unknown",
                (DWORD)sf.AddrPC.Offset,
                (line.FileName && line.FileName[0])?last_part(line.FileName):"unknown",
                (line.FileName && line.FileName[0])?line.LineNumber:0,
                sym->Name[0]?sym->Name:"unknown");
    }

    SymCleanup(hProcess);

    return 1;
}


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
    
    fprintf(stderr, "--------------------------------------------------------\n");
    fprintf(stderr, "Exception 0x%08X:\n  ", info->exception);
    exception_desc(stderr, info->exception, info->data1, info->data2);
    fprintf(stderr, "\n  at instruction 0x%08X in thread 0x%08X\n", info->ip, GetCurrentThreadId());
    backtrace(*pExceptionPointers->ContextRecord);
    fprintf(stderr, "--------------------------------------------------------\n");

                
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
void wvcrash_add_signal(int sig) {}
void wvcrash_setup(const char *_argv0, const char *_desc) {}

#endif // Not Linux
