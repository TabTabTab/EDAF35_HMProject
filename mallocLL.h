#ifndef MALLOCLL_H
#define MALLOCLL_H

#define _BSD_SOURCE

#define SBRK_FAIL ((void*) -1)

#include <stdlib.h>
#include <stdbool.h>

typedef struct bnode_t bnode_t;


struct bnode_t{
	size_t size;
	bnode_t* next;
	bnode_t* prev;
	bool is_free;
};


#define BNODE_SIZE (sizeof (bnode_t))
#define MINIMUM_DATA_SIZE (4)


bnode_t* find_bnode(size_t size);
bnode_t* create_bnode(size_t size);

/**
splits the node in case it has space to spare
**/
void split_bnode(bnode_t* bnode,size_t required_size);


bnode_t* merge_bnode(bnode_t* bnode);
bnode_t* merge_bnode_right(bnode_t* bnode);
bnode_t* merge_bnode_left(bnode_t* bnode);

size_t aligned_size(size_t data_size);

void lock_alloc();
void unlock_alloc();
/**
lets the node beleive its the only node in the world, does not modify size
**/
void virginate_bnode(bnode_t* bnode);
void register_node(bnode_t* bnode);
void insert_node(bnode_t* bnode, bnode_t* pred_bnode, bnode_t* succ_bnode);
void* get_data_ptr(bnode_t* bnode);
void* split_get_data_unlock(bnode_t* bnode,size_t size);

bool are_neighbours(bnode_t* potential_left, bnode_t* potential_right);

bnode_t* get_bnode(void* data_ptr);

void* realloc(void *ptr, size_t size);
void* malloc(size_t size);
void* calloc(size_t nitems, size_t size);
void free(void* ptr);

#endif
