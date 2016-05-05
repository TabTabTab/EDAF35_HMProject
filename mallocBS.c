#include "mallocBS.h"

//#define PLAYING
//#define DO_PRINT

#ifdef DO_PRINT
	#define FAKE_PRINT(...) printf(__VA_ARGS__)
#else
	#define FAKE_PRINT(...)
#endif


static block_t* free_list[N];
static char* memory_pool=NULL; //[POOL_SIZE];

void* calloc(size_t nitems, size_t size)
{
	size_t tot_size=nitems*size;
	void* ptr=malloc(tot_size);
	if(ptr!=NULL){
		memset(ptr,0,tot_size);
	}
	return ptr;
}

void* realloc(void* ptr, size_t size)
{
	if(ptr==NULL){
		return malloc(size);
	}
	block_t* block=get_block(ptr);
	size=align_size(size+BLOCK_SIZE);
	size_t req_kval=two_exp(size);
	if( req_kval <= block->kval ){
		//we might be abl to decrease the size
		split_block(block,req_kval);
		//not needed as split always returns the left most block
		block->reserved=true;
		return ptr;
	}
	//we need to store this in case we need to memcpy
	size_t old_mem_size=two_to_pow(block->kval)-BLOCK_SIZE;

	//check if we can expand to the right
	bool block_big_enough=false;
	while(!block_big_enough){


	block_t* buddy=find_buddy(block);
	if(buddy!=NULL && buddy > block && !buddy->reserved
		&& buddy->kval==block->kval){
		block=concatonate_block(block,buddy);
		remove_from_free_list(buddy);
	}else{
		break;
	}
		block_big_enough = req_kval == block->kval;
	}
	//otherwise we may as well find a better block
	if(block_big_enough){
		return block+1;
	}else{
		void* new_ptr=malloc(size);
		if(new_ptr!=NULL){
			memcpy(new_ptr, ptr, old_mem_size);
			free(block+1);
		}
		return new_ptr;
	}
}

void* malloc(size_t size)
{
	if(memory_pool!=NULL){
		size_t tot_size=align_size(size+BLOCK_SIZE);
		block_t* block=find_free_block(tot_size);

		if(block==NULL){
			return NULL;
		}
		block->reserved=true;
		return (void*)(block+1);
	}else{
		void* pool = sbrk(POOL_SIZE);
		if (pool==SBRK_FAIL){
			return NULL;
		}
		memory_pool=(char*)pool;
		block_t* first_block=(block_t*)pool;

		first_block->reserved=false;
		first_block->kval=N;
		first_block->succ=NULL;
		first_block->pred=NULL;

		free_list[N-1]=first_block;



		return malloc(size);
	}
}

void free(void* ptr)
{
	if(ptr==NULL){
		return;
	}
	block_t* block=get_block(ptr);
	block=merge_block(block);
	add_to_free_list(block);
}

block_t* find_free_block(size_t size)
{
	size_t req_kval=two_exp(size);
	block_t* block=NULL;
	size_t index=req_kval;
	while(index<=N){
		if(free_list[index-1]!=NULL){
			block_t* avail_block=free_list[index-1];
			remove_from_free_list(avail_block);
			block=split_block(avail_block,req_kval);
			block->reserved=true;
			return block;
		}
		index++;
	}
	return NULL;
}

block_t* split_block(block_t* block,size_t req_kval)
{
	size_t current_kval=block->kval;
	char* byte_ptr=(char*)block;
	while(current_kval > req_kval){
		current_kval--;

		block_t* right_half=(block_t*)(byte_ptr + two_to_pow(current_kval));
		right_half->kval=current_kval;
		add_to_free_list(right_half);
	}
	block->kval=req_kval;
	return block;
}

block_t* merge_block(block_t* block)
{

	size_t current_kval=block->kval;
	if(current_kval==N){
		return block;
	}

	block_t* buddy = find_buddy(block);


	while(buddy!=NULL && !buddy->reserved && buddy->kval==current_kval){
		remove_from_free_list(buddy);
		block=concatonate_block(block,buddy);
		current_kval=block->kval;
		if(current_kval==N){
			buddy=NULL;
		}else{
			buddy = find_buddy(block);
		}
	}
	return block;
}

block_t* find_buddy(block_t* block)
{
	if(block->kval<N){
		return (block_t*)(memory_pool + ( ( ((char*)block) - memory_pool) ^ (1 << block->kval) ) );
	}else{
		return NULL;
	}
}

block_t* concatonate_block(block_t* block_1,block_t* block_2)
{
	block_t* left,*right;
	if(block_1<block_2){
		left=block_1;
		right=block_2;
	}else{
		left=block_2;
		right=block_1;
	}
	left->kval+=1;
	return left;
	//now we should concat

}
void add_to_free_list(block_t* block)
{
	size_t kval=block->kval;
	block->reserved=false;
	block->succ=free_list[kval-1];
	block->pred=NULL;

	if(free_list[kval-1]!=NULL){
		free_list[kval-1]->pred=block;
	}
	free_list[kval-1]=block;
}
void remove_from_free_list(block_t* block)
{
	block_t* next=block->succ;
	block_t* pred=block->pred;
	if(next!=NULL){
		next->pred=pred;
	}
	if(pred!=NULL){
		pred->succ=next;
	}else{
		//block was first in its list
		free_list[block->kval-1]=next;
	}
}
size_t align_size(size_t size)
{
	size_t a_size=two_to_pow(two_exp(size));
	return a_size;
}
size_t two_to_pow(size_t pow)
{
	return 1<<pow;
}
size_t two_exp(size_t value)
{
	size_t ONE=0x1;

	size_t nbits=0;
	size_t nones=0;

	while(value>0){
		nbits++;
		if(value & ONE == ONE){
			nones++;
		}
		value>>=1;
	}
	return (nones>1) ? nbits : nbits-1;
}
block_t* get_block(void* data_ptr)
{
	block_t* block=((block_t*)data_ptr)-1;
	return block;
}


/*
int main()
{
	void* p;
	//p = malloc(POOL_SIZE-BLOCK_SIZE+0);
	//printf("addr: %p\n",p);
	//size_t s=POOL_SIZE;
	//size_t sh=POOL_SIZE/2;
	//printf("%zu, %zu\n",s,sh);
	p=malloc(POOL_SIZE/2);
	printf("addr1: %p\n",p);
	p=realloc(p,POOL_SIZE/2);
	printf("addr2: %p\n",p);
	size_t size=10;
	void* mem1=malloc(size*=3);
	printf("addr: %p\n",mem1);
	mem1=realloc(mem1,size*=3);
	printf("addr: %p\n",mem1);
	mem1=realloc(mem1,size*=3);
	printf("addr: %p\n",mem1);
}
*/
