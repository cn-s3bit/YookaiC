#ifndef SDLEX_MEMORYPOOL_H
#define SDLEX_MEMORYPOOL_H
#include <stdlib.h>
void * memorypool_malloc(size_t size);
void memorypool_free(void * pt, size_t size);
void memorypool_free_4bytes(void * pt);
#endif
