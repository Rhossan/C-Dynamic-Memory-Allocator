#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "../include/sfmm.h"

/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */

 /*****************************************
	SET ERRNO ACCORDINGLY AND RETURN NULL FOR MALLOC, FREE, REALLOC
	MAKE A COUNTER TO KEEP TRACK OF HOW MUCH FREE SPACE THERE IS TOTAL
	FIX PADDING!!
 *///////////////////////////////////////////////

#define word_size	8
#define Q_word_size	16
void* BEGINNING;
void* UPPERBOUND;
sf_free_header* freelist_head = NULL;
sf_footer* freelist_foot = NULL;

int pageCount = 0;
int pageSize = 0;

static size_t internal = 0;
static size_t external = 0;
static size_t allocations = 0;
static size_t frees = 0;
static size_t coalesce = 0;

// void* check_validate(){
// 	if(pageCount>4){
// 		errno = ENOMEM;
// 		return NULL;
// 	}

// }

void increment_internal_frag(size_t padding){
	internal +=16;
	internal +=padding;
}
size_t totalSpace(size_t size){
	size_t space_needed = 0;
	size_t payload;
	size_t padding = 0;

	if (size < Q_word_size){ //payload is 16 bytes
		//space_needed = 2*D_word_size;
		payload = size;
		padding = 16-size;
	}
	else{
		payload = size;
		//while(payload %16 != 0){ payload++;} //make sure payload is 16 byte alligned, address falls on a 16byte alligned, and remember the padding!
		padding = 0; //payload - size;
	}

	int c = payload + padding;
	while (c%16!=0){
		padding++;
		c = payload + padding;
	}

	space_needed += 16 + payload + padding; //space needed + 16 bytes for header footer
	return space_needed;
}
size_t totalSpace_padding(size_t size){
	size_t space_needed = 0;
	size_t payload;
	size_t padding = 0;

	if (size < Q_word_size){ //payload is 16 bytes
		//space_needed = 2*D_word_size;
		payload = size;
		padding = 16-size;
	}
	else{
		payload = size;
		//while(payload %16 != 0){ payload++;} //make sure payload is 16 byte alligned, address falls on a 16byte alligned, and remember the padding!
		padding = 0; //payload - size;
	}

	int c = payload + padding;
	while (c%16!=0){
		padding++;
		c = payload + padding;
	}

	space_needed += 16 + payload + padding; //space needed + 16 bytes for header footer
	return padding;
}

/*
size_t space_needed = 0;
	size_t payload;
	size_t padding = 0;
	void* ptr;
	void* return_ptr;

	if (size < Q_word_size){ //payload is 16 bytes
		//space_needed = 2*D_word_size;
		payload = size;
		padding = 16-size;
	}
	else{
		payload = size;
		//while(padding %16 != 0){ payload++;} //make sure payload is 16 byte alligned, address falls on a 16byte alligned, and remember the padding!
		//padding = payload - size;
		padding = 0;
	}

	int c = payload + padding;
	while (c%16!=0){
		padding++;
		c = payload + padding;
	}

	space_needed += 16 + payload + padding; //space needed + 16 bytes for header footer

*/

void RemoveFromFreeList(sf_free_header* block){

	//if block is the head
	if(block->next != NULL && block->prev == NULL){
		block->next->prev = NULL;
		block = block->next;
		freelist_head = block;
	}	
	//if block is the middle
	if(block->next != NULL && block->prev != NULL){
		block->next->prev = NULL;
		block->prev->next = NULL;
	}
	//if block is the tail
	if(block->next == NULL && block->prev != NULL){
		block->prev->next = NULL;
		block = block->prev;
		//do i need to do something for freelist head or foot?
	}
															//update last free head pointer which we will use to have the freelist head point to
	
}


