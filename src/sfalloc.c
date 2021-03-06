#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "../include/sfmm.h"

#include <stdio.h>
#include <string.h>

// Colors
#ifdef COLOR
    #define KNRM  "\x1B[0m"
    #define KRED  "\x1B[1;31m"
    #define KGRN  "\x1B[1;32m"
    #define KYEL  "\x1B[1;33m"
    #define KBLU  "\x1B[1;34m"
    #define KMAG  "\x1B[1;35m"
    #define KCYN  "\x1B[1;36m"
    #define KWHT  "\x1B[1;37m"
    #define KBWN  "\x1B[0;33m"
#else
    /* Color was either not defined or Terminal did not support */
    #define KNRM
    #define KRED
    #define KGRN
    #define KYEL
    #define KBLU
    #define KMAG
    #define KCYN
    #define KWHT
    #define KBWN
#endif

#ifdef DEBUG
    #define debug(S, ...)   fprintf(stdout, KMAG "DEBUG: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define error(S, ...)   fprintf(stderr, KRED "ERROR: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define warn(S, ...)    fprintf(stderr, KYEL "WARN: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define info(S, ...)    fprintf(stdout, KBLU "INFO: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
    #define success(S, ...) fprintf(stdout, KGRN "SUCCESS: %s:%s:%d " KNRM S, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
    #define debug(S, ...)
    #define error(S, ...)   fprintf(stderr, KRED "ERROR: " KNRM S, ##__VA_ARGS__)
    #define warn(S, ...)    fprintf(stderr, KYEL "WARN: " KNRM S, ##__VA_ARGS__)
    #define info(S, ...)    fprintf(stdout, KBLU "INFO: " KNRM S, ##__VA_ARGS__)
    #define success(S, ...) fprintf(stdout, KGRN "SUCCESS: " KNRM S, ##__VA_ARGS__)
#endif

// Define 20 megabytes as the max heap size
#define MAX_HEAP_SIZE (20 * (1 << 20))
#define VALUE1_VALUE 320
#define VALUE2_VALUE 0xDEADBEEFF00D

#define press_to_cont() do { \
    printf("Press Enter to Continue"); \
    while(getchar() != '\n'); \
    printf("\n"); \
} while(0)

#define null_check(ptr, size) do { \
    if ((ptr) == NULL) { \
        error("Failed to allocate %lu byte(s) for an integer using sf_malloc.\n", (size)); \
        error("Aborting...\n"); \
        assert(false); \
    } else { \
        success("sf_malloc returned a non-null address: %p\n", (ptr)); \
    } \
} while(0)

#define payload_check(ptr) do { \
    if ((unsigned long)(ptr) % 16 != 0) { \
        warn("Returned payload address is not divisble by a quadword. %p %% 16 = %lu\n", (ptr), (unsigned long)(ptr) % 16); \
    } \
} while(0)

#define check_prim_contents(actual_value, expected_value, fmt_spec, name) do { \
    if (*(actual_value) != (expected_value)) { \
        error("Expected " name " to be " fmt_spec " but got " fmt_spec "\n", (expected_value), *(actual_value)); \
        error("Aborting...\n"); \
        assert(false); \
    } else { \
        success(name " retained the value of " fmt_spec " after assignment\n", (expected_value)); \
    } \
} while(0)

