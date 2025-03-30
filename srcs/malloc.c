#include "libft_malloc.h"
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>


// t_zone *g_tiny_zones  = NULL;
// t_zone *g_small_zones = NULL;
// t_zone *g_large_zones = NULL;

t_zone *g_zones = NULL;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

//=============================================================================
// Helper Functions
//=============================================================================

// Create a new zone using mmap and initialize its first block.
static t_zone *create_zone(t_zone_type type, size_t zone_size)
{
    t_zone *zone = mmap(NULL, zone_size, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (zone == MAP_FAILED)
        return NULL;
    zone->type = type;
    zone->size = zone_size;
    zone->next = NULL;
    // The zone header is at the start of the region. Immediately after,
    // we initialize the first block for allocations.
    zone->blocks = (t_block *)((char *)zone + sizeof(t_zone));
    zone->blocks->size = zone_size - sizeof(t_zone) - BLOCK_SIZE;
    zone->blocks->free = 1;
    zone->blocks->next = NULL;
    zone->blocks->prev = NULL;
    return zone;
}

// Add a zone to a global list.
static void add_zone(t_zone *zone)
{
    zone->next = g_zones;
    g_zones = zone;
}

// Remove a zone from a global list.
void remove_zone(t_zone *zone)
{
    // pthread_mutex_lock(&g_mutex);
    t_zone *prev = NULL;
    t_zone *curr = g_zones;
    while (curr)
    {
        if (curr == zone)
        {
            if (prev)
                prev->next = curr->next;
            else
                g_zones = curr->next;
            break;
        }
        prev = curr;
        curr = curr->next;
    }
    // pthread_mutex_unlock(&g_mutex);
}
// Find a free block in the given zone that fits at least 'size' bytes.
static t_block *find_free_block_in_zone(t_zone *zone, size_t size)
{
    t_block *block = zone->blocks;
    while (block)
    {
        if (block->free && block->size >= size)
            return block;
        block = block->next;
    }
    return NULL;
}

static t_block *find_free_block(t_zone_type type, size_t size, t_zone **zone_found)
{
    t_zone *zone = g_zones;
    t_block *block = NULL;

    while (zone)
    {
        if (zone->type == type)
        {
            block = find_free_block_in_zone(zone, size);
            if (block)
            {
                if (zone_found)
                    *zone_found = zone;
                return block;
            }
        }
        zone = zone->next;
    }
    return NULL;
}

// Split a block if it is considerably larger than requested.
static void split_block(t_block *block, size_t size)
{
    if (block->size >= size + BLOCK_SIZE + 8) // leave minimal room for a new block
    {
        t_block *new_block = (t_block *)((char *)block + BLOCK_SIZE + size);
        new_block->size = block->size - size - BLOCK_SIZE;
        new_block->free = 1;
        new_block->next = block->next;
        new_block->prev = block;
        if (new_block->next)
            new_block->next->prev = new_block;
        block->size = size;
        block->next = new_block;
    }
}

// Given a pointer to a block (user data), find its parent zone.
t_zone *get_zone_for_ptr(void *ptr)
{
    t_zone *zone = g_zones;
    while (zone)
    {
        if ((char *)ptr > (char *)zone && (char *)ptr < ((char *)zone + zone->size))
            return zone;
        zone = zone->next;
    }
    return NULL;
}

// Coalesce adjacent free blocks within a zone.
void coalesce(t_zone *zone)
{
    t_block *block = zone->blocks;
    while (block)
    {
        if (block->free && block->next && block->next->free)
        {
            block->size += BLOCK_SIZE + block->next->size;
            block->next = block->next->next;
            if (block->next)
                block->next->prev = block;
            continue;
        }
        block = block->next;
    }
}

//=============================================================================
// Allocator API Functions
//=============================================================================

void *ft_malloc(size_t size)
{
    t_zone *zone_found = NULL;
    t_block *block = NULL;
    size_t aligned_size;

    if (size == 0)
        return NULL;
    
    // Align size to 8 bytes.
    aligned_size = (size + 7) & ~7;

    // Lock the global mutex before accessing g_zones.
    pthread_mutex_lock(&g_mutex);

    if (aligned_size <= TINY_MAX)
    {
        block = find_free_block(TINY, aligned_size, &zone_found);
        if (!block)
        {
            // No suitable block found; create a new TINY zone.
            zone_found = create_zone(TINY, TINY_ZONE_SIZE);
            if (!zone_found) {
                pthread_mutex_unlock(&g_mutex);
                return NULL;
            }
            // add_zone now should not lock internally.
            add_zone(zone_found);
            block = zone_found->blocks;
        }
    }
    else if (aligned_size <= SMALL_MAX)
    {
        block = find_free_block(SMALL, aligned_size, &zone_found);
        if (!block)
        {
            zone_found = create_zone(SMALL, SMALL_ZONE_SIZE);
            if (!zone_found) {
                pthread_mutex_unlock(&g_mutex);
                return NULL;
            }
            add_zone(zone_found);
            block = zone_found->blocks;
        }
    }
    else
    {
        // LARGE allocation: each gets its own mmap.
        size_t total_size = BLOCK_SIZE + aligned_size;
        t_zone *zone = mmap(NULL, total_size, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (zone == MAP_FAILED) {
            pthread_mutex_unlock(&g_mutex);
            return NULL;
        }
        zone->type = LARGE;
        zone->size = total_size;
        zone->next = NULL;
        zone->blocks = (t_block *)((char *)zone + sizeof(t_zone));
        zone->blocks->size = aligned_size;
        zone->blocks->free = 0;
        zone->blocks->next = NULL;
        zone->blocks->prev = NULL;
        add_zone(zone);
        pthread_mutex_unlock(&g_mutex);
        return (void *)(zone->blocks + 1);
    }
    
    // At this point, we have a free block from a TINY or SMALL zone.
    split_block(block, aligned_size);
    block->free = 0;
    
    pthread_mutex_unlock(&g_mutex);
    return (void *)(block + 1);
}

void *ft_realloc(void *ptr, size_t size)
{
    if (!ptr)
        return ft_malloc(size);  // Use our custom ft_malloc
    if (size == 0)
    {
        ft_free(ptr);            // Use our custom ft_free
        return NULL;
    }
    
    t_block *block = (t_block *)ptr - 1;
    size_t aligned_size = (size + 7) & ~7;
    
    if (block->size >= aligned_size)
    {
        split_block(block, aligned_size);
        return ptr;
    }
    
    // Otherwise, allocate a new block, copy the data, and free the old block.
    void *new_ptr = ft_malloc(size);  // Use our custom ft_malloc
    if (!new_ptr)
        return NULL;
    size_t copy_size = (block->size < aligned_size) ? block->size : aligned_size;
    memcpy(new_ptr, ptr, copy_size);
    ft_free(ptr);  // Use our custom ft_free
    return new_ptr;
}

void show_alloc_mem(void)
{
    t_zone *zone;
    t_block *block;
    size_t total = 0;
    
    // Display TINY zones.
    zone = g_zones;
    printf("TINY :\n");
    while (zone)
    {
        printf("%p - %p : Zone size %zu\n", zone, (char *)zone + zone->size, zone->size);
        block = zone->blocks;
        while (block)
        {
            if (!block->free)
            {
                printf("%p - %p : %zu bytes\n", (void *)(block + 1),
                       (void *)((char *)(block + 1) + block->size), block->size);
                total += block->size;
            }
            block = block->next;
        }
        zone = zone->next;
    }
    
    // Display SMALL zones.
    zone = g_zones;
    printf("SMALL :\n");
    while (zone)
    {
        printf("%p - %p : Zone size %zu\n", zone, (char *)zone + zone->size, zone->size);
        block = zone->blocks;
        while (block)
        {
            if (!block->free)
            {
                printf("%p - %p : %zu bytes\n", (void *)(block + 1),
                       (void *)((char *)(block + 1) + block->size), block->size);
                total += block->size;
            }
            block = block->next;
        }
        zone = zone->next;
    }
    
    // Display LARGE zones.
    zone = g_zones;
    printf("LARGE :\n");
    while (zone)
    {
        printf("%p - %p : Zone size %zu\n", zone, (char *)zone + zone->size, zone->size);
        block = zone->blocks;
        if (!block->free)
        {
            printf("%p - %p : %zu bytes\n", (void *)(block + 1),
                   (void *)((char *)(block + 1) + block->size), block->size);
            total += block->size;
        }
        zone = zone->next;
    }
    printf("Total : %zu bytes\n", total);
}
