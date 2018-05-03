
#include "dlmalloc.h"



void *malloc(size_t howmany)
{
    
    return dlmalloc(howmany);
}

void *calloc(size_t a, size_t b)
{
    return dlcalloc(a, b);
}


void free(void *ptr) //Just Proxy, if somebody really wants to free before malloc, screw them
{
    return dlfree(ptr);
}

void* realloc(void* a, size_t howmany)
{
    return dlrealloc(a, howmany);
}

