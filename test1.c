#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dlfcn.h>
#define EHEAP_START_ADDRESS 0xf0000000
#define EHEAP_SIZE 0x10000000 
#define MAGIC_MAGIC 0x08032018
int main()
{
	char *test = malloc(1024);
	printf("%p\n", test);
	// unsigned int (*get_allocated_space_size)(void);
	// unsigned int (*get_gm_state)(void **ptr);
	// get_allocated_space_size = dlsym(RTLD_LOCAL, "mm_get_allocated_space_size");
	// get_gm_state = dlsym(RTLD_LOCAL, "mm_get_gm_state");
	// if(get_allocated_space_size == 0 || get_gm_state == 0)
	// {
	// 	printf("Not persistent heap enabled exit.\n");
	// 	exit(-1);
	// }

	// int *data = malloc(1024 * 400);
	// memset(data, 1, 1024*4);
	// printf("%p\n", data);
	
	// void *gm_ptr = 0;
	// unsigned int allocated_num = get_allocated_space_size();
	// unsigned int gm_state_size = get_gm_state(&gm_ptr);
	// printf("%u %u %p.\n", allocated_num, gm_state_size, gm_ptr);
	// FILE *perheap = fopen("perheap", "w");
	// unsigned int magic_num = MAGIC_MAGIC;
	// fwrite(&magic_num, sizeof(magic_num), 1, perheap);
	// fwrite(gm_ptr, 1, gm_state_size, perheap);
	// fwrite(&magic_num, sizeof(magic_num), 1, perheap);
	// fwrite(&allocated_num, sizeof(allocated_num), 1, perheap);
	// fwrite(&magic_num, sizeof(magic_num), 1, perheap);
	// fwrite((void*)EHEAP_START_ADDRESS,1, allocated_num, perheap);
	// fwrite(&magic_num, sizeof(magic_num), 1, perheap);
	// fclose(perheap);
	return 0;
}
