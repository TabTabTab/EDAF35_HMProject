#include "mallocLL.h"

#include <pthread.h>

#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#include  <sys/types.h>

static bnode_t* current_head=NULL;
pthread_mutex_t alloc_mutex;


void lock_alloc()
{
	pthread_mutex_lock (&alloc_mutex);
}
void unlock_alloc()
{
	pthread_mutex_unlock (&alloc_mutex);

   	//pthread_exit((void*) 0);
}

void free(void* ptr)
{
	printf("calling custom function: %s\n", __FUNCTION__);
	if(ptr==NULL){
		return;
	}
	lock_alloc();

	bnode_t* bnode=get_bnode(ptr);
	bnode->is_free=true;
	merge_bnode(bnode);

	unlock_alloc();
}

void* malloc(size_t size)
{
	printf("calling custom function: %s\n", __FUNCTION__);
	size=aligned_size(size);
	lock_alloc();
	bnode_t* bnode = find_bnode(size);
	if(bnode==NULL){
		unlock_alloc();
		return NULL;
	}
	//split the node if too large
	split_bnode(bnode,size);

	bnode->is_free=false;
	void* data=get_data_ptr(bnode);
	unlock_alloc();
	return data;
}

void* calloc(size_t nitems, size_t size)
{
	printf("calling custom function: %s\n", __FUNCTION__);
	size=aligned_size(size);
	size_t tot_size=nitems*size;
	void* data=malloc(tot_size);
	if(data!=NULL){
		memset(data,0,tot_size);
	}
	return data;
}

void* realloc(void *ptr, size_t size)
{
	printf("calling custom function: %s\n", __FUNCTION__);
	size=aligned_size(size);
	if(ptr==NULL){
		return malloc(size);
	}
	lock_alloc();
	bnode_t* current=get_bnode(ptr);
	size_t prev_size=current->size;

	if(current->size >size){
		//we don't need to work, but we may be able to split
		return split_get_data_unlock(current,size);
	}

	//try to expand the current node to the right(in that case no data needs to move)
	current=merge_bnode_right(current);
	if(current->size >size){
		//we don't need to work, but we may be able to split
		return split_get_data_unlock(current,size);
	}
	//try to expand the current node to the left(in that case data needs to move)
	current=merge_bnode_left(current);
	if(current->size >size){
		//we don't need to work, but we may be able to split
		memcpy(get_data_ptr(current), ptr, prev_size);
		return split_get_data_unlock(current,size);
	}
	//in case nothing worked we need to allocate new memory
	unlock_alloc();
	void* new_data=malloc(size);
	if(new_data==NULL){
		return NULL;
	}
	memcpy(new_data, ptr, prev_size);

	free(get_data_ptr(current));
	return new_data;

}

void* split_get_data_unlock(bnode_t* bnode, size_t size)
{
	split_bnode(bnode,size);
	void* data=get_data_ptr(bnode);
	unlock_alloc();
	return data;
}

bnode_t* find_bnode(size_t size)
{
	//this will only happen when no memory has ever been allocated
	if(current_head==NULL){
		current_head=create_bnode(size);

		if(current_head==NULL){
			return NULL;
		}

		virginate_bnode(current_head);
	}

	bnode_t* start_node=current_head;
	do{
		if(current_head->is_free && 
			current_head->size >= size){
			return current_head;
		}
		current_head=current_head->next;
	}while(current_head!=start_node);

	//if no node with the required size was available
	bnode_t* new_node=create_bnode(size);
	if(new_node==NULL){
		return NULL;
	}
	register_node(new_node);
	return new_node;
}

size_t aligned_size(size_t data_size)
{
	size_t aligned_size=(((data_size-1)>>3)<<3)+8;
	return aligned_size;
}

bnode_t* create_bnode(size_t size)
{
	void* request = sbrk(BNODE_SIZE+size);
	if (request==SBRK_FAIL){
		return NULL;
	}
	bnode_t* new_bnode=request;
	new_bnode->size=size;
	return new_bnode;
}

bnode_t* merge_bnode(bnode_t* bnode)
{
	bnode_t* pred=bnode->prev;
	bnode=merge_bnode_left(bnode);
	bnode=merge_bnode_right(bnode);
	return bnode;
}
bnode_t* merge_bnode_left(bnode_t* bnode)
{
	bnode_t* left=bnode->prev;
	if(left->is_free && are_neighbours(left, bnode)){
		left->size = left->size + bnode->size + BNODE_SIZE;
		left->next=bnode->next;
		//now use the left node as current
		bnode=left;
	}
	return bnode;
}
bnode_t* merge_bnode_right(bnode_t* bnode)
{
	bnode_t* right=bnode->next;
	if(right->is_free && are_neighbours(bnode,right)){
		bnode->size = bnode->size + right->size + BNODE_SIZE;
		bnode->next=right->next;
	}
	return bnode;
}

bool are_neighbours(bnode_t* potential_left, bnode_t* potential_right)
{
	char* byte_ptr=(char*)potential_left;
	bnode_t* right=(bnode_t*)(byte_ptr + BNODE_SIZE + potential_left->size);
	if(right==potential_right){
		return true;
	}else{
		return false;
	}
}

void split_bnode(bnode_t* bnode,size_t required_size)
{
	size_t split_treshold=aligned_size(required_size+BNODE_SIZE+MINIMUM_DATA_SIZE);

	if(bnode->size < split_treshold){
		return;
	}
	size_t pred_tot_size=required_size+BNODE_SIZE;

	size_t new_data_size=bnode->size - pred_tot_size;

	bnode->size=required_size;

	char* byte_ptr=(char*)bnode;
	byte_ptr=byte_ptr+pred_tot_size;
	bnode_t* new_bnode=(bnode_t*)byte_ptr;
	new_bnode->size=new_data_size;
	new_bnode->is_free=true;

	insert_node(new_bnode, bnode, bnode->next);
}
void virginate_bnode(bnode_t* bnode)
{
	bnode->next=bnode;
	bnode->prev=bnode;
	bnode->is_free=true;
}

void register_node(bnode_t* new_node)
{
	bnode_t* pred_bnode=current_head->prev;
	bnode_t* succ_bnode=current_head;

	insert_node(new_node,pred_bnode,succ_bnode);
}
void insert_node(bnode_t* bnode, bnode_t* pred_bnode, bnode_t* succ_bnode)
{
	//inform neighbours
	pred_bnode->next=bnode;
	succ_bnode->prev=bnode;	

	//inform self
	bnode->next=succ_bnode;
	bnode->prev=pred_bnode;
}
void* get_data_ptr(bnode_t* bnode)
{
	return bnode+1;
}

bnode_t* get_bnode(void* data_ptr)
{
	bnode_t* bnode=((bnode_t*)data_ptr)-1;
	return bnode;
}

int main()
{
	printf("alsize %d\n",aligned_size(14));

	//return 0;
	unsigned* mem=(unsigned*)malloc(sizeof (unsigned));
	*mem=2;
	unsigned* mem2=(unsigned*)malloc(sizeof (unsigned));
	*mem2=21;
	void* mem3=malloc(100);
	int i=0;
	while(i<100){
		printf("");
		i++;
	}
	printf("\n");
	printf("mem %u\n",mem);
	printf("mem2 %u\n",mem2);
	printf("mem3 %u\n",mem3);

	free(mem);
	mem=calloc(1,20);
	mem2=realloc(mem2,100);
	printf("mem %u\n",mem);
	printf("mem2 %u\n",*mem2);
	printf("mem3 %u\n",mem3);

	free(mem);
	free(mem2);
	free(mem3);
}