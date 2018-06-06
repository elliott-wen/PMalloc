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
#define MAXFILEALLOWED 16
struct memfile_cookie {
           char   *buf;        /* Dynamically sized buffer for data */
           size_t  allocated;  /* Size of buf */
           size_t  endpos;     /* Number of characters in buf */
           off_t   offset;     /* Current file offset in buf */
           char *open_filename;
           char mode;
           FILE *fp;
           char int_buf[BUFSIZ];

};
struct memfile_cookie **cookies = NULL;

static void removeCookie(struct memfile_cookie *cookie)
{
  for(int j = 0; j < MAXFILEALLOWED ; j ++)
  {
    if(cookies[j] == cookie)
    {
      cookies[j] = 0;
      break;
    }
  }
}

static int findEmptyCookie()
{
  if(cookies == NULL)
   {
      cookies = dlmalloc(sizeof(struct memfile_cookie*) * MAXFILEALLOWED);
      if(cookies == NULL)
      {
        return -1;
      }
      memset(cookies, 0, sizeof(struct memfile_cookie*) * MAXFILEALLOWED);
   }
   for(int j = 0; j < MAXFILEALLOWED; j ++)
   {
      if(cookies[j] == 0)
      {
        return j;
      }
   }
   return -1;

}

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
           //printf("reading %d\n", cookie->offset);
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
           if(cookie->mode == WRITEMODE && cookie->open_filename != NULL)
           {
              int output_file_hd = open(cookie->open_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRGRP | S_IROTH);
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
           removeCookie(cookie);
           dlfree(cookie);
           return 0;
}

cookie_io_functions_t  z_memfile_func = {
               .read  = z_memfile_read,
               .write = z_memfile_write,
               .seek  = z_memfile_seek,
               .close = z_memfile_close
};




static int _obtain_readable_filechunk_mem(const char *filename, char **_mem, int *_filesize)
{
  int input_file_fd = open(filename, O_RDONLY);
  if(input_file_fd < 0)
  {
    return -1;
  }
  size_t file_size = lseek(input_file_fd, (size_t)0, SEEK_END);
  lseek(input_file_fd, 0, SEEK_SET);
  if(file_size > 16 * 1024 * 1024 && file_size < 0)
  {
      close(input_file_fd);
      return -1;
  }
  char* tmp_buf = dlmalloc(file_size);
  if(tmp_buf == NULL)
  {
      close(input_file_fd);
      return -1;
  }

  int readS = read(input_file_fd, tmp_buf, file_size);
  if(readS != file_size)
  {
      close(input_file_fd);
      dlfree(tmp_buf);
      return -1;
  }

  close(input_file_fd);
  *_filesize = file_size;
  *_mem = tmp_buf;
  return 0;


}




static int _checkfilewritable(const char *filename)
{
    int output_file_hd = open(filename, O_WRONLY | O_CREAT, S_IRWXU | S_IRGRP | S_IROTH);
    if(output_file_hd > 0)
    {
      close(output_file_hd);
      return 0;
    }
    return -1;
}

static FILE* _fopenread(const char *filename, int slot)
{
      FILE *mem_file;
      char *mem;
      int file_size;
      if(_obtain_readable_filechunk_mem(filename, &mem, &file_size) == -1)
      {
        return NULL;
      }
      int filename_len = strlen(filename);
      char* open_filename = dlmalloc(filename_len + 1);
      if(open_filename == NULL)
      {
        dlfree(mem);
        return NULL;
      }

      struct memfile_cookie *mycookie = dlmalloc(sizeof(struct memfile_cookie));
      if(mycookie == NULL)
      {
        dlfree(mem);
        dlfree(open_filename);
        return NULL;
      }
      mycookie->allocated = file_size;
      mycookie->offset = 0;
      mycookie->endpos = file_size;
      mycookie->buf = mem;
      mycookie->open_filename = open_filename;
      memcpy(mycookie->open_filename, filename, filename_len);
      mycookie->open_filename[filename_len] = 0;
      mycookie->mode = READMODE;
      mem_file = fopencookie(mycookie, "rb", z_memfile_func);
      if(mem_file == NULL)
      {
        dlfree(mem);
        dlfree(open_filename);
        dlfree(mycookie);
        return NULL;
      }
      setbuf(mem_file, mycookie->int_buf);     
      mycookie->fp = mem_file;
      cookies[slot] = mycookie;
      return mem_file;
}

static FILE *_fopenwrite(const char *filename, int slot)
{
       FILE* mem_file = NULL;
      if(_checkfilewritable(filename) == -1)
      {
        return NULL;
      }
     
      int filename_len = strlen(filename);
      char* open_filename = dlmalloc(filename_len + 1);
      if(open_filename == NULL)
      {
        return NULL;
      }
         
      char *tmpbuf = dlmalloc(102400);
      if(tmpbuf == NULL)
      {
        dlfree(open_filename);
        return NULL;
      }

      struct memfile_cookie *mycookie = dlmalloc(sizeof(struct memfile_cookie));
      if(mycookie == NULL)
      {
        dlfree(open_filename);
        dlfree(tmpbuf);
        return NULL;
      }

         
              
      mycookie->allocated = 102400;
      mycookie->offset = 0;
      mycookie->endpos = 0;
      mycookie->buf = tmpbuf;
      mycookie->open_filename = open_filename;
      memcpy(mycookie->open_filename, filename, filename_len);
      mycookie->open_filename[filename_len] = 0;
      mycookie->mode = WRITEMODE;
      mem_file = fopencookie(mycookie, "wb", z_memfile_func);
      if(mem_file == NULL)
      {
        dlfree(open_filename);
        dlfree(mycookie);
        return NULL;
      }
      setbuf(mem_file, mycookie->int_buf);
      cookies[slot] = mycookie;
      mycookie->fp = mem_file;
      return mem_file;
}

static int _reloadfile(struct memfile_cookie *cookie)
{
    dlfree(cookie->buf);
    cookie->buf = NULL;
    //printf("reloading %s %d\n", cookie->open_filename, cookie->offset);
    char *newmem;
    int newfilesize;
    if(_obtain_readable_filechunk_mem(cookie->open_filename, &newmem, &newfilesize) == -1)
        return -1;
                
    cookie->buf = newmem;
    cookie->allocated = cookie->endpos = newfilesize;
    return 0;
    
}

FILE* fopen(const char* filename, const char* mode){
   // if(orig_fopen == NULL)
   // {
   // 		orig_fopen = dlsym(RTLD_NEXT, "fopen"); 
   // 		if(orig_fopen == NULL)
   // 		{
   // 			return NULL;
   // 		}
   // }
   int slot = -1;
   slot = findEmptyCookie();
   if(slot == -1)
   {
    return NULL;
   }

   if(strcmp(mode, "r") == 0 || strcmp(mode, "rb") == 0)
   {
      return _fopenread(filename, slot);
   }
   else if(strcmp(mode, "w") == 0 || strcmp(mode, "wb") == 0)
   { 
   		//printf("===>opening %s\n", filename);
      
      return _fopenwrite(filename, slot);

   }
   return NULL; 
}

int pfile_reload_input_file()
{
  if(cookies == NULL) return -1;
  for(int j = 0; j < MAXFILEALLOWED; j ++)
  {
    if(cookies[j] != 0 && cookies[j]->mode == READMODE)
    {
      fflush(cookies[j]->fp);
      //printf("before %d\n", cookies[j]->offset);
      if(_reloadfile(cookies[j]) == -1)
      {

        return -1;
      }
    }
  }
  return 0;
}
#endif