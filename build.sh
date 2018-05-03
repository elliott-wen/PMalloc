echo 'Building PMalloc'
clang -O3 -fPIC -g -DUSE_DL_PREFIX=1 -DUSE_LOCKS=0 -DUSE_OS_MORE_CORE=1  -DMORECORE=osMoreCore -D_GNU_SOURCE=1 -DFOOTERS=1 -DMORECORE_CANNOT_TRIM=1 \
-DHAVE_MREMAP=0 -DHAVE_MMAP=0 -DNO_MALLINFO=1 -DNO_MALLOC_STATS=1 -shared -ldl \
dlmalloc.c dlmalloc.h pmalloc.c pfile.c
echo 'Outputing shared library'
mv a.out libpmalloc.so

