#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define HEAP_START_ADDRESS   0xf0000000


// static void validate_process()
// {
//     char name[1024];
//     int pid = getpid();
//     sprintf(name, "/proc/%d/cmdline",pid);
//     FILE* f = fopen(name,"r");
//     if(f){
//         size_t stmp;
//         stmp = fread(name, sizeof(char), 1024, f);
//         name[1023] = '\0';
//         fclose(f);
//         if (strstr(name, "lua") != NULL) {
//         	return;
//         }
//         else
//         {
//         	fprintf(stderr, "It is not a validated program %s. Exiting.\n", name);
//         }
//     }
//     else
//     {
//     	fprintf(stderr, "Error to validate process name.\n");
//     }
//     exit(-1);
    
// }


// void *realloc(void *ptr, size_t size) {
	
// 	return mrealloc(md, ptr, size);
// }

// void free(void *ptr) {
	
//     mfree(md, ptr);
// }


// void *malloc(size_t size) {
	
//     return mmalloc(md, size);
// }

// void *calloc(size_t num, size_t size) {
	
//     return mcalloc(md, num, size);
// }



__attribute__((constructor)) void _mstart(void) {
    

    fprintf(stderr, "Persistent Heap Library Loaded\n");
    //validate_process();
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



