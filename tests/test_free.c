#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "libft_malloc.h"  // This header declares ft_malloc, ft_free, ft_realloc, show_alloc_mem

void    *ft_malloc(size_t size);
void    ft_free(void *ptr);
void    *ft_realloc(void *ptr, size_t size);
void    show_alloc_mem(void);

//-----------------------------------------------------------------------------
// Test 1: Freeing a NULL pointer should do nothing.
//-----------------------------------------------------------------------------
void test_free_null(void)
{
    printf("Running test_free_null...\n");
    ft_free(NULL);
    printf("test_free_null passed.\n");
}

//-----------------------------------------------------------------------------
// Test 2: Free a single allocated block.
//-----------------------------------------------------------------------------
void test_free_single_block(void)
{
    printf("Running test_free_single_block...\n");
    char *ptr = ft_malloc(50);
    assert(ptr != NULL);
    memset(ptr, 'X', 50);
    ft_free(ptr);
    // Allocate again to see if freed memory can be reused.
    char *ptr2 = ft_malloc(50);
    assert(ptr2 != NULL);
    memset(ptr2, 'Y', 50);
    ft_free(ptr2);
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
    // Allocate three blocks consecutively.
    char *ptr1 = ft_malloc(30);
    char *ptr2 = ft_malloc(30);
    char *ptr3 = ft_malloc(30);
    assert(ptr1 && ptr2 && ptr3);
    
    // Fill with distinct patterns.
    memset(ptr1, 'A', 30);
    memset(ptr2, 'B', 30);
    memset(ptr3, 'C', 30);
    
    // Free first and third blocks.
    ft_free(ptr1);
    ft_free(ptr3);
    // Free the middle block; this should cause the free blocks to merge (coalesce).
    ft_free(ptr2);
    
    // Now allocate a block larger than a single block (but within the coalesced size).
    char *ptr_big = ft_malloc(80);
    assert(ptr_big != NULL);
    memset(ptr_big, 'D', 80);
    ft_free(ptr_big);
    
    printf("test_coalesce_adjacent_blocks passed.\n");
}

//-----------------------------------------------------------------------------
// Test 4: Double free a block.
// Although double free is undefined behavior, our ft_free should not crash.
//-----------------------------------------------------------------------------
void test_double_free(void)
{
    printf("Running test_double_free...\n");
    char *ptr = ft_malloc(50);
    assert(ptr != NULL);
    memset(ptr, 'Z', 50);
    ft_free(ptr);
    // Call ft_free a second time on the same pointer.
    ft_free(ptr);
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
    
    // Allocate many small blocks (e.g., 32 bytes each).
    for (int i = 0; i < NUM_BLOCKS; i++) {
        blocks[i] = ft_malloc(32);
        assert(blocks[i] != NULL);
        memset(blocks[i], 'M', 32);
    }
    
    // Free all blocks sequentially.
    for (int i = 0; i < NUM_BLOCKS; i++) {
        ft_free(blocks[i]);
    }
    
    // Optionally, display the memory state. Zones that are completely free should be unmapped.
    printf("Memory state after freeing many blocks:\n");
    show_alloc_mem();
    
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
        // Allocate a random size between 1 and 256 bytes.
        size_t size = (rand() % 256) + 1;
        ptr = ft_malloc(size);
        assert(ptr != NULL);
        memset(ptr, 0, size);
        ft_free(ptr);
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
    test_double_free();
    test_free_many_blocks();
    test_free_stress();
    printf("All ft_free tests passed successfully.\n");
    return 0;
}
