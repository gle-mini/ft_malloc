#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "libft_malloc.h"

void    *malloc(size_t size);
void    free(void *ptr);
void    *realloc(void *ptr, size_t size);
void    show_alloc_mem(void);

//-----------------------------------------------------------------------------
// Test 1: Freeing a NULL pointer should do nothing.
//-----------------------------------------------------------------------------
void test_free_null(void)
{
    printf("Running test_free_null...\n");
    free(NULL);
    printf("test_free_null passed.\n");
}

//-----------------------------------------------------------------------------
// Test 2: Free a single allocated block.
//-----------------------------------------------------------------------------
void test_free_single_block(void)
{
    printf("Running test_free_single_block...\n");
    char *ptr = malloc(50);
    assert(ptr != NULL);
    memset(ptr, 'X', 50);
    free(ptr);
    char *ptr2 = malloc(50);
    assert(ptr2 != NULL);
    memset(ptr2, 'Y', 50);
    free(ptr2);
    printf("test_free_single_block passed.\n");
}

//-----------------------------------------------------------------------------
// Test 3: Coalesce adjacent blocks.
// Allocate three consecutive blocks, free them in an order that triggers coalescing,
// then try to allocate a larger block that should fit into the coalesced free space.
//-----------------------------------------------------------------------------
void test_coalesce_adjacent_blocks(void)
{
    printf("Running test_coalesce_adjacent_blocks...\n");
    char *ptr1 = malloc(30);
    char *ptr2 = malloc(30);
    char *ptr3 = malloc(30);
    assert(ptr1 && ptr2 && ptr3);
    
    memset(ptr1, 'A', 30);
    memset(ptr2, 'B', 30);
    memset(ptr3, 'C', 30);
    
    free(ptr1);
    free(ptr3);
    free(ptr2);
    
    char *ptr_big = malloc(80);
    assert(ptr_big != NULL);
    memset(ptr_big, 'D', 80);
    free(ptr_big);
    
    printf("test_coalesce_adjacent_blocks passed.\n");
}

//-----------------------------------------------------------------------------
// Test 4: Double free a block.
// Although double free is undefined behavior, our free should not crash.
//-----------------------------------------------------------------------------
void test_double_free(void)
{
    printf("Running test_double_free...\n");
    char *ptr = malloc(50);
    assert(ptr != NULL);
    memset(ptr, 'Z', 50);
    free(ptr);
    free(ptr);
    printf("test_double_free passed.\n");
}

//-----------------------------------------------------------------------------
// Test 5: Free many blocks to force unmapping of an entire zone.
// Allocate many small (TINY) blocks, then free them all. If every block in the zone
// becomes free, the zone should be unmapped.
//-----------------------------------------------------------------------------
void test_free_many_blocks(void)
{
    printf("Running test_free_many_blocks...\n");
    #define NUM_BLOCKS 150
    char *blocks[NUM_BLOCKS];
    
    for (int i = 0; i < NUM_BLOCKS; i++) {
        blocks[i] = malloc(32);
        assert(blocks[i] != NULL);
        memset(blocks[i], 'M', 32);
    }
    
    for (int i = 0; i < NUM_BLOCKS; i++) {
        free(blocks[i]);
    }
    
    printf("Memory state after freeing many blocks:\n");
    show_alloc_mem();
    show_alloc_mem_hex();
    
    #undef NUM_BLOCKS
    printf("test_free_many_blocks passed.\n");
}

//-----------------------------------------------------------------------------
// Test 6: Stress test: Mixed allocations and frees in a loop.
//-----------------------------------------------------------------------------
void test_free_stress(void)
{
    printf("Running test_free_stress...\n");
    #define ITERATIONS 1000
    void *ptr;
    for (int i = 0; i < ITERATIONS; i++) {
        size_t size = (rand() % 256) + 1;
        ptr = malloc(size);
        assert(ptr != NULL);
        memset(ptr, 0, size);
        free(ptr);
    }
    #undef ITERATIONS
    printf("test_free_stress passed.\n");
}

//-----------------------------------------------------------------------------
// Main: Run All Tests
//-----------------------------------------------------------------------------
int main(void)
{
    test_free_null();
    test_free_single_block();
    test_coalesce_adjacent_blocks();
    // test_double_free();
    test_free_many_blocks();
    test_free_stress();
    printf("All free tests passed successfully.\n");
    return 0;
}
