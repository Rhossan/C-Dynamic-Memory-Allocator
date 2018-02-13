#include <criterion/criterion.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/sfmm.h"

/**
 *  HERE ARE OUR TEST CASES NOT ALL SHOULD BE GIVEN STUDENTS
 *  REMINDER MAX ALLOCATIONS MAY NOT EXCEED 4 * 4096 or 16384 or 128KB
 */

Test(sf_memsuite, Malloc_an_Integer, .init = sf_mem_init, .fini = sf_mem_fini) {
    int *x = sf_malloc(sizeof(int));
    *x = 4;
    cr_assert(*x == 4, "Failed to properly sf_malloc space for an integer!");
}

Test(sf_memsuite, Free_block_check_header_footer_values, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *pointer = sf_malloc(sizeof(short));
    sf_free(pointer);
    pointer = pointer - 8;
    sf_header *sfHeader = (sf_header *) pointer;
    cr_assert(sfHeader->alloc == 0, "Alloc bit in header is not 0!\n");
    sf_footer *sfFooter = (sf_footer *) (pointer - 8 + (sfHeader->block_size << 4));
    cr_assert(sfFooter->alloc == 0, "Alloc bit in the footer is not 0!\n");
}

Test(sf_memsuite, PaddingSize_Check_char, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *pointer = sf_malloc(sizeof(char));
    pointer = pointer - 8;
    sf_header *sfHeader = (sf_header *) pointer;
    cr_assert(sfHeader->padding_size == 15, "Header padding size is incorrect for malloc of a single char!\n");
}

Test(sf_memsuite, Check_next_prev_pointers_of_free_block_at_head_of_list, .init = sf_mem_init, .fini = sf_mem_fini) {
    int *x = sf_malloc(4);
    memset(x, 0, 4);
    cr_assert(freelist_head->next == NULL);
    cr_assert(freelist_head->prev == NULL);
}

Test(sf_memsuite, Coalesce_no_coalescing, .init = sf_mem_init, .fini = sf_mem_fini) {
    void *x = sf_malloc(4);
    void *y = sf_malloc(4);
    memset(y, 0xFF, 4);
    sf_free(x);
    cr_assert(freelist_head == x-8);
    sf_free_header *headofx = (sf_free_header*) (x-8);
    sf_footer *footofx = (sf_footer*) (x - 8 + (headofx->header.block_size << 4)) - 8;

    sf_blockprint((sf_free_header*)((void*)x-8));
    // All of the below should be true if there was no coalescing
    cr_assert(headofx->header.alloc == 0);
    cr_assert(headofx->header.block_size << 4 == 32);
    cr_assert(headofx->header.padding_size == 0);

    cr_assert(footofx->alloc == 0);
    cr_assert(footofx->block_size << 4 == 32);
}

/*
//############################################
// STUDENT UNIT TESTS SHOULD BE WRITTEN BELOW
// DO NOT DELETE THESE COMMENTS
//############################################
*/
Test(sf_memsuite, check_Realloc_EXPAND_splinter_take_entire_block, .init = sf_mem_init, .fini = sf_mem_fini){
    long *value01 = sf_malloc(sizeof(long));
    //null_check(value01, sizeof(int));
     sf_varprint(value01);
    //press_to_cont();

    void* value02 = sf_malloc(64);//80
     sf_varprint(value02);
    //press_to_cont();

      long *value03 = sf_malloc(sizeof(long));
    //null_check(value03, sizeof(int));
     sf_varprint(value03);
     //press_to_cont();

    void* value04 = sf_malloc(128);//size 144
    sf_varprint(value04);
    //sf_snapshot(true);

    sf_free(value03);
    sf_varprint(sf_realloc(value02,90));// need size 112
    sf_free_header* headof_value02 = (sf_free_header*) (value02-8);
    cr_assert(headof_value02 -> header.block_size<<4 == 112, "blocksize should be 112 when you expand and take the whole block");

}

