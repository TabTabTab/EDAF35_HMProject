#define _BSD_SOURCE

#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


#define N (23)
#define POOL_SIZE (1<<N)		//(8*1024*1024)

#define SBRK_FAIL ((void*) -1)

typedef struct block_t block_t;

struct block_t{
	bool reserved; /* one if reserved. */
	char kval; /* current value of K. */
	block_t* succ; /* successor block in list. */
	block_t* pred; /* predecessor block in list. */
};

#define BLOCK_SIZE (sizeof(block_t))

block_t* find_free_block(size_t size);



void* realloc(void* ptr, size_t size);
void* malloc(size_t size); 
void* calloc(size_t nitems, size_t size); 
void free(void* ptr);


//private 
size_t two_exp(size_t value);
size_t two_to_pow(size_t pow);
block_t* split_block(block_t* block,size_t req_kval);