void* iterateMalloc(size_t space_needed, size_t padding){

	sf_free_header* current_free_block = freelist_head;
	size_t block_size_currentFree  = freelist_head->header.block_size<<4;
	//get the block size of the freelist head
	//iterate through a while loop to iterate through the freelist and find where to place the free block
	while(space_needed > block_size_currentFree){
		if(current_free_block->next == NULL){errno = ENOMEM; return NULL;}
		current_free_block = current_free_block->next;
		block_size_currentFree  = freelist_head->header.block_size<<4;
	}
	
	//if this case, split free block into one alloc with malloced space, and the free block
	// remove the whole block from the list, split, and put back as the new head of the freelist
	size_t new_block_size = (block_size_currentFree - space_needed);
	if( new_block_size >= 32 ){
		RemoveFromFreeList(current_free_block);//remove from the list 
		sf_header* new_alloc_header = (sf_header*)current_free_block;
		

		new_alloc_header-> alloc = 1;
		new_alloc_header-> block_size = space_needed >> 4;
		new_alloc_header-> padding_size = padding;
		//increment_internal_frag(new_alloc_header->padding_size);
		void* return_ptr = new_alloc_header;
		return_ptr += 8;
		void* foot = new_alloc_header;
		foot -= 8;
		size_t ptr_to_foot = space_needed;
		//ptr_to_foot -= 8;
		foot += ptr_to_foot; //go to footer
	
		sf_footer* footer = foot;
		footer -> alloc = 1;
		footer -> block_size = space_needed >> 4;

		void* ptr_to_free = foot;
		ptr_to_free += 8;
		sf_free_header* old_free_header = freelist_head;
		freelist_head = ptr_to_free;
		freelist_head -> header.block_size = new_block_size >> 4;
		freelist_head -> header.alloc = 0;
		freelist_head -> next = old_free_header;
		freelist_head -> prev = NULL;


		void* free_foot = ptr_to_free;// + (freelist_head->header.block_size << 4) -8;
		free_foot -= 8;
		size_t ptr_to_free_foot =  (freelist_head->header.block_size << 4);
		free_foot += ptr_to_free_foot;
		sf_footer* free_footer = free_foot;

		size_t size_of_footer_free_block = (freelist_head->header.block_size << 4);
		free_footer->block_size = size_of_footer_free_block >> 4;
		free_footer->alloc = 0;

		return return_ptr;


	}

	RemoveFromFreeList(current_free_block);//remove from the list 
	sf_header* new_alloc_header = (sf_header*)current_free_block;
		

		new_alloc_header-> alloc = 1;
		new_alloc_header-> block_size = (space_needed+new_block_size) >> 4;
		new_alloc_header-> padding_size = padding;
		//increment_internal_frag(new_alloc_header->padding_size);
		void* return_ptr = new_alloc_header;
		return_ptr += 8;
		void* foot = new_alloc_header;
		foot -= 8;
		size_t ptr_to_foot = space_needed + new_block_size;
		//ptr_to_foot -= 8;
		foot += ptr_to_foot; //go to footer
	
		sf_footer* footer = foot;
		footer -> alloc = 1;
		footer -> block_size = space_needed >> 4;

		void* ptr_to_free = foot;
		ptr_to_free += 8;
		//sf_free_header* old_free_header = freelist_head;
	return return_ptr;

	/*size_t block_size_freeHead  = freelist_head->header.block_size <<4;
	sf_header* head = (void*)freelist_head; //have header 
	
	//sf_header* head = sf_sbrk(0)-4096;

	//sf_header* head = &freelist_head->header;
	sf_footer* foot;
	//head->alloc = 1;
	head->block_size = space_needed >>4;// (payload + padding); //change this to total block size?????
	//freelist_head->header.block_size = head->block_size;fix this : update freelist head DONE
	head->padding_size = padding;
	head->alloc = 1;

	ptr = head;
	ptr += 8;	//have payload
	return_ptr = ptr;//what you want to return to the user must start at the payload

	ptr += (payload + padding); // ptr to the footer WILL FAIL W SPLINTERS
	foot = (sf_footer*)ptr;//set footer
	foot->alloc = 1;
	foot->block_size = space_needed>>4; //getting weird values when i print blocksize in gdb (like if i expect it to be 32, i get 2, but it works..)

	ptr += 8; //next free block
					freelist_head = ptr; //set free list head
					freelist_head->header.block_size = ((block_size_freeHead) - space_needed)>>4;
					freelist_foot = ptr + (freelist_head->header.block_size << 4) -8;
					freelist_foot->block_size = freelist_head->header.block_size;*/
	}

void *sf_malloc(size_t size){

	if(size <= 0){
		errno = ENOMEM;
		return NULL;
	}
	if(size >= 16368){
		errno = ENOMEM;
		return NULL;
	}
	//if(pageCount !=0 && totalSpace(size)>freelist_head->header.block_size){freelist_head=NULL;}
	if(freelist_head == NULL){
		pageCount++;
		pageSize+=4096;
		//*******/make sure there is condition for when page count reaches 4
		//******** fix next line for sf_sbrk
						freelist_head = sf_sbrk(0);
						sf_sbrk(1);
						BEGINNING = freelist_head;
						UPPERBOUND = BEGINNING;
						UPPERBOUND += 4096;
						//sf_sbrk(1);
						//freelist_foot = sf_sbrk(0)-8;//set footer at end of page with 8 bytes to spare?
						

						freelist_head->next = NULL;//((sf_free_header*)footer);
						freelist_head->prev = NULL;

  						freelist_head->header.alloc = 0;
  						freelist_head->header.block_size = 4096>>4;
  		//freelist_head->header.unused_bits = 0;
  						freelist_head->header.padding_size = 0;

  						//void ptr = head and then add to pointer block size -8 at suggestion of the TA
						void* pointer_foot = freelist_head;
						size_t x = (freelist_head->header.block_size<<4)-8;

						pointer_foot += x;
						freelist_foot = pointer_foot;

  						freelist_foot->alloc = 0;
  						freelist_foot->block_size = 4096>>4; //*****make sure freelist foot is same block size as freelist head

	}
	
	size_t space_needed = 0;
	size_t payload;
	size_t padding = 0;
	void* ptr;
	void* return_ptr;

	if (size < Q_word_size){ //payload is 16 bytes
		//space_needed = 2*D_word_size;
		payload = size;
		padding = 16-size;
	}
	else{
		payload = size;
		//while(padding %16 != 0){ payload++;} //make sure payload is 16 byte alligned, address falls on a 16byte alligned, and remember the padding!
		//padding = payload - size;
		padding = 0;
	}

	int c = payload + padding;
	while (c%16!=0){
		padding++;
		c = payload + padding;
	}

	space_needed += 16 + payload + padding; //space needed + 16 bytes for header footer

	increment_internal_frag(padding);

	////////////////////////////////////////////////////////////////////////////////////// (1) determine space needed to malloc

	if(freelist_head -> next == NULL){
	
	size_t block_size_freeHead  = freelist_head->header.block_size <<4;
	sf_header* head = (void*)freelist_head; //have header 
	
	//sf_header* head = sf_sbrk(0)-4096;

	//sf_header* head = &freelist_head->header;
	sf_footer* foot;
	//head->alloc = 1;
	head->block_size = space_needed >>4;// (payload + padding); //change this to total block size?????
	//freelist_head->header.block_size = head->block_size; 
	head->padding_size = padding;
	head->alloc = 1;

	ptr = head;
	ptr += 8;	//have payload
	return_ptr = ptr;//what you want to return to the user must start at the payload

	ptr += (payload + padding); // ptr to the footer WILL FAIL W SPLINTERS
	foot = (sf_footer*)ptr;//set footer
	foot->alloc = 1;
	foot->block_size = space_needed>>4; //getting weird values when i print blocksize in gdb (like if i expect it to be 32, i get 2, but it works..)

	ptr += 8; //next free block
					freelist_head = ptr; //set free list head
					freelist_head->header.block_size = ((block_size_freeHead) - space_needed)>>4;
					freelist_foot = ptr + (freelist_head->header.block_size << 4) -8;
					freelist_foot->block_size = freelist_head->header.block_size;
					//why not set the alloc to 0?? ********
	}
	else{
		return_ptr = iterateMalloc(space_needed,padding);}

	//DO I NEED TO UPDATE THE PADDING???


	//size_t s = space_needed/16;
	//freelist_head->header.block_size -= s;
	//freelist_foot->block_size = freelist_head->header.block_size;




	return (void*) return_ptr;

 




	/*
	if (size < D_word_size){
		space_needed = 2*D_word_size;
	}
	else{ //make size divisible by 8 or 16
		space_needed = D_word_size * ((size + (D_word_size) + (D_word_size-1)) / D_word_size);
	} */
	//printf("%lu\n", space_needed);

 

  //while loop to see if you can find (size) bytes avaiable + 16 and then make sure it is divisible by 8(or is it 16?)
	/*size_t space_needed = size + 16;
	while(space_needed%16 !=0){ space_needed++;} //space needed + 16 for header/footer
	//char * ptr =  sf_sbrk(0)+4096;
	

	while() */
}


void Coallesce_caseOne(void* ptr,size_t block_size,void* footer_ptr){ //last one is currentblock footer
		

	sf_free_header* new_Header = ptr;
	new_Header->next = freelist_head;
	new_Header->prev = NULL;
	new_Header->header.alloc = 0;
	new_Header->header.block_size = block_size;
	new_Header->header.padding_size = 0;
	sf_footer* footer = footer_ptr;
	footer->alloc = 0;
	footer->block_size = block_size;
	new_Header->header.padding_size = 0;
	freelist_head = new_Header;
	

	//freelist_head = ptr;
	//old_free->prev = freelist_head;
	//old_free->next = freelist_foot;
	//freelist_head->next = old_free;
	//freelist_head->prev = NULL;
	//freelist_head->header.alloc = 0;
	//freelist_head->header.block_size = old_free->header.block_size;
	//freelist_head->header.padding_size = 0;
	//freelist_foot->block_size = freelist_head->header.block_size;

	//printf("hello, this worked so far....\n");


}
void Coallesce_caseTwo_realloc(sf_header* current_block, sf_header* next_block_header, void* currentblock_footer){
	sf_free_header* next_block = (sf_free_header*)next_block_header;
	size_t old_block_size = next_block->header.block_size; //current block's size

	//edge case if freeing block with freelist	
	if(next_block->next == NULL && next_block->prev == NULL){
		sf_free_header* prev = freelist_head->prev;
		freelist_head = (sf_free_header*)current_block; //make freelist point to currentblock

		size_t i = current_block->block_size<<4;
		i += old_block_size<<4;
		freelist_head->header.block_size = i>>4;
		freelist_head->header.alloc = 0;
		//sf_footer* footer = currentblock_footer;
		//footer->alloc = 0;
		freelist_head->prev = NULL;
		freelist_head->next = prev;
		void* footer = freelist_head;
		footer += i;
		footer -=8;
		sf_footer* foot = footer;
		foot->alloc = 0;
		foot->block_size = i>>4;


		return;
	} 

	RemoveFromFreeList(next_block); //update last free head pointer which we will use to have the freelist head point to



	//coallesce the two blocks
	sf_free_header* new_head = (sf_free_header*)current_block;
	new_head->next = freelist_head;
	new_head->prev = NULL;
	new_head->header.alloc = 0;
	size_t new_block_size = (current_block->block_size<<4); //payload and padding minus 16 for header footer
	new_block_size += old_block_size<<4;//(next_block_header->block_size<<4)-16;//payload and padding minus 16 for header footer
	
	new_head->header.block_size = new_block_size>>4;
	new_head->header.padding_size = 0;
	// sf_footer* footer = currentblock_footer;
	// footer += 8;
	sf_footer* footer = (void*)next_block_header;
	size_t size_ptr_to_end_coallesced = next_block_header->block_size<<4;
	size_ptr_to_end_coallesced -= 8;
	void *ptr_to_end_coallesced = next_block_header;
	ptr_to_end_coallesced += size_ptr_to_end_coallesced;
	footer = ptr_to_end_coallesced;
	
	 footer->alloc = 0;
	 footer->block_size = new_block_size>>4;
	 freelist_head = new_head;


	// //new_block_size += 16; //header footer
	// new_head->header.block_size = new_block_size>>4;

	// // wont i need to update freelist head and footer?
	// new_head->header.padding_size = 0;
	// sf_footer* footer = currentblock_footer;

	// //footer += new_head->header.block_size;
	// //footer = footer - 8;
	// footer->alloc = 0;
	// footer->block_size = new_block_size>>4;
	// freelist_head = new_head;

}

void Coallesce_caseTwo(sf_header* current_block, sf_header* next_block_header, void* currentblock_footer){
	//step 1 get next block 
	sf_free_header* next_block = (sf_free_header*)next_block_header;
	size_t old_block_size = next_block->header.block_size; //current block's size

	//edge case if freeing block with freelist	
	if(next_block->next == NULL && next_block->prev == NULL){
		sf_free_header* prev = freelist_head->prev;
		freelist_head = (sf_free_header*)current_block; //make freelist point to currentblock

		size_t i = current_block->block_size<<4;
		i += old_block_size<<4;
		freelist_head->header.block_size = i>>4;
		freelist_head->header.alloc = 0;
		//sf_footer* footer = currentblock_footer;
		//footer->alloc = 0;
		freelist_head->prev = NULL;
		freelist_head->next = prev;
		void* footer = freelist_head;
		footer += i;
		footer -=8;
		sf_footer* foot = footer;
		foot->alloc = 0;
		foot->block_size = i>>4;


		return;
	} 

	RemoveFromFreeList(next_block); //update last free head pointer which we will use to have the freelist head point to



	//coallesce the two blocks
	sf_free_header* new_head = (sf_free_header*)current_block;
	new_head->next = freelist_head;
	new_head->prev = NULL;
	new_head->header.alloc = 0;
	size_t new_block_size = (current_block->block_size<<4); //payload and padding minus 16 for header footer
	new_block_size += old_block_size<<4;//(next_block_header->block_size<<4)-16;//payload and padding minus 16 for header footer
	//new_block_size += 16; //header footer
	new_head->header.block_size = new_block_size>>4;

	// wont i need to update freelist head and footer?
	new_head->header.padding_size = 0;
	sf_footer* footer = currentblock_footer;

	//footer += new_head->header.block_size;
	//footer = footer - 8;
	footer->alloc = 0;
	footer->block_size = new_block_size>>4;
	freelist_head = new_head;


}
void Coallesce_caseThree(sf_header* current_block, sf_header* prev_block_header, void* currentblock_footer){
	sf_free_header* prev_block = (sf_free_header*)prev_block_header;
	size_t old_block_size = prev_block->header.block_size; //current block's size
	RemoveFromFreeList(prev_block); //update last free head pointer which we will use to have the freelist head point to

	sf_free_header* new_head = (sf_free_header*)prev_block;
	new_head->next = freelist_head;
	new_head->prev = NULL;
	new_head->header.alloc = 0;
	size_t new_block_size = (current_block->block_size<<4); //payload and padding minus 16 for header footer
	new_block_size += old_block_size<<4;//(next_block_header->block_size<<4)-16;//payload and padding minus 16 for header footer
	//new_block_size += 16; //header footer
	new_head->header.block_size = new_block_size>>4;

	new_head->header.padding_size = 0;
	sf_footer* footer = currentblock_footer;

	//footer += new_head->header.block_size;
	//footer = footer - 8;
	footer->alloc = 0;
	footer->block_size = new_block_size>>4;
	freelist_head = new_head;

	}

void Coallesce_caseFour(sf_header* current_block, sf_header* next_block_header, sf_header* prev_block_header, void* current_block_footer){
	//size_t x = prev_block_header->block_size; //MAY LEAD TO ERRORS KEEP A CLOSE EYE ON THIS
	Coallesce_caseThree(current_block,prev_block_header,current_block_footer);
	size_t x= next_block_header->block_size<<4;
	current_block_footer += x;

	Coallesce_caseTwo(prev_block_header,next_block_header,current_block_footer);
	//current_block_footer += 32;
}



void sf_free(void *ptr){
//for freeing, make a pointer to header, subtract 8 to get footer
	//know when to use varprint and other one
	//
				//	sf_free_header* freeing_head = freelist_head;
	frees++;
	if(ptr == NULL){errno = ENOMEM; return (void)NULL;} //MAYBE SET AN ERROR
	//CHECK RANGES SHOULD BE NEXT CHECK *****


	sf_header* prev_block;
	sf_header* current_block;
	sf_header* next_block;

	ptr -= 8; //(-8 to the payload)
	current_block = ptr; // current block points to head now
	if(current_block->alloc == 0){
		errno = ENOMEM;
		return; //MAYBE SET ERRNO
	}

	ptr -= 8;
	prev_block = ptr; //prev block points to previous block's footer
	//size_t x = (prev_block->block_size<<4);
	void* ptr_prev_fix = ptr;
	ptr_prev_fix = ptr - (prev_block->block_size<<4);
	ptr_prev_fix += 8;
	prev_block = ptr_prev_fix;
	//freelist_foot = ptr + (freelist_head->header.block_size << 4) -8;
	//prev_block -= x; //go to header

	ptr += 8;//point back to current block's head


	//size_t x = current_block->block_size<<4;
	//printf("%lu\n", x);
	ptr += current_block->block_size<<4;
	next_block = ptr;
	ptr -= 8;
	void* current_block_footer = ptr;
	//if prev block is NULL as in it is out of range of list, do case 1 using next block
	if(prev_block < (sf_header*)BEGINNING){
		if(next_block->alloc == 1)
			Coallesce_caseOne(current_block,current_block->block_size,current_block_footer);
			coalesce++;
	}

	else if(prev_block->alloc == 1 && next_block->alloc == 1){
			Coallesce_caseOne(current_block,current_block->block_size,current_block_footer);
			
	}
	else if(prev_block->alloc == 1 && next_block->alloc == 0){
			size_t x= next_block->block_size<<4;
			current_block_footer += x;

			Coallesce_caseTwo(current_block, next_block,current_block_footer);
			coalesce++;
	}
	else if(prev_block->alloc == 0 && next_block->alloc == 1){
			
			//current_block_footer += prev_block->block_size<<4;
			Coallesce_caseThree(current_block, prev_block, current_block_footer);
			coalesce++;

	}
	else if(prev_block->alloc == 0 && next_block->alloc == 0){
		Coallesce_caseFour(current_block,next_block,prev_block,current_block_footer);
		coalesce++;
	}

	/*new_free->header.alloc = 0;
	new_free->header.block_size = 32;
	new_free->header.padding_size = 0;
	new_free->next = freelist_head;
	new_free->prev = NULL;

	freelist_head = new_free;

	freelist_head->next = freeing_head;
	freelist_head->prev = NULL;

	ptr+=32;
	sf_footer* new_free_foot = ptr;
	new_free_foot->alloc =0;
	new_free_foot->block_size=32;
*/


}

void startCoallesce(sf_header* prev_block, sf_header* next_block, sf_header* current_block, sf_footer* current_block_footer){
if(prev_block < (sf_header*)BEGINNING){
		if(next_block->alloc == 1)
			Coallesce_caseOne(current_block,current_block->block_size,current_block_footer);
	}

	else if(prev_block->alloc == 1 && next_block->alloc == 1){
			Coallesce_caseOne(current_block,current_block->block_size,current_block_footer);
	}
	else if(prev_block->alloc == 1 && next_block->alloc == 0){
			size_t x= next_block->block_size<<4;
			current_block_footer += x;

			Coallesce_caseTwo(current_block, next_block,current_block_footer);
	}
	else if(prev_block->alloc == 0 && next_block->alloc == 1){
			
			//current_block_footer += prev_block->block_size<<4;
			Coallesce_caseThree(current_block, prev_block, current_block_footer);

	}
	else if(prev_block->alloc == 0 && next_block->alloc == 0){
		Coallesce_caseFour(current_block,next_block,prev_block,current_block_footer);
	}
}

void setFooter(sf_header* head,size_t space_needed){
	void* foot = head;
		foot -= 8;
		size_t ptr_to_foot = space_needed;
		//ptr_to_foot -= 8;
		foot += ptr_to_foot; //go to footer
	
		sf_footer* footer = foot;
		footer -> alloc = 1;
		footer -> block_size = space_needed >> 4;
}

void expand(sf_header* current_block, sf_header* next_block_header, size_t size){
	size_t current_block_size = current_block->block_size<<4;
	size_t space_needed = totalSpace(size);
	size_t new_block_size = (space_needed - current_block_size);
	sf_header* new_block_header = next_block_header;

	//case 1 next to a free block 
	if(next_block_header->alloc == 0){
		size_t determine_take_whole_block = next_block_header->block_size<<4;
		determine_take_whole_block -= new_block_size;
		//take whole block
		if(determine_take_whole_block <= 0){
			//footer goes to a8
			//header is same at 38
			//remove from freelist
			void* ptr_to_footer = next_block_header;
			size_t size_to_next_foot = next_block_header->block_size<<4;
			ptr_to_footer += size_to_next_foot;
			ptr_to_footer -= 8; //should be  a0
			sf_footer* footer = ptr_to_footer;


			sf_free_header* remove_free = (sf_free_header*)new_block_header;
			RemoveFromFreeList(remove_free);
			sf_header* new_realloc_block = current_block;
			new_realloc_block->block_size = space_needed>>4;
			footer->block_size = space_needed>>4;
			footer->alloc = 1;
			new_realloc_block->padding_size = totalSpace_padding(size);
			increment_internal_frag(new_realloc_block->padding_size);
			footer->alloc = 1;
			

		}



	}
	else{
		//splinter
		//take entire block
	}

}

void shrink(sf_header* current_block, size_t size, void* ptr){
	size_t current_block_size = current_block->block_size<<4;
	size_t space_needed = totalSpace(size);
	size_t new_block_size = (current_block_size - space_needed);
	sf_header* new_block_header = current_block;
	//splinter
	//so change the realloc to smaller number block size but still have footer at space needed distance
	if( new_block_size < 32 ){
		//dont use this block size, its the difference between what you have and is used for case two of shrink(coallesce)
		//current_block->block_size = new_block_size >> 4; 

	//padding size changes to padding of requested size
	current_block->padding_size = 0;
		//set padding
	//set footer
	void* foot = current_block;
	foot -= 8;
	size_t ptr_to_foot = current_block_size;
	foot += ptr_to_foot; //go to footer
	
	sf_footer* footer = foot;
	footer -> alloc = 1;
	//footer -> block_size = new_block_size >> 4;
	//footer -> alloc = 1;

	current_block->padding_size = totalSpace_padding(size);
	increment_internal_frag(current_block->padding_size);

	}
	else{
	// no splinter
	void* foot = current_block;
	foot -= 8;
	size_t ptr_to_foot = current_block_size;
	foot += ptr_to_foot; //go to footer
	void* ptr_to_new_head_alloc = current_block;

	//change current_block values to split
	current_block -> block_size = space_needed >> 4;
	setFooter(current_block,space_needed);

	ptr_to_new_head_alloc += space_needed;
	//ptr_to_new_head_alloc -= 8;
	new_block_header = ptr_to_new_head_alloc;
	//foot += 8;
	

	sf_footer* footer = foot;
	foot += 8;
	sf_header* next_block_header = foot;
	new_block_header->block_size = new_block_size >> 4;
	//do i need to set the footer values to free before coallescing?
	//case 1 coallesce
	if(next_block_header->alloc == 0){
		Coallesce_caseTwo_realloc(new_block_header,next_block_header,footer);
		coalesce++;
	}
	else{
		//no coallescing needed
		//new block header is where we want to start and footer is end
		sf_free_header* old_head = freelist_head;
		freelist_head = (sf_free_header*)new_block_header;
		freelist_head->header.alloc = 0; //just in case, size is done already (above if statement)
		freelist_head -> next = old_head;
		freelist_head -> prev = NULL;
		footer->block_size = new_block_size >> 4;
		footer->alloc = 0;


	}

	}

	

}


void *sf_realloc(void *ptr, size_t size){
	//check cases 
	if(ptr == NULL){errno = ENOMEM; return NULL;} //MAYBE SET AN ERROR
	//check if it is a free block
	if(size == 0){errno = ENOMEM; return NULL;}

	void* check_ptr = ptr;
	check_ptr -= 8;
	sf_header* current_block = check_ptr;
	void* return_ptr = current_block;
		return_ptr += 8;
	if(current_block->alloc == 0){
		errno = ENOMEM;
		return NULL;
	}
	//CHECK RANGES!!!!!!!



	void* ptr_to_next_block = current_block;
	ptr_to_next_block += current_block->block_size <<4;
	//ptr_to_next_block -= 8;
	sf_header* next_block_header = ptr_to_next_block;

	size_t block_size = current_block->block_size<<4;

	//expand
	if(block_size < size){
		expand(current_block,next_block_header,size);
		return return_ptr;
	}
	//SHRINK 
	
	if(block_size > size){
		shrink(current_block,size,ptr);
		return return_ptr;
	}
	if(block_size == size){
		//*********DO THIS
	}

	
	return return_ptr;



	
}

int sf_info(info* meminfo){

	meminfo->internal = internal;

	sf_free_header* current_block = freelist_head;
	size_t total_external = 0;

	while(current_block->next != NULL){
		total_external += current_block->header.block_size<<4;
		current_block = current_block->next;
	}
	total_external += current_block->header.block_size<<4;
	external = total_external;

	meminfo->external = external;
	meminfo->allocations = allocations;
	meminfo->frees = frees;
	meminfo->coalesce = coalesce;

  return -1;
}
