#include "mallocBS.h"




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
	size=align_size(size);
	if(ptr==NULL){
		return malloc(size);
	}
	block_t* block=get_block(ptr);
	size_t req_kval=two_exp(size+BLOCK_SIZE);
	if( req_kval <= block->kval ){
		//we might be abl to decrease the size
		split_block(block,req_kval);
		return ptr;
	}
	//otherwise we may as well find a better block
	//TODO 15/5-15: check if we can expand to the right 
	void* new_ptr=malloc(size);
	if(new_ptr!=NULL){
		size_t old_mem_size=two_to_pow(block->kval)-BLOCK_SIZE;
		memcpy(new_ptr, ptr, old_mem_size);
	}
	free(ptr);
	return new_ptr;
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
			return split_block(avail_block,req_kval);
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
	block->reserved=true;
	return block;
}

block_t* merge_block(block_t* block)
{

	size_t current_kval=block->kval;
	if(current_kval==N){
		return block;
	}
	block_t* buddy = (block_t*)(memory_pool + ( ( ((char*)block) - memory_pool) ^ (1 << current_kval) ) );
	while(buddy!=NULL && !buddy->reserved){
		remove_from_free_list(buddy);
		block=concatonate_block(block,buddy);
		current_kval=block->kval;
		if(current_kval==N){
			buddy=NULL;
		}else{
			buddy = (block_t*)(memory_pool + ( ( ((char*)block) - memory_pool) ^ (1 << current_kval) ) );
		}
	}
	return block;
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
}
int main()
{
	void* mem1=malloc(10);
	void* mem2=malloc(10);

	printf("mem1 %p\n",mem1);
	printf("mem2 %p\n",mem2);
	printf("diff= %zu\n",mem2-mem1);
	free(mem1);

	void* mem3=calloc(1,10);
	free(mem2);
	free(mem3);
	printf("mem3 %p\n",mem3);

	mem3=calloc(1,200);
	mem3=realloc(mem3,1000);
}