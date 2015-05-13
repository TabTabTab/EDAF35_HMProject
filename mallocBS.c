#include "mallocBS.h"




static block_t* free_list[N];
static char* memory_pool=NULL; //[POOL_SIZE];



void* malloc(size_t size)
{
	if(memory_pool!=NULL){




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



block_t* find_free_block(size_t size)
{
	size_t req_kval=two_exp(size);
	block_t* block=NULL;
	size_t index=req_kval;
	while(index<N){
		if(free_list[index]!=NULL){
			block_t*
			free_list[index]
			return split_block(block,req_kval);
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
		///NOT DONE YET! 11/5-2015
		block_t* right_half=(block_t*)(byte_ptr + two_to_pow(current_kval));
		right_half->reserved=false;
		right_half->kval=current_kval;
		right_half->succ=free_list[current_kval-1];
		right_half->pred=NULL;
		if(free_list[current_kval-1]!=NULL){
			free_list[current_kval-1]->prev=right_half;
		}
		free_list[current_kval-1]=right_half;
	}

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

int main()
{
	size_t n=4;
	printf("%zd\n",two_exp(n));
}