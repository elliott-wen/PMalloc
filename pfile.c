#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <stdlib.h>
#include "dlmalloc.h"
static FILE* (*orig_fopen)(const char *filename, const char* mode) = NULL;
static int (*orig_fclose)(FILE *fp) = NULL;

struct memfile_cookie {
           char   *buf;        /* Dynamically sized buffer for data */
           size_t  allocated;  /* Size of buf */
           size_t  endpos;     /* Number of characters in buf */
           off_t   offset;     /* Current file offset in buf */
           char *write_filename;
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

static int z_memfile_seek(void *c, off64_t *offset, int whence)
{
           off64_t new_offset;
           struct memfile_cookie *cookie = c;

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
           if(cookie->write_filename != NULL) 
           {
              //Start Dumping File To Disk
              //printf("I am now dummping ====> %s %d\n", cookie->write_filename, cookie->offset);
              FILE *newwrite = orig_fopen(cookie->write_filename, "wb");
              if(newwrite != NULL)
              {
                  
                  fwrite(cookie->buf, 1, cookie->offset, newwrite);
                  fclose(newwrite);
              }
              dlfree(cookie->write_filename);
              cookie->write_filename = NULL;
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
   if(orig_fopen == NULL)
   {
   		orig_fopen = dlsym(RTLD_NEXT, "fopen"); 
   		if(orig_fopen == NULL)
   		{
   			return NULL;
   		}
   }

   if(strcmp(mode, "r") == 0 || strcmp(mode, "rb") == 0)
   {
   		/*We directly pump the file into the memory*/
   		FILE* input_file = orig_fopen(filename, mode);
   		FILE* mem_file = NULL;
   		if(input_file != NULL)
   		{
   			fseek(input_file, 0, SEEK_END); // seek to end of file
			  size_t file_size = ftell(input_file);
        fseek(input_file, 0, SEEK_SET);
	   		if(file_size < 16 * 1024 * 1024 && file_size > 0)
	   		{
	   			
          struct memfile_cookie *mycookie = dlmalloc(sizeof(struct memfile_cookie));
	   			char* tmp_buf = dlmalloc(file_size);
	   			if(tmp_buf != NULL && mycookie != NULL)
	   			{
	   				int readS = fread(tmp_buf, 1, file_size, input_file);
	   				if(readS == file_size)
	   				{

                mycookie->allocated = file_size;
                mycookie->offset = 0;
                mycookie->endpos = file_size;
                mycookie->buf = tmp_buf;
                mycookie->write_filename = 0;
                mem_file = fopencookie(mycookie, mode, z_memfile_func);
	   					  if(mem_file != NULL)
                {
                  setbuf(mem_file, mycookie->int_buf);
                }
	   				}
	   			}
	   		
	   		}
	   	
	   		fclose(input_file);

   		}
   		
   		return mem_file;
   }
   else if(strcmp(mode, "w") == 0 || strcmp(mode, "wb") == 0)
   {
   		//printf("===>opening %s\n", filename);
      FILE* output_file = orig_fopen(filename, mode);
      FILE* mem_file = NULL;
   		if(output_file != NULL)
      {
          fclose(output_file);//We dont write now.
          int filename_len = strlen(filename);
          char* write_filename = dlmalloc(filename_len + 1);
          //printf("==>opening %s\n", filename);
          struct memfile_cookie *mycookie = dlmalloc(sizeof(struct memfile_cookie));
          char* tmp_buf = dlmalloc(4096);
          if(tmp_buf != NULL && mycookie != NULL && write_filename != NULL)
          {
              
              mycookie->allocated = 4096;
              mycookie->offset = 0;
              mycookie->endpos = 0;
              mycookie->buf = tmp_buf;
              mycookie->write_filename = write_filename;
              memcpy(mycookie->write_filename, filename, filename_len);
              mycookie->write_filename[filename_len] = 0;
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
