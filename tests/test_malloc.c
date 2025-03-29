#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "libft_malloc.h"

// Prototype our allocator API (provided by allocator.h)
void    *ft_malloc(size_t size);
void    ft_free(void *ptr);
void    *ft_realloc(void *ptr, size_t size);
void    show_alloc_mem(void);

//-----------------------------------------------------------------------------
// Test 1: Malloc Zero
//-----------------------------------------------------------------------------
void test_malloc_zero(void)
{
    printf("Running test_malloc_zero...\n");
    void *ptr = ft_malloc(0);
    // Our implementation returns NULL when size == 0.
    assert(ptr == NULL);
    printf("test_malloc_zero passed.\n");
}

//-----------------------------------------------------------------------------
// Test 2: Tiny Allocation (< TINY_MAX)
//-----------------------------------------------------------------------------
void test_malloc_tiny(void)
{
    printf("Running test_malloc_tiny...\n");
    size_t size = 32; // well below TINY_MAX (64 bytes)
    char *ptr = ft_malloc(size);
    assert(ptr != NULL);
    // Check that the returned pointer is 8-byte aligned.
    assert(((uintptr_t)ptr & 7) == 0);
    
    // Write to the allocated memory and check that it holds.
    memset(ptr, 0xAB, size);
    for (size_t i = 0; i < size; i++)
        assert(ptr[i] == (char)0xAB);
    
    ft_free(ptr);
    printf("test_malloc_tiny passed.\n");
}

//-----------------------------------------------------------------------------
// Test 3: Tiny Allocation Boundary (exactly TINY_MAX bytes)
//-----------------------------------------------------------------------------
void test_malloc_tiny_boundary(void)
{
    printf("Running test_malloc_tiny_boundary...\n");
    size_t size = 64; // TINY_MAX as defined in our allocator
    char *ptr = ft_malloc(size);
    assert(ptr != NULL);
    assert(((uintptr_t)ptr & 7) == 0);
    
    memset(ptr, 0xCD, size);
    for (size_t i = 0; i < size; i++)
        assert(ptr[i] == (char)0xCD);
    
    ft_free(ptr);
    printf("test_malloc_tiny_boundary passed.\n");
}

//-----------------------------------------------------------------------------
// Test 4: Small Allocation (TINY_MAX < size <= SMALL_MAX)
//-----------------------------------------------------------------------------
void test_malloc_small(void)
{
    printf("Running test_malloc_small...\n");
    size_t size = 128; // within SMALL range (<=1024)
    char *ptr = ft_malloc(size);
    assert(ptr != NULL);
    assert(((uintptr_t)ptr & 7) == 0);
    
    memset(ptr, 0xEF, size);
    for (size_t i = 0; i < size; i++)
        assert(ptr[i] == (char)0xEF);
    
    ft_free(ptr);
    printf("test_malloc_small passed.\n");
}

//-----------------------------------------------------------------------------
// Test 5: Large Allocation (size > SMALL_MAX)
//-----------------------------------------------------------------------------
void test_malloc_large(void)
{
    printf("Running test_malloc_large...\n");
    size_t size = 2048; // greater than SMALL_MAX (1024 bytes)
    char *ptr = ft_malloc(size);
    assert(ptr != NULL);
    assert(((uintptr_t)ptr & 7) == 0);
    
    memset(ptr, 0x12, size);
    for (size_t i = 0; i < size; i++)
        assert(ptr[i] == (char)0x12);
    
    ft_free(ptr);
    printf("test_malloc_large passed.\n");
}

//-----------------------------------------------------------------------------
// Test 6: Multiple Allocations and Non-Overlapping Memory
//-----------------------------------------------------------------------------
void test_malloc_multiple(void)
{
    printf("Running test_malloc_multiple...\n");
    #define NUM_ALLOCS 10
    size_t sizes[NUM_ALLOCS] = {16, 32, 48, 64, 80, 96, 112, 128, 256, 512};
    void *ptrs[NUM_ALLOCS];
    
    for (int i = 0; i < NUM_ALLOCS; i++) {
        ptrs[i] = ft_malloc(sizes[i]);
        assert(ptrs[i] != NULL);
        // Fill memory with a pattern.
        memset(ptrs[i], (int)(i + 1), sizes[i]);
    }
    
    // Check for non-overlap:
    for (int i = 0; i < NUM_ALLOCS; i++) {
        char *start_i = (char *)ptrs[i];
        char *end_i = start_i + sizes[i];
        for (int j = i + 1; j < NUM_ALLOCS; j++) {
            char *start_j = (char *)ptrs[j];
            char *end_j = start_j + sizes[j];
            // Ensure that blocks do not overlap.
            assert(end_i <= start_j || end_j <= start_i);
        }
    }
    
    for (int i = 0; i < NUM_ALLOCS; i++) {
        ft_free(ptrs[i]);
    }
    printf("test_malloc_multiple passed.\n");
    #undef NUM_ALLOCS
}

//-----------------------------------------------------------------------------
// Test 7: Realloc Increase
//-----------------------------------------------------------------------------
void test_realloc_increase(void)
{
    printf("Running test_realloc_increase...\n");
    size_t initial = 32;
    char *ptr = ft_malloc(initial);
    assert(ptr != NULL);
    
    // Fill with a known pattern.
    memset(ptr, 'A', initial);
    
    // Increase size.
    char *new_ptr = ft_realloc(ptr, 64);
    assert(new_ptr != NULL);
    // The first 32 bytes should be preserved.
    for (size_t i = 0; i < initial; i++)
        assert(new_ptr[i] == 'A');
    
    ft_free(new_ptr);
    printf("test_realloc_increase passed.\n");
}

//-----------------------------------------------------------------------------
// Test 8: Realloc Decrease
//-----------------------------------------------------------------------------
void test_realloc_decrease(void)
{
    printf("Running test_realloc_decrease...\n");
    size_t initial = 64;
    char *ptr = ft_malloc(initial);
    assert(ptr != NULL);
    
    memset(ptr, 'B', initial);
    
    // Decrease size.
    char *new_ptr = ft_realloc(ptr, 32);
    assert(new_ptr != NULL);
    for (size_t i = 0; i < 32; i++)
        assert(new_ptr[i] == 'B');
    
    ft_free(new_ptr);
    printf("test_realloc_decrease passed.\n");
}

//-----------------------------------------------------------------------------
// Test 9: Realloc with NULL pointer (should behave like malloc)
//-----------------------------------------------------------------------------
void test_realloc_null(void)
{
    printf("Running test_realloc_null...\n");
    char *ptr = ft_realloc(NULL, 128);
    assert(ptr != NULL);
    memset(ptr, 'C', 128);
    for (size_t i = 0; i < 128; i++)
        assert(ptr[i] == 'C');
    
    ft_free(ptr);
    printf("test_realloc_null passed.\n");
}

//-----------------------------------------------------------------------------
// Test 10: Realloc with size zero (should free the pointer and return NULL)
//-----------------------------------------------------------------------------
void test_realloc_zero(void)
{
    printf("Running test_realloc_zero...\n");
    char *ptr = ft_malloc(128);
    assert(ptr != NULL);
    char *new_ptr = ft_realloc(ptr, 0);
    // Our implementation frees the block and returns NULL.
    assert(new_ptr == NULL);
    printf("test_realloc_zero passed.\n");
}

//-----------------------------------------------------------------------------
// Test 11: show_alloc_mem Function
//-----------------------------------------------------------------------------
void test_show_alloc_mem(void)
{
    printf("Running test_show_alloc_mem...\n");
    // Allocate several blocks.
    ft_malloc(32);
    ft_malloc(64);
    ft_malloc(2048);
    // Display the current memory zones.
    printf("Memory state:\n");
    show_alloc_mem();
    printf("test_show_alloc_mem passed.\n");
}

//-----------------------------------------------------------------------------
// Main: Run All Tests
//-----------------------------------------------------------------------------
int main(void)
{
    test_malloc_zero();
    test_malloc_tiny();
    test_malloc_tiny_boundary();
    test_malloc_small();
    test_malloc_large();
    test_malloc_multiple();
    test_realloc_increase();
    test_realloc_decrease();
    test_realloc_null();
    test_realloc_zero();
    test_show_alloc_mem();
    printf("All malloc tests passed successfully.\n");
    return 0;
}
