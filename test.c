#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <fcntl.h>

int main()
{
	//char *test = malloc(1024);
	//printf("%p\n", test);
	unsigned int (*store_heap_file)(char *filename);
	store_heap_file = dlsym(RTLD_LOCAL, "mm_store_heap_file");
	if(store_heap_file == 0)
	{
		printf("Not persistent heap enabled exit.\n");
		exit(-1);
	}

	int *data = malloc(1024 * 400);
	memset(data, 1, 1024*4);
	printf("%p\n", data);
	int x = store_heap_file("pheap");
	if(x == 0)
	{
		printf("store okay\n");
	}
	else
	{
		printf("It is a bug\n");
	}
	return 0;
}
