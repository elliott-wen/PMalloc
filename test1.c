#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <fcntl.h>
#define EHEAP_START_ADDRESS 0xf0000000
#define EHEAP_SIZE 0x10000000 
#define MAGIC_MAGIC 0x08032018
int main()
{
	//char *test = malloc(1024);
	//printf("%p\n", test);
	unsigned int (*get_allocated_space_size)(void);
	unsigned int (*get_gm_state)(void **ptr);
	unsigned int (*get_mparams)(void **ptr);
	get_allocated_space_size = dlsym(RTLD_LOCAL, "mm_get_allocated_space_size");
	get_gm_state = dlsym(RTLD_LOCAL, "mm_get_gm_state");
	get_mparams = dlsym(RTLD_LOCAL, "mm_get_mparams");
	if(get_allocated_space_size == 0 || get_gm_state == 0)
	{
		printf("Not persistent heap enabled exit.\n");
		exit(-1);
	}

	int *data = malloc(1024 * 400);
	memset(data, 1, 1024*4);
	printf("%p\n", data);
	
	void *gm_ptr = 0;
	void *param_ptr = 0;
	unsigned int allocated_num = get_allocated_space_size();
	unsigned int gm_state_size = get_gm_state(&gm_ptr);
	unsigned int gm_param_size = get_mparams(&param_ptr);
	int fd = open("perheap", O_WRONLY);
	unsigned int magic_num = MAGIC_MAGIC;
	write(fd, &magic_num, sizeof(magic_num));
	write(fd, gm_ptr, gm_state_size);
	write(fd, &magic_num, sizeof(magic_num));
	write(fd, param_ptr,  gm_param_size);
	write(fd, &magic_num, sizeof(magic_num));
	write(fd, &allocated_num, sizeof(allocated_num));
	write(fd, &magic_num, sizeof(magic_num));
	write(fd, (void*)EHEAP_START_ADDRESS, allocated_num);
	write(fd, &magic_num, sizeof(magic_num));
	close(fd);
	allocated_num = get_allocated_space_size();
	gm_state_size = get_gm_state(&gm_ptr);
	gm_param_size = get_mparams(&param_ptr);
	printf("%u %u %u %p %p.\n", allocated_num, gm_state_size, gm_param_size, gm_ptr, param_ptr);
	return 0;
}
