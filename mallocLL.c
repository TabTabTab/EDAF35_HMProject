#include "mallocLL.h"


#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>




static bnode_t* current_head=NULL;


void* malloc(size_t size)
{
	bnode_t* bnode = find_bnode(size);
}


bnode_t*  find_bnode(size_t size)
{
	bnode_t* start_node=current_head;
	do{
		


		current_head=current_head->next;
	}while(current_head!=start_node);

}

int main()
{
	printf("hello\n");
}