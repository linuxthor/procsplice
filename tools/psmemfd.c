#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

// A quick *hack* to run procsplice heapdump into an memfd then 
// execute 'strings' command to get strings of other processes
// for grepping etc without writing files to disk

int main(int argc, char **argv)
{
    int         x;
    char cmd[256];
    char str[256];

    if(argc < 2) 
    {
        printf("[%s] <pid>\n",argv[0]);
        exit(1);
    }

    x = syscall(SYS_memfd_create, "memfd", NULL);    
    sprintf(cmd, "procsplice -hd -p %s -f /proc/%d/fd/%d > /dev/null 2>&1", 
                                                         argv[1], getpid(), x);
    sprintf(str, "strings /proc/%d/fd/%d", getpid(), x);
   
    system(cmd);
    system(str);

    return 0;
}
