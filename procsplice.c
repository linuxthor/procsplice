/* 
 * procsplice
 *
 * Copyright (c) 2020 linuxthor.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3. You must call me Uncle. 
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/uio.h>

void show_help(void)
{
    printf("\nUsage:\n"); 
    printf(" procsplice [options]\n\n");   
    printf(" e.g 'procsplice --heap dump -p $$ -f /tmp/heap_$$'\n");
    printf("    ('procsplice -hd -p $$ -f /tmp/heap_$$')\n\n");
    printf(" e.g 'procsplice -hl -p 1234 -f /tmp/heap'\n");
    printf("     'procsplice -sd -p 1234 -f /tmp/stack'\n");
    printf("     'procsplice -cd -p 1234 -f /tmp/code'\n\n");
    exit(0);
}

int get_and_save(unsigned long from, unsigned long to, pid_t pid, FILE *f)
{
    struct iovec  local[1];
    struct iovec remote[1];
    unsigned char     *buf;
    ssize_t    take, nread;

    take = (to - from);
    buf  = malloc(take);
 
    if(buf) 
    {
        local[0].iov_base  =  buf;
        local[0].iov_len   =  take;
        remote[0].iov_base =  (void *) from;
        remote[0].iov_len  =  take;

        nread = process_vm_readv(pid, local, 1, remote, 1, 0);
        if (nread != take)
        {
            printf("Warning - Expected %ld bytes but got %ld\n", take, nread);
        }

        if(nread > 0)  
        {
            fwrite(buf, 1, take, f);
        }
        free(buf);
    }
    return 0;
}

int read_and_put(unsigned long from, unsigned long to, pid_t pid, FILE *f)
{
    struct iovec  local[1];
    struct iovec remote[1];
    unsigned char     *buf;
    ssize_t    sizer, nwri;

    sizer = (to - from); 
    buf = malloc(sizer); 

    if(buf)
    {
        if (fread(buf, 1, sizer, f) == sizer)  
        {
            local[0].iov_base  = buf;
            local[0].iov_len   = sizer;
            remote[0].iov_base = (void *) from;  
            remote[0].iov_len  = sizer; 

            nwri = process_vm_writev(pid, local, 1, remote, 1, 0); 
 
            if(nwri != sizer)
            {
                printf("Warning - Expected %ld but wrote %ld bytes instead\n", 
                                                                  sizer, nwri); 
            }
            return 0; 
        }
        else 
        {
            printf("Error - Was asked to load %ld bytes but read fewer bytes\n", 
                                                                         sizer);
            exit(1); 
        }
    }     
    return 0;
}

int main(int argc, char **argv)
{
    char         fname[64],
                 reid[512],
                  flags[6],
                   nam[64],
                  cexe[64],
                  cnam[64];

    char             *fnam;

    pid_t              pid;
    unsigned long from, to,
              pgoff, major,
                     minor;

    unsigned      long ino;
    FILE            *f,*uf;
    char *stak = "[stack]";
    char *heep =  "[heap]";
    char *rxp  =    "r-xp";

    unsigned long amin,  
                      amax;

    // options.. 
                          int c,hd,hl,sd,sl,cd,
                                      cl,ad,al;
    static struct option long_options[] =
    {
        {"heap",  optional_argument, NULL, 'h'},
        {"stack", optional_argument, NULL, 's'},
        {"code",  optional_argument, NULL, 'c'},
        {"misc",  optional_argument, NULL, 'm'},
        {"amin",  optional_argument, NULL, 'i'},
        {"amax",  optional_argument, NULL, 'o'},
        {"pid",   optional_argument, NULL, 'p'},
        {"file",  optional_argument, NULL, 'f'},
        {NULL, 0, NULL, 0}
    };

    if(argc < 2) 
    {
        show_help();
    }

    while ((c = getopt_long(argc, argv, "h:s:c:m:i:o:p:f:", 
                                     long_options, NULL)) != -1)
    {
        switch (c)
        {
            case 'h':     
                if(optarg[0] == 'd')
                {
                    hd = 1;   // dump process heap to a file
                }
                if(optarg[0] == 'l')
                {
                    hl = 1;   // load file to heap
                }
           break;
           case 's':   
               if(optarg[0] == 'd')
               {
                   sd = 1;   // dump process stack to a file
               }
               if(optarg[0] == 'l')
               {
                   sl = 1; 
               }
           break;
           case 'c':    
               if(optarg[0] == 'd')
               {
                   cd = 1;   // dump process code to a file
               }
               if(optarg[0] == 'l')
               {
                   cl = 1;   
                   printf("Warning - this may fail if memory protection\n");
                   printf("doesn't allow writing - see tools directory \n");
                   printf("for a (hacky) way to force this..\n\n");
                   exit(1); 
               }
           break; 
           case 'a':    
               if(optarg[0] == 'd')
               { 
                   ad = 1;   // dump arbitrary process memory to a file
               }
               if(optarg[0] == 'l')
               {
                   al = 1;   // load file to arbitrary address
               }
           break;
           case 'i':    
               amin = strtoul(optarg, NULL, 16); 
           break;
           case 'o':    
               amax = strtoul(optarg, NULL, 16); 
           break;
           case 'p':     
               pid =  atoi(optarg);     
           break;
           case 'f':            
               fnam = (char *)optarg;
           break;
        }
    }

    if(!pid)
    {
        printf("[%s] You must supply a PID\n", argv[0]);
        exit(1); 
    }

    sprintf(cexe, "/proc/%d/exe", pid); 
    if(readlink(cexe, cnam, 64) == 0)
    {
        printf("There was a problem resolving PID to path\n");
        exit(1); 
    } 

    if(!hd && !sd && !cd && !ad && !hl && !sl && !cl && !al)
    {
        printf("[%s] You must choose an action to perform\n", argv[0]);
        exit(1); 
    }
   
    if((ad || al) && (!amin && !amax))
    {
        printf("[%s] Arbitrary dump/load requires addresses\n", argv[0]);
        exit(1); 
    }  
 
    if((sd || hd || cd || ad || sl || hl || cl || al) && !fnam)
    {
        printf("[%s] You must supply a filename\n", argv[0]);
        exit(1);
    }

    if(sd || hd || cd || ad)
    {
         uf = fopen(fnam, "wb+");
    }
    if(sl || hl || cl || al)
    {
        uf = fopen(fnam, "rb+");
    }
    if(uf == NULL)
    {
        printf("Error opening %s\n", fnam);
    }

    sprintf(fname, "/proc/%d/maps", pid);
    f = fopen(fname,   "rb");

    if(f == NULL)
    {
        printf("Error opening /proc/%d/maps\n", pid);
        exit(1);
    }

    if(ad)
    {
        printf("Saving range to file %s\n", fnam);
        get_and_save(amin, amax, pid, uf); 
        exit(0); 
    }

    if(al)
    {
        printf("Loading range from file %s\n", fnam);
        read_and_put(amin, amax, pid, uf); 
        exit(0);
    }

    while (1)
    {
        if (fgets(reid, 512, f) == NULL) break;
        int ret = sscanf(reid, "%lx-%lx %4c %lx %lx:%lx %lu %s", &from, &to, flags,
                                                 &pgoff, &major, &minor, &ino, nam);
        if (ret != 8)
        {
            continue;
        }

        if (strstr(nam, heep) != 0)
        {
            if(hd == 1)
            {
                printf("Saving heap to file %s\n", fnam);
                get_and_save(from, to, pid, uf);
                exit(0);
            }
            if(hl == 1)
            {
                printf("Loading heap from file %s\n", fnam);
                read_and_put(from, to, pid, uf);
                exit(0);
            }
        }

        if (strstr(nam, stak) != 0)
        {
            if(sd == 1)
            {
                printf("Saving stack to file %s\n", fnam);
                get_and_save(from, to, pid, uf);
                exit(0);
            }
            if(sl == 1)
            {
                printf("Loading stack from file %s\n", fnam);
                read_and_put(from, to, pid, uf);
                exit(0);
            }
        }
        if ((strcmp(flags, rxp) == 0) && (strcmp(nam, cnam) == 0))
        {
            if(cd == 1)
            {
                printf("Saving code to file %s\n", fnam);
                get_and_save(from, to, pid, uf); 
                exit(0);
            }
            if(cl == 1)
            {
                printf("Restoring code from file %s\n", fnam);
                read_and_put(from, to, pid, uf);
                exit(0); 
            }
        }
    }
    fclose(uf);
    return 0;
}
