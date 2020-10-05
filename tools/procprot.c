#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/ptrace.h> 

void show_help(void)
{
    printf("procprot -m (permission) -p (pid) -i (from) -o (to)\n");
    exit(0); 
}

int main(int argc, char **argv)
{   
           int c, perm, status; 
                     pid_t pid;
  struct user_regs_struct rbkp,
                          regs;  
        u_int64_t minad, maxad,
                        diffad; 

              u_int64_t bupins; 

    static struct option long_options[] =
    {
        {"mem",   optional_argument, NULL, 'm'},
        {"amin",  optional_argument, NULL, 'i'},
        {"amax",  optional_argument, NULL, 'o'},
        {"pid",   optional_argument, NULL, 'p'},
        {NULL, 0, NULL, 0}
    };

    if(argc < 2) 
    {
        show_help();
    } 

    while ((c = getopt_long(argc, argv, "m:i:o:p:",
                                     long_options, NULL)) != -1)
    {
        switch(c)
        {
            case 'm':
               perm = atoi(optarg); 
            break;
            case 'i':
               minad = strtoul(optarg, NULL, 16); 
            break; 
            case 'o':
               maxad = strtoul(optarg, NULL, 16); 
            break; 
            case 'p': 
               pid = atoi(optarg); 
            break; 
        }
    } 

    if(!perm||!minad||!maxad||!pid)
    {
        show_help();
    }

    printf("Attempting attach of process %d\n",pid);
    if((ptrace(PTRACE_ATTACH, pid, NULL, NULL)) < 0)
    {
        printf("Error attaching to process\n");
        exit(1); 
    }

    printf("Waiting for process\n");
    wait(NULL);

    printf("Fetching registers\n");
    if(((ptrace(PTRACE_GETREGS, pid, NULL, &regs)) < 0) ||
       ((ptrace(PTRACE_GETREGS, pid, NULL, &rbkp)) < 0))
    {
        printf("Error fetching registers\n");
        exit(1);
    }

    diffad = (maxad - minad); 

    regs.rax = 10;          // sys_mprotect
    regs.rdi = minad;      
    regs.rsi = diffad; 
    regs.rdx = perm; 

    printf("Setting registers\n");
    if((ptrace(PTRACE_SETREGS, pid, NULL, &regs)) < 0)
    {
        printf("Couldn't set registers\n");
        exit(1); 
    }

    printf("Backup instruction\n");
    if((bupins = ptrace(PTRACE_PEEKTEXT, pid, (void *)regs.rip, NULL)) < 0)
    {
        printf("Failed in backing up instructions at RIP\n");
        exit(1); 
    }

    printf("Inject to process at %p\n",(void *)regs.rip);
    if((ptrace(PTRACE_POKETEXT, pid, (void *)regs.rip, 0x050f)) < 0)
    {
        printf("Couldn't inject into process\n"); 
        exit(1); 
    }

    printf("Stepping\n");
    if((ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL)) < 0) 
    {
        printf("Error making syscall\n");
        exit(1);
    }

    waitpid(pid, &status, 0); 
    if (WIFSTOPPED(status))
    {
        // ok
    }
    else
    {
        printf("Oh nose! \n");
        exit(1); 
    }
    // inspect return here if we want to.. 

    printf("Restoring code\n");
    if((ptrace(PTRACE_POKETEXT, pid, (void *)regs.rip, bupins)) < 0)
    {
        printf("Couldn't restore to process\n"); 
        exit(1); 
    }

    if((ptrace(PTRACE_SETREGS, pid, NULL, &rbkp)) < 0)
    {
        printf("Couldn't restore registers\n");
        exit(1); 
    }

    if((ptrace(PTRACE_DETACH, pid, NULL, NULL)) < 0)
    {
        printf("Couldn't detach..");
        exit(1); 
    }

    return 0;
}
