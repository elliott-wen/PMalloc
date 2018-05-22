#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <fcntl.h>

int main()
{
	char data[128];
	FILE *ftest = fopen("1.txt", "r");
	fread(data, 1, 2, ftest);
	//usleep(10000000);
	int res = fseek(ftest, 0,  SEEK_CUR);
	//int res = 0;
	fread(data + 2, 1, 2, ftest);
	printf("fseek return %d  %c %c %c %c\n", res, data[0], data[1], data[2], data[3]);

	fclose(ftest);
	return 0;
	//char *test = malloc(1024);
	//printf("%p\n", test);
	// unsigned int (*store_heap_file)(char *filename);
	// store_heap_file = dlsym(RTLD_LOCAL, "mm_store_heap_file");
	// if(store_heap_file == 0)
	// {
	// 	printf("Not persistent heap enabled exit.\n");
	// 	exit(-1);
	// }

	// int *data = malloc(1024 * 400);
	// memset(data, 1, 1024*4);
	// printf("%p\n", data);
	// int x = store_heap_file("pheap");
	// if(x == 0)
	// {
	// 	printf("store okay\n");
	// }
	// else
	// {
	// 	printf("It is a bug\n");
	// }
	// return 0;
}
