
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include "mmprivate.h"
int abc[1024*1024*4] = {1};
int check_dump_enable()
{
	pid_t pid = getpid();
	char fname[PATH_MAX];
    unsigned long writable = 0, total = 0, shared = 0;
    FILE *f = 0;

    sprintf(fname, "/proc/%ld/maps", (long)pid);
    f = fopen(fname, "r");

    if(!f)
	{
		fprintf(stderr, "unable to open pmap\n");
		return 0;
	}
	while(!feof(f)) 
	{
			char buf[PATH_MAX+100], perm[5], dev[6], mapname[PATH_MAX];
			unsigned long begin, end, size, inode, foo;
			int n;

			if(fgets(buf, sizeof(buf), f) == 0)
			    break;
			mapname[0] = '\0';
			sscanf(buf, "%lx-%lx %4s %lx %5s %ld %s", &begin, &end, perm,
				&foo, dev, &inode, mapname);
			if(strstr(mapname, "persistentheap") != NULL)
			{
				printf("Persistent Heap Detected\n");
				return 1;
			}
    }
	return 0;
}

void get_mem_map(unsigned long * mstart, unsigned long * msize, unsigned long *heapsize)
{
	pid_t pid = getpid();
	char fname[PATH_MAX];
    unsigned long writable = 0, total = 0, shared = 0;
    FILE *f = 0;

    sprintf(fname, "/proc/%ld/maps", (long)pid);
    f = fopen(fname, "r");

    int i = 0;
	while(!feof(f)) 
	{
		i += 1;
		char buf[PATH_MAX+100], perm[5], dev[6], mapname[PATH_MAX];
		unsigned long begin, end, size, inode, foo;
		int n;
		if(fgets(buf, sizeof(buf), f) == 0 || i > 5)
			    break;
		mapname[0] = '\0';
		sscanf(buf, "%lx-%lx %4s %lx %5s %ld %s", &begin, &end, perm,
				&foo, dev, &inode, mapname);
		//printf("%s %d %ld %ld\n", mapname, i, begin, end);
		if(i == 1)
		{
			*heapsize = end - begin;
		}
		else if(i == 4)
		{
			*mstart = begin;
		}
		else if (i == 5)
		{
			*msize = end - *mstart;
		}

    }
	
}

int main()
{
	unsigned long bs_start, bs_size, heapsize;
	if(!check_dump_enable())
	{
		fprintf(stderr, "No persistent heap detected! Aborted\n");
		return -1;
	}
	struct mdesc* mdp = (void*)0xf0000000;
	
	char *hello = malloc(8193);
	
	memset(hello, 1, 1024);
	free(hello);
	printf("%p %p %p %p %d\n", mdp->base, hello, mdp->breakval, mdp->top, mdp->headersize);
	
	memset(abc, 1023, 0);
	
	while(1)
		usleep(1000000);
}