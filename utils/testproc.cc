#include "wvsubproc.h"
#include <stdio.h>
#include <sys/wait.h>


int main()
{
    WvSubProc proc;
    
#if 1
    // ls should die by itself.
    fprintf(stderr, "starting ls...\n");
    proc.start("ls", "ls", "-F", "/", NULL);
    fprintf(stderr, "started (%d/%d)...\n", proc.running, proc.pid);
    proc.wait(10*1000);
    fprintf(stderr, "done (%d/%d)...\n", proc.running, proc.estatus);
    proc.start_again();
    fprintf(stderr, "started (%d/%d)...\n", proc.running, proc.pid);
    proc.wait(10*1000);
    fprintf(stderr, "done (%d/%d)...\n", proc.running, proc.estatus);
    fprintf(stderr, "\n\n");
    
    // sleep should die after getting SIGTERM.
    fprintf(stderr, "starting sleep...\n");
    proc.start("sleep", "sleep", "10", NULL);
    fprintf(stderr, "started (%d/%d)...\n", proc.running, proc.pid);
    proc.wait(2100);
    fprintf(stderr, "wait done (%d/%d)...\n", proc.running, proc.estatus);
    fprintf(stderr, "sending SIGTERM...\n");
    proc.kill(SIGTERM);
    proc.wait(2200);
    fprintf(stderr, "wait done (%d/%d)...\n", proc.running, proc.estatus);
    fprintf(stderr, "\n\n");

    // sleep should die after getting SIGTERM, so proc.stop() shouldn't take
    // any time to run.
    fprintf(stderr, "starting sleep again...\n");
    proc.start("sleep", "sleep", "10", NULL);
    fprintf(stderr, "started (%d/%d)...\n", proc.running, proc.pid);
    proc.wait(0);
    fprintf(stderr, "wait done (%d/%d)...\n", proc.running, proc.estatus);
    fprintf(stderr, "sending SIGTERM...\n");
    proc.stop(2300);
    fprintf(stderr, "stop done (%d/%d)...\n", proc.running, proc.estatus);
    fprintf(stderr, "\n\n");
    
    // bash should refuse to die from SIGTERM, so we have to wait for SIGKILL.
    fprintf(stderr, "starting bash...\n");
    proc.start("bash", "bash", NULL);
    fprintf(stderr, "started (%d/%d)...\n", proc.running, proc.pid);
    proc.wait(2300);
    fprintf(stderr, "wait done (%d/%d)...\n", proc.running, proc.estatus);
    fprintf(stderr, "stopping process...\n");
    proc.stop(2400);
    fprintf(stderr, "stop done (%d/%d)...\n", proc.running, proc.estatus);
    fprintf(stderr, "\n\n");
    
    // a process that backgrounds itself
    fprintf(stderr, "starting bash -c 'sleep 100 &'...\n");
    proc.start("bash", "bash", "-c", "sleep 100 &", NULL);
    fprintf(stderr, "started (%d/%d)...\n", proc.running, proc.pid);
    proc.wait(2300);
    fprintf(stderr, "wait done (%d/%d)...\n", proc.running, proc.estatus);
    fprintf(stderr, "stopping process...\n");
    proc.stop(2400);
    fprintf(stderr, "stop done (%d/%d)...\n", proc.running, proc.estatus);
    fprintf(stderr, "\n\n");
#endif
    
    // a fancier process with subprocesses and helpful debug messages
    fprintf(stderr, "starting ./complex-proc.sh...\n");
    proc.start("./complex-proc.sh", "./complex-prox.sh", "XYZ", NULL);
    fprintf(stderr, "started (%d/%d)...\n", proc.running, proc.pid);
    proc.wait(4000);
    fprintf(stderr, "wait done (%d/%d/%d)...\n",
	    proc.running, proc.estatus, proc.old_pids.count());
    fprintf(stderr, "stopping process...\n");
    proc.stop(2000, false);
    fprintf(stderr, "stop done (%d/%d/%d)...\n",
	    proc.running, proc.estatus, proc.old_pids.count());
    fprintf(stderr, "start again...\n");
    proc.start_again();
    proc.wait(4000);
    fprintf(stderr, "wait done (%d/%d/%d)...\n",
	    proc.running, proc.estatus, proc.old_pids.count());
    fprintf(stderr, "stopping process...\n");
//    proc.stop(10000, true);
    fprintf(stderr, "stop done (%d/%d/%d)...\n",
	    proc.running, proc.estatus, proc.old_pids.count());
    fprintf(stderr, "\n\n");
    
    fprintf(stderr, "Checking for leftover subprocesses...\n");
    pid_t pid;
    int status;
    while ((pid = ::waitpid(-1, &status, 0)) > 0)
	fprintf(stderr, "LEFTOVER!!  pid=%d, status=%d\n", pid, status);
    
    return 0;
}