int main(int argc, char *argv[]) {
    // Initialize the custom allocator
    sf_mem_init(MAX_HEAP_SIZE);


     /*MY OWN TESTCASES!!!!!!!!*/
 
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
    if(info->frees == 2)printf("worked\n"); 
    if(info->coalesce == 1)printf("worked\n");
    sf_snapshot(true);
    printf("%lu\n", info->internal);
    printf("%lu\n", info->external);
    press_to_cont();
    //cr_assert(info->external == )
    //  void *x = sf_malloc(4);
    // void *y = sf_malloc(4);
    // memset(y, 0xFF, 4);
    // sf_varprint(x);
    // sf_free(x);
    // sf_varprint(x);
    // if(freelist_head == x-8) printf("worked\n");;
    // sf_free_header *headofx = (sf_free_header*) (x-8);
    // sf_footer *footofx = (sf_footer*) (x - 8 + (headofx->header.block_size << 4)) - 8;

    // sf_blockprint((sf_free_header*)((void*)x-8));
    // // All of the below should be true if there was no coalescing
    // if(headofx->header.alloc == 0)printf("worked\n");;
    // if(headofx->header.block_size << 4 == 32)printf("worked\n");;
    // if(headofx->header.padding_size == 12)printf("worked\n");;

    // if(footofx->alloc == 0)printf("worked\n");;
    // if(footofx->block_size << 4 == 32)printf("worked\n");;

    
    // int *value01 = sf_malloc(sizeof(int));
    // null_check(value01, sizeof(int));
    // //press_to_cont();

    // void* value02 = sf_malloc(70);
    //  sf_varprint(value02);
    // //press_to_cont();
    // //allocates for 96 bytes

   //  long *value01 = sf_malloc(sizeof(long));
   //  null_check(value01, sizeof(int));
   //   sf_varprint(value01);
   //  //press_to_cont();

   //  void* value02 = sf_malloc(64);//80
   //   sf_varprint(value02);
   //  //press_to_cont();

   //    long *value03 = sf_malloc(sizeof(long));
   //  null_check(value03, sizeof(int));
   //   sf_varprint(value03);
   //   //press_to_cont();

   //  void* value04 = sf_malloc(128);//144
   //  sf_varprint(value04);
   //  //sf_snapshot(true);
   //   //press_to_cont();



   //   //Free a variable
   //  printf("=== Test1: Free a block and snapshot ===\n");
   //  //info("Freeing value1...\n");
   //  sf_free(value03);
   //  sf_snapshot(true);
   // //press_to_cont();

   //  printf("=== Test2: Realloc ===\n");
   //  //info("Freeing value1...\n");
   //  //sf_free(value04);
   //  sf_varprint(sf_realloc(value02,90));// need size 112
   //  sf_snapshot(true);
    
   //  press_to_cont();


   //  long *value05 = sf_malloc(sizeof(long));
   //  null_check(value05, sizeof(int));
   // // press_to_cont();

   //  printf("=== Test1: Perform a snapshot ===\n");
   //  sf_snapshot(true);
   //  //press_to_cont();

    
   //  // Free a variable
   //  printf("=== Test2: Free a block and snapshot ===\n");
   //  info("Freeing value1...\n");
   //  sf_free(value01);
   //  sf_snapshot(true);
   //  press_to_cont();

   //  printf("=== Test2: Free a block and snapshot ===\n");
   //  info("Freeing value1...\n");
   //  sf_free(value04);
   //  sf_snapshot(true);
   //  press_to_cont();

    
   //   // Free a variable
   //  printf("=== Test WE WANT: Free a block and snapshot ===\n");
   //  info("Freeing value1...\n");
   //  sf_free(value04);
   //  sf_snapshot(true);
   //  //press_to_cont();

   //   void* value06 = sf_malloc(100);
   //   sf_varprint(value06);
   //   sf_snapshot(true);
    // press_to_cont();
    
    /*int *value06 = sf_malloc(sizeof(int));
    null_check(value06, sizeof(int));
    sf_snapshot(true);
    press_to_cont(); */

     // Free a variable
    /*
    printf("=== Test3: Free a block and snapshot ===\n");
    info("Freeing value1...\n");
    sf_free(value03);
    sf_snapshot(true);
    press_to_cont();

    // Free a variable
    printf("=== Test4: Free a block and snapshot ===\n");
    info("Freeing value1...\n");
    sf_free(value02);
    sf_snapshot(true);
    press_to_cont();

    
    */
    /*MY OWN TESTCASES!!!!!!!!*/

    // Tell the user about the fields
    info("Initialized heap with %dmb of heap space.\n", MAX_HEAP_SIZE >> 20);
    press_to_cont();

    // Print out title for first test
    printf("=== Test1: Allocation test ===\n");
    // Test #1: Allocate an integer
    int *value1 = sf_malloc(sizeof(int));
    null_check(value1, sizeof(int));
    payload_check(value1);
    // Print out the allocator block
    sf_varprint(value1);
    press_to_cont();

    // Now assign a value
    printf("=== Test2: Assignment test ===\n");
    info("Attempting to assign value1 = %d\n", VALUE1_VALUE);
    // Assign the value
    *value1 = VALUE1_VALUE;
    // Now check its value
    check_prim_contents(value1, VALUE1_VALUE, "%d", "value1");
    press_to_cont();

    printf("=== Test3: Allocate a second variable ===\n");
    info("Attempting to assign value2 = %ld\n", VALUE2_VALUE);
    long *value2 = sf_malloc(sizeof(long));
    null_check(value2, sizeof(long));
    payload_check(value2);
    sf_varprint(value2);
    // Assign a value
    *value2 = VALUE2_VALUE;
    // Check value
    check_prim_contents(value2, VALUE2_VALUE, "%ld", "value2");
    press_to_cont();

   

    printf("=== Test4: does value1 still equal %d ===\n", VALUE1_VALUE);
    check_prim_contents(value1, VALUE1_VALUE, "%d", "value1");
    press_to_cont();

    // Snapshot the freelist
    printf("=== Test5: Perform a snapshot ===\n");
    sf_snapshot(true);
    press_to_cont();

    // Free a variable
    printf("=== Test6: Free a block and snapshot ===\n");
    info("Freeing value1...\n");
    sf_free(value1);
    sf_snapshot(true);
    press_to_cont();

    // Allocate more memory
    printf("=== Test7: 8192 byte allocation ===\n");
    void *memory = sf_malloc(8192);
    sf_varprint(memory);
    sf_free(memory);
    press_to_cont();

    sf_mem_fini();

    return EXIT_SUCCESS;
}
