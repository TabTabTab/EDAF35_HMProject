#include "mallocBS.h"




static block_t* free_list[N];
static char memory[POOL_SIZE];


block_t* find_free_block(size_t size)
{
	size_t req_kval=two_exp(size);
	block_t* block=NULL;
	size_t index=req_kval;
	while(index<N){
		if(free_list[index]!=NULL){
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