#include <string.h>
#include <stdlib.h>
#include "mmalloc.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
static void *md = 0;
static int fd = -1;


static void init_memory()
{
	fd = open("heap_memory", O_CREAT | O_RDWR, 0666);
	if(fd <= 0)
	{
		fprintf(stderr, "error to init heap memory file\n");
	 	exit(-1);
	}
	md = mmalloc_attach(fd, (void*) 0xf0000000);
	if(md==0)
	{
	 	fprintf(stderr, "error to init memory\n");
	 	exit(-1);
	}
}


static void close_memory()
{
	mmalloc_detach(md);
}

void *realloc(void *ptr, size_t size) {
	
	return mrealloc(md, ptr, size);
}

void free(void *ptr) {
	
    mfree(md, ptr);
}


void *malloc(size_t size) {
	
    return mmalloc(md, size);
}

void *calloc(size_t num, size_t size) {
	
    return mcalloc(md, num, size);
}



__attribute__((constructor)) void _mstart(void) {
    printf("Advanced heap memory library loaded!\n");
    init_memory();
}

__attribute__((destructor)) void _mend(void) {
    printf("Advanced heap memory library closed!\n");
    close_memory();
}


