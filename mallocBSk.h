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




void* realloc(void* ptr, size_t size);
void* malloc(size_t size); 
void* calloc(size_t nitems, size_t size); 
void free(void* ptr);


//private 
block_t* find_free_block(size_t k_val);
size_t two_exp(size_t value);
size_t two_to_pow(size_t pow);
size_t align_size(size_t size);
block_t* merge_block(block_t* block);
block_t* split_block(block_t* block,size_t req_kval);
block_t* concatonate_block(block_t* block_1,block_t* block_2);
block_t* get_block(void* data_ptr);
block_t* find_buddy(block_t* block);
void add_to_free_list(block_t* block);
void remove_from_free_list(block_t* block);