Test(sf_memsuite, check_Realloc_SHRINK_splinter_take_entire_block, .init = sf_mem_init, .fini = sf_mem_fini){
      int *value01 = sf_malloc(sizeof(int));
       sf_varprint(value01);
    //null_check(value01, sizeof(int));
    //press_to_cont();

    void* value02 = sf_malloc(128);//144
     sf_varprint(value02);
    //press_to_cont();
    //allocates for 96 bytes

    int *value03 = sf_malloc(sizeof(int));
       sf_varprint(value03);
    //press_to_cont();

      long *value04 = sf_malloc(64);//80
    //null_check(value04, sizeof(int));
     sf_varprint(value04);
     //press_to_cont();

      sf_free(value03);

    sf_varprint(sf_realloc(value02,64));// need size 112
    sf_free_header* headof_value02 = (sf_free_header*) (value02-8);
    cr_assert(headof_value02 -> header.block_size<<4 == 80);
    void* ptr = headof_value02;
    ptr += 176;
    ptr-= 8;
    sf_footer* footer_of_value03 = ptr;
    cr_assert(footer_of_value03->block_size<<4 == 96);
    cr_assert(footer_of_value03->alloc == 0);

    //sf_snapshot(true);
     //press_to_cont();

}

Test(sf_memsuite, check_coallesce_right, .init = sf_mem_init, .fini = sf_mem_fini){
     int *value01 = sf_malloc(sizeof(int));
       sf_varprint(value01);
    //null_check(value01, sizeof(int));
    //press_to_cont();

    void* value02 = sf_malloc(128);//144
     sf_varprint(value02);
    //press_to_cont();
    //allocates for 96 bytes

    int *value03 = sf_malloc(sizeof(int));
       sf_varprint(value03);
    //press_to_cont();

     int *value04 = sf_malloc(sizeof(int));
       sf_varprint(value04);

    sf_free(value03);
    sf_free(value02);

    sf_free_header* headof_value02 = (sf_free_header*) (value02-8);
    cr_assert(headof_value02->header.block_size<<4 == 176,"Checking coallescing right by freeing a block of 32 bytes, then freeing a block before it that is 140, should be total 176!");
    cr_assert(headof_value02->header.alloc == 0);


}

Test(sf_memsuite, check_coallesce_left, .init = sf_mem_init, .fini = sf_mem_fini){
     int *value01 = sf_malloc(sizeof(int));
       sf_varprint(value01);
    //null_check(value01, sizeof(int));
    //press_to_cont();

    void* value02 = sf_malloc(128);//144
     sf_varprint(value02);
    //press_to_cont();
    //allocates for 96 bytes

    int *value03 = sf_malloc(sizeof(int));
       sf_varprint(value03);
    //press_to_cont();

     int *value04 = sf_malloc(sizeof(int));
       sf_varprint(value04);

    sf_free(value02);
    sf_free(value03);

    sf_free_header* headof_value02 = (sf_free_header*) (value02-8);
    cr_assert(headof_value02->header.block_size<<4 == 176, "coallescing left, should be total 176");
    cr_assert(headof_value02->header.alloc == 0);


}


Test(sf_memsuite, check_coallesce_both, .init = sf_mem_init, .fini = sf_mem_fini){
     int *value01 = sf_malloc(sizeof(int));
       sf_varprint(value01);
    //null_check(value01, sizeof(int));
    //press_to_cont();

    void* value02 = sf_malloc(128);//144
     sf_varprint(value02);
    //press_to_cont();
    //allocates for 96 bytes

    int *value03 = sf_malloc(sizeof(int));
       sf_varprint(value03);
    //press_to_cont();

     int *value04 = sf_malloc(sizeof(int));
       sf_varprint(value04);

    sf_free(value02);
    sf_free(value04);
    sf_free(value03);

    //sf_free_header* headof_value02 = (sf_free_header*) (value02-8);
    cr_assert(freelist_head->header.block_size<<4 == 4064, "coallescing left and right, and also checking edgecase, should be total 4064");
    cr_assert(freelist_head->header.alloc == 0);


}


Test(sf_memsuite, check_iterating_malloc, .init = sf_mem_init, .fini = sf_mem_fini){
     int *value01 = sf_malloc(sizeof(int));
       sf_varprint(value01);
    //null_check(value01, sizeof(int));
    //press_to_cont();

    void* value02 = sf_malloc(128);//144
     sf_varprint(value02);
    //press_to_cont();
    //allocates for 96 bytes

    int *value03 = sf_malloc(sizeof(int));
       sf_varprint(value03);
    //press_to_cont();

     int *value04 = sf_malloc(sizeof(int));
       sf_varprint(value04);

    sf_free(value04);
    sf_free(value02);
    void *value05 = sf_malloc(sizeof(long)*5);
    sf_varprint(value05);
    sf_snapshot(true);

    sf_header* headof_value05 = (sf_header*) (value05-8);
    cr_assert(headof_value05->block_size<<4 == 64,"the block size isn't correct when malloc is done(when iterating through freelist)");
    cr_assert(freelist_head->header.block_size<<4 == 80, "iterate through the free list, make sure splitting works, and freelist is kept intact");
    
    }
Test(sf_memsuite, coallesce_beginning_edgecase, .init = sf_mem_init, .fini = sf_mem_fini){
    int *value01 = sf_malloc(sizeof(int));
       sf_varprint(value01);
    //null_check(value01, sizeof(int));
    //press_to_cont();

    void* value02 = sf_malloc(128);//144
     sf_varprint(value02);
    //press_to_cont();
    //allocates for 96 bytes

    int *value03 = sf_malloc(sizeof(int));
       sf_varprint(value03);
    //press_to_cont();

     int *value04 = sf_malloc(sizeof(int));
       sf_varprint(value04);

    sf_free(value02);
    sf_free(value01);
    cr_assert(freelist_head->header.block_size<<4 == 176,"beginning edge case for coallescing, it must be total of 176!");
}

Test(sf_memsuite, check_combination_realloc_malloc, .init = sf_mem_init, .fini = sf_mem_fini){
     int *value01 = sf_malloc(sizeof(int));
       sf_varprint(value01);
    //null_check(value01, sizeof(int));
    //press_to_cont();

    void* value02 = sf_malloc(128);//144
     sf_varprint(value02);
    //press_to_cont();
    //allocates for 96 bytes

    int *value03 = sf_malloc(sizeof(int));
       sf_varprint(value03);
    //press_to_cont();

     int *value04 = sf_malloc(sizeof(int));
       sf_varprint(value04);

    sf_free(value02);
    sf_free(value03);
    void* value05 = sf_malloc(80);
    sf_realloc(value05,50);

    sf_header* headof_value05 = (sf_header*) (value05-8);
    cr_assert(headof_value05->block_size<<4 == 96,"the block size isn't correct when doing malloc, then free, coallescing, then realloc shrink");
   

    }

    Test(sf_memsuite, check_info, .init = sf_mem_init, .fini = sf_mem_fini){
    info* info = sf_malloc(200);

     int *value01 = sf_malloc(sizeof(int));
       sf_varprint(value01);
    //null_check(value01, sizeof(int));
    //press_to_cont();

    void* value02 = sf_malloc(128);//144
     sf_varprint(value02);
    //press_to_cont();
    //allocates for 96 bytes

    int *value03 = sf_malloc(sizeof(int));
       sf_varprint(value03);
    //press_to_cont();

     int *value04 = sf_malloc(sizeof(int));
       sf_varprint(value04);

    sf_free(value02);
    sf_free(value03);
    void* value05 = sf_malloc(80);
    sf_realloc(value05,50);

    sf_info(info);
    cr_assert(info->frees == 2, "we only called free twice");   
    cr_assert(info->coalesce == 1, "we coalesce when we free block 2, then block3");
    cr_assert(info->external == 3712);
    }

Test(sf_memsuite, check_errors, .init = sf_mem_init, .fini = sf_mem_fini){

     int *value01 = sf_malloc(sizeof(int));
       sf_varprint(value01);
    //null_check(value01, sizeof(int));
    //press_to_cont();

    void* value02 = sf_malloc(128);//144
     sf_varprint(value02);
    //press_to_cont();
    //allocates for 96 bytes

    int *value03 = sf_malloc(sizeof(int));
       sf_varprint(value03);
    //press_to_cont();

     int *value04 = sf_malloc(sizeof(int));
       sf_varprint(value04);

    sf_free(value02);
    sf_free(value03);
    void* value05 = sf_malloc(80);
    sf_free(value05);
    void* x = sf_realloc(value05,50);
    void * y = sf_malloc(0);
    cr_assert(x == NULL, "you must not allow realloc of a free block");
    cr_assert(y == NULL, "you must not allow malloc of 0");

    }