#include "wvsubproc.h"
#include <stdio.h>


int main()
{
    WvSubProc proc;
    
    // ls should die by itself.
    fprintf(stderr, "starting ls...\n");
    proc.start("ls", "ls", "-F", "/", NULL);
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
    
    return 0;
}
