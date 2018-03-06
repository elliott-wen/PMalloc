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
#define START_ADDRESS   0xf0000000





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
    
    fprintf(stderr, "Persistent Heap Library Loaded\n");
    char *check_point_num = getenv("CHECKPOINT");
	if(check_point_num != 0 && check_point_num[0] != '\0')
	{
		fprintf(stderr, "Load with a checkpoint %s!\n", check_point_num);
		fd = open(check_point_num, O_RDWR, 0666);
		if(fd <= 0)
		{
			fprintf(stderr, "Error when loading checkpoint memory file\n");
		 	exit(-1);
		}
	}
	md = mmalloc_attach(fd, (void*) START_ADDRESS);
	if(md==0)
	{
	 	fprintf(stderr, "error to init memory\n");
	 	exit(-1);
	}
}



