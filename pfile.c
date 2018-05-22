#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "dlmalloc.h"

#ifdef PERSISTENT_FILE
#define WRITEMODE 0
#define READMODE 1
struct memfile_cookie {
           char   *buf;        /* Dynamically sized buffer for data */
           size_t  allocated;  /* Size of buf */
           size_t  endpos;     /* Number of characters in buf */
           off_t   offset;     /* Current file offset in buf */
           char *open_filename;
           char mode;
           char int_buf[BUFSIZ];

};

static ssize_t z_memfile_write(void *c, const char *buf, size_t size)
{
           char *new_buff;
           struct memfile_cookie *cookie = c;

           /* Buffer too small? Keep doubling size until big enough */

           while (size + cookie->offset > cookie->allocated) {
               if(cookie->allocated > 8 * 1024 * 1024)
               {
                  return -1;
               }
               new_buff = dlrealloc(cookie->buf, cookie->allocated * 2);
               if (new_buff == NULL) {
                   return -1;
               } else {
                   cookie->allocated *= 2;
                   cookie->buf = new_buff;
               }
           }

           memcpy(cookie->buf + cookie->offset, buf, size);

           cookie->offset += size;
           if (cookie->offset > cookie->endpos)
               cookie->endpos = cookie->offset;

           return size;
}

static ssize_t z_memfile_read(void *c, char *buf, size_t size)
{
           ssize_t xbytes;
           struct memfile_cookie *cookie = c;

           /* Fetch minimum of bytes requested and bytes available */

           xbytes = size;
           if (cookie->offset + size > cookie->endpos)
               xbytes = cookie->endpos - cookie->offset;
           if (xbytes < 0)     /* offset may be past endpos */
              xbytes = 0;

           memcpy(buf, cookie->buf + cookie->offset, xbytes);

           cookie->offset += xbytes;
           return xbytes;
}


// static int _reload(void *c)
// {
//                 struct memfile_cookie *cookie = c;
//                 dlfree(cookie->buf);
//                 cookie->buf = NULL;
//                 cookie->allocated = 0;
//                 //Pump New File to the memory
//                 int input_file_fd = open(cookie->open_filename, O_RDONLY);
//                 if(input_file_fd > 0)
//                 {
//                     size_t file_size = lseek(input_file_fd, (size_t)0, SEEK_END);
//                     lseek(input_file_fd, 0, SEEK_SET);
//                     if(file_size < 16 * 1024 * 1024 && file_size > 0)
//                     {
//                       char* tmp_buf = dlmalloc(file_size);
//                       if(tmp_buf != NULL)
//                       {
//                         cookie->allocated = file_size;
//                         cookie->endpos = file_size;
//                         cookie->buf = tmp_buf;
//                         return 0;
//                       }
//                       else
//                       {
//                         return -1;
//                       }
//                     }
//                     else
//                     {
//                       return -1;
//                     }
//                 }
//                 else
//                 {
//                   return -1;
//                 }
// }

static int z_memfile_seek(void *c, off64_t *offset, int whence)
{
           off64_t new_offset;
           struct memfile_cookie *cookie = c;
           // if(cookie->mode == READMODE)
           // {
           //    if(_reload(c) != 0)
           //    {
           //      return -1;
           //    }
           // }
           if (whence == SEEK_SET)
                new_offset = *offset;         
           else if (whence == SEEK_END)
               new_offset = cookie->endpos + *offset;
           else if (whence == SEEK_CUR)
               new_offset = cookie->offset + *offset;
           else
              return -1;
           
                

           if (new_offset < 0)
               return -1;

           cookie->offset = new_offset;
           *offset = new_offset;
           return 0;
}

static int z_memfile_close(void *c)
{
           struct memfile_cookie *cookie = c;
           if(cookie->mode == WRITEMODE && cookie->open_filename != NULL)
           {
              int output_file_hd = open(cookie->open_filename, O_WRONLY | O_CREAT, S_IRWXU | S_IRGRP | S_IROTH);
              if(output_file_hd > 0)
              {
                  write(output_file_hd, cookie->buf, cookie->offset);
                  close(output_file_hd);
              }
           }

           if(cookie->open_filename != NULL) 
           {
              //Start Dumping File To Disk
              //printf("I am now dummping ====> %s %d\n", cookie->open_filename, cookie->offset);
              
              dlfree(cookie->open_filename);
              cookie->open_filename = NULL;
           }
           dlfree(cookie->buf);
           cookie->allocated = 0;
           cookie->buf = NULL;

           dlfree(cookie);
           return 0;
}

cookie_io_functions_t  z_memfile_func = {
               .read  = z_memfile_read,
               .write = z_memfile_write,
               .seek  = z_memfile_seek,
               .close = z_memfile_close
};



FILE* fopen(const char* filename, const char* mode){
   // if(orig_fopen == NULL)
   // {
   // 		orig_fopen = dlsym(RTLD_NEXT, "fopen"); 
   // 		if(orig_fopen == NULL)
   // 		{
   // 			return NULL;
   // 		}
   // }

   if(strcmp(mode, "r") == 0 || strcmp(mode, "rb") == 0)
   {
   		/*We directly pump the file into the memory*/
   		int input_file_fd = open(filename, O_RDONLY);
   		FILE* mem_file = NULL;
   		if(input_file_fd > 0)
   		{
        int filename_len = strlen(filename);
        char* open_filename = dlmalloc(filename_len + 1);
   			size_t file_size = lseek(input_file_fd, (size_t)0, SEEK_END);
			  lseek(input_file_fd, 0, SEEK_SET);
	   		if(file_size < 16 * 1024 * 1024 && file_size > 0)
	   		{
	   			
          struct memfile_cookie *mycookie = dlmalloc(sizeof(struct memfile_cookie));
	   			char* tmp_buf = dlmalloc(file_size);
	   			if(tmp_buf != NULL && mycookie != NULL)
	   			{
	   				int readS = read(input_file_fd, tmp_buf, file_size);
	   				if(readS == file_size)
	   				{

                mycookie->allocated = file_size;
                mycookie->offset = 0;
                mycookie->endpos = file_size;
                mycookie->buf = tmp_buf;
                mycookie->open_filename = open_filename;
                memcpy(mycookie->open_filename, filename, filename_len);
                mycookie->open_filename[filename_len] = 0;
                mycookie->mode = READMODE;
                mem_file = fopencookie(mycookie, mode, z_memfile_func);
	   					  if(mem_file != NULL)
                {
                  setbuf(mem_file, mycookie->int_buf);
                }
	   				}
	   			}
	   		
	   		}
	   	
	   		close(input_file_fd);

   		}
   		
   		return mem_file;
   }
   else if(strcmp(mode, "w") == 0 || strcmp(mode, "wb") == 0)
   {
   		//printf("===>opening %s\n", filename);
      int output_file_hd = open(filename, O_WRONLY | O_CREAT, S_IRWXU | S_IRGRP | S_IROTH);
      FILE* mem_file = NULL;
   		if(output_file_hd > 0)
      {
          close(output_file_hd);//We dont write now.
          int filename_len = strlen(filename);
          char* open_filename = dlmalloc(filename_len + 1);
          //printf("==>opening %s\n", filename);
          struct memfile_cookie *mycookie = dlmalloc(sizeof(struct memfile_cookie));
          char* tmp_buf = dlmalloc(4096);
          if(tmp_buf != NULL && mycookie != NULL && open_filename != NULL)
          {
              
              mycookie->allocated = 4096;
              mycookie->offset = 0;
              mycookie->endpos = 0;
              mycookie->buf = tmp_buf;
              mycookie->open_filename = open_filename;
              memcpy(mycookie->open_filename, filename, filename_len);
              mycookie->open_filename[filename_len] = 0;
              mycookie->mode = WRITEMODE;
              mem_file = fopencookie(mycookie, mode, z_memfile_func);
              if(mem_file != NULL)
              {
                  setbuf(mem_file, mycookie->int_buf);
              }
          }
      }
      return mem_file;

   }
   return NULL;
}
#endif