#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "libft_malloc.h"

void    *malloc(size_t size);
void    free(void *ptr);
void    *realloc(void *ptr, size_t size);
void    show_alloc_mem(void);

//-----------------------------------------------------------------------------
// Test 2: Tiny Allocation (< TINY_MAX)
//-----------------------------------------------------------------------------
void test_malloc_tiny(void)
{
    printf("Running test_malloc_tiny...\n");
    size_t size = 32;
    char *ptr = malloc(size);
    assert(ptr != NULL);
    assert(((uintptr_t)ptr & 7) == 0);
    
    memset(ptr, 0xAB, size);
    for (size_t i = 0; i < size; i++)
        assert(ptr[i] == (char)0xAB);
    
    free(ptr);
    printf("test_malloc_tiny passed.\n");
}

//-----------------------------------------------------------------------------
// Test 3: Tiny Allocation Boundary (exactly TINY_MAX bytes)
//-----------------------------------------------------------------------------
void test_malloc_tiny_boundary(void)
{
    printf("Running test_malloc_tiny_boundary...\n");
    size_t size = 64;
    char *ptr = malloc(size);
    assert(ptr != NULL);
    assert(((uintptr_t)ptr & 7) == 0);
    
    memset(ptr, 0xCD, size);
    for (size_t i = 0; i < size; i++)
        assert(ptr[i] == (char)0xCD);
    
    free(ptr);
    printf("test_malloc_tiny_boundary passed.\n");
}

//-----------------------------------------------------------------------------
// Test 4: Small Allocation (TINY_MAX < size <= SMALL_MAX)
//-----------------------------------------------------------------------------
void test_malloc_small(void)
{
    printf("Running test_malloc_small...\n");
    size_t size = 128;
    char *ptr = malloc(size);
    assert(ptr != NULL);
    assert(((uintptr_t)ptr & 7) == 0);
    
    memset(ptr, 0xEF, size);
    for (size_t i = 0; i < size; i++)
        assert(ptr[i] == (char)0xEF);
    
    free(ptr);
    printf("test_malloc_small passed.\n");
}

//-----------------------------------------------------------------------------
// Test 5: Large Allocation (size > SMALL_MAX)
//-----------------------------------------------------------------------------
void test_malloc_large(void)
{
    printf("Running test_malloc_large...\n");
    size_t size = 2048;
    char *ptr = malloc(size);
    assert(ptr != NULL);
    assert(((uintptr_t)ptr & 7) == 0);
    
    memset(ptr, 0x12, size);
    for (size_t i = 0; i < size; i++)
        assert(ptr[i] == (char)0x12);
    
    free(ptr);
    printf("test_malloc_large passed.\n");
}

void test_malloc_very_large(void)
{
    printf("Running test_malloc_very_large...\n");
    size_t size = 100000;
    char *ptr = malloc(size);
    assert(ptr != NULL);
    assert(((uintptr_t)ptr & 7) == 0);

    memset(ptr, 0x15, size);
    for (size_t i = 0; i < size; i++)
        assert(ptr[i] == (char)0x15);
    
    free(ptr);
    printf("test_malloc_very_large passed.\n");
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
        ptrs[i] = malloc(sizes[i]);
        assert(ptrs[i] != NULL);
        memset(ptrs[i], (int)(i + 1), sizes[i]);
    }

    for (int i = 0; i < NUM_ALLOCS; i++) {
        char *start_i = (char *)ptrs[i];
        char *end_i = start_i + sizes[i];
        for (int j = i + 1; j < NUM_ALLOCS; j++) {
            char *start_j = (char *)ptrs[j];
            char *end_j = start_j + sizes[j];
            assert(end_i <= start_j || end_j <= start_i);
        }
    }
    
    for (int i = 0; i < NUM_ALLOCS; i++) {
        free(ptrs[i]);
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
    char *ptr = malloc(initial);
    assert(ptr != NULL);
    memset(ptr, 'A', initial);
    char *new_ptr = realloc(ptr, 64);
    assert(new_ptr != NULL);
    for (size_t i = 0; i < initial; i++)
        assert(new_ptr[i] == 'A');
    
    free(new_ptr);
    printf("test_realloc_increase passed.\n");
}

//-----------------------------------------------------------------------------
// Test 8: Realloc Decrease
//-----------------------------------------------------------------------------
void test_realloc_decrease(void)
{
    printf("Running test_realloc_decrease...\n");
    size_t initial = 64;
    char *ptr = malloc(initial);
    assert(ptr != NULL);
    
    memset(ptr, 'B', initial);
    
    char *new_ptr = realloc(ptr, 32);
    assert(new_ptr != NULL);
    for (size_t i = 0; i < 32; i++)
        assert(new_ptr[i] == 'B');
    
    free(new_ptr);
    printf("test_realloc_decrease passed.\n");
}

//-----------------------------------------------------------------------------
// Test 9: Realloc with NULL pointer (should behave like malloc)
//-----------------------------------------------------------------------------
void test_realloc_null(void)
{
    printf("Running test_realloc_null...\n");
    char *ptr = realloc(NULL, 128);
    assert(ptr != NULL);
    memset(ptr, 'C', 128);
    for (size_t i = 0; i < 128; i++)
        assert(ptr[i] == 'C');
    
    free(ptr);
    printf("test_realloc_null passed.\n");
}

//-----------------------------------------------------------------------------
// Test 10: Realloc with size zero (should free the pointer and return NULL)
//-----------------------------------------------------------------------------
void test_realloc_zero(void)
{
    printf("Running test_realloc_zero...\n");
    char *ptr = malloc(128);
    printf("First malloc :\n");
    show_alloc_mem();
    assert(ptr != NULL);
    printf("Realloc :\n");
    show_alloc_mem();
    char *new_ptr = realloc(ptr, 0);
    assert(new_ptr == NULL);
    printf("test_realloc_zero passed.\n");
}

//-----------------------------------------------------------------------------
// Test 11: show_alloc_mem Function
//-----------------------------------------------------------------------------
 void test_show_alloc_mem(void)
 {
    printf("Running test_show_alloc_mem...\n");
    void *a = malloc(32);
    void *b = malloc(64);
    void *c = malloc(2048);
    assert(a && b && c);
    printf("Memory state:\n");
    show_alloc_mem();
    free(a);
    free(b);
    free(c);
    printf("test_show_alloc_mem passed.\n");
 }
 
 void test_show_alloc_mem_hex(void)
 {
    printf("Running test_show_alloc_mem_hex...\n");
    void *a = malloc(32);
    void *b = malloc(64);
    void *c = malloc(2048);
    assert(a && b && c);
    printf("Memory state:\n");
    show_alloc_mem_hex();
    free(a);
    free(b);
    free(c);
    printf("test_show_alloc_mem_hex passed.\n");
 }

//-----------------------------------------------------------------------------
// Main: Run All Tests
//-----------------------------------------------------------------------------
int main(void)
{
    test_malloc_tiny();
    test_malloc_tiny_boundary();
    test_malloc_small();
    test_malloc_large();
    test_malloc_very_large();
    test_malloc_multiple();
    test_realloc_increase();
    test_realloc_decrease();
    test_realloc_null();
    test_realloc_zero();
    test_show_alloc_mem();
    printf("All malloc tests passed successfully.\n");
    return 0;
}
