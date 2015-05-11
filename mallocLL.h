#ifndef MALLOCLL_H
#define MALLOCLL_H

#define _BSD_SOURCE

#include <stdlib.h>
#include <stdbool.h>

typedef struct bnode_t bnode_t;


struct bnode_t{
	size_t size;
	bnode_t* next;
	bnode_t* prev;
	bool is_freed;
};

bnode_t* find_bnode(size_t size);

void* realloc(void *ptr, size_t size);
void* malloc(size_t size);
void* calloc(size_t nitems, size_t size); 
void free(void* ptr);

#endif