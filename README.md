HEAP Persistent Memory Allocation (PMalloc)

In short, this is a library designed for application checkpoint/restore, of which one cruical step is to dumping/reinstall the memory of the data/bss segments and the heap. Dumping is relatively easy (e.g., using fwrite to save the contents to a file) while reinstalling is a bit tricky due to the heap address difference between each run of an application. 

More specifically, traditional heap allocation relies on the system-built in functions (i.e,m malloc/calloc/realloc/free). These functions mainly utilizes sbrk system call to obtain memory from the kernel. Nevertheless, the addresses the sbrk returns can vary drastically depending on many factors (e.g., whether the kernel enables address randomization and how much memory is requested, etc). It poses an obstacle to reinstall the memory when restoing an application, i.e., how to persist the heap state of each run, such that pointers in data/bss segments can be still vaild.

A great amount of research has been done to solve this issue. A promising solution is CRIU [I dont have citation], however it  requires the kernel to have certain features [still I dont have citation please google it]. Another one is mmalloc [Citation missing], which uses mmap rather than sbrk to obtain heap memory from the system, because the mmap system call allows applications to specicfy the heap address wanted. Nevertheless, mmalloc is a very old, unmaintained, lack-of-testing, and unfamous library, which tends to buggy.

This repository provides another solution, named Pmalloc. The proposed library is built on top of the famous, stable and efficient dlmalloc. 
