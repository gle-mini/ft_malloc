#include "libft_malloc.h"
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>

t_zone *g_zones = NULL;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

//=============================================================================
// Helper Functions
//=============================================================================


/**
 * @brief Creates a new memory zone using mmap and initializes its first block.
 *
 * This function maps a new memory region of the given size, sets the zone type, and
 * initializes the first block header covering the remainder of the zone.
 *
 * @param type The type of the memory zone (TINY, SMALL, or LARGE).
 * @param zone_size The total size in bytes for the new zone.
 * @return Pointer to the created t_zone structure, or NULL if mmap fails.
 */
static t_zone *create_zone(t_zone_type type, size_t zone_size)
{
    t_zone *zone = mmap(NULL, zone_size, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (zone == MAP_FAILED)
        return NULL;
    zone->type = type;
    zone->size = zone_size;
    zone->next = NULL;
    zone->blocks = (t_block *)((char *)zone + sizeof(t_zone));
    zone->blocks->size = zone_size - sizeof(t_zone) - BLOCK_SIZE;
    zone->blocks->free = 1;
    zone->blocks->next = NULL;
    zone->blocks->prev = NULL;
    return zone;
}


/**
 * @brief Adds a memory zone to the global zones list.
 *
 * This function prepends the provided zone to the global linked list of zones.
 *
 * @param zone Pointer to the memory zone to be added.
 */
static void add_zone(t_zone *zone)
{
    zone->next = g_zones;
    g_zones = zone;
}

/**
 * @brief Removes a memory zone from the global zones list.
 *
 * This function searches the global zones list and removes the specified zone.
 *
 * @param zone Pointer to the memory zone to be removed.
 */
void remove_zone(t_zone *zone)
{
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
}

/**
 * @brief Finds a free block within a zone that fits at least 'size' bytes.
 *
 * Iterates through the linked list of blocks in a given zone and returns the first
 * block that is free and has enough size.
 *
 * @param zone Pointer to the memory zone to search.
 * @param size The minimum number of bytes required.
 * @return Pointer to a suitable free block, or NULL if none is found.
 */
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


/**
 * @brief Finds a free block of a given type in the global zones that fits 'size' bytes.
 *
 * Searches the global zones list for a zone of the specified type and then finds a free block
 * within that zone that can accommodate the requested size.
 *
 * @param type The type of zone (TINY, SMALL, or LARGE) to search in.
 * @param size The number of bytes required.
 * @param zone_found Optional output parameter to return the zone where the block was found.
 * @return Pointer to a free block, or NULL if no suitable block exists.
 */
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

/**
 * @brief Splits a free block if it is considerably larger than requested into an allocated block and a residual free block.
 *
 * If the free block has significantly more space than requested, it is split into:
 * - An allocated block of exactly 'size' bytes.
 * - A new free block that contains the remaining space.
 *
 * @param block Pointer to the free block to be split.
 * @param size The requested allocation size.
 */
static void split_block(t_block *block, size_t size)
{
    if (block->size >= size + BLOCK_SIZE + 8)
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

/**
 * @brief Determines the memory zone that contains the given pointer.
 *
 * Iterates through the global zones list to find which zone holds the address specified by ptr.
 *
 * @param ptr Pointer assumed to be part of a memory block header.
 * @return Pointer to the corresponding zone, or NULL if not found.
 */
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

/**
 * @brief Coalesces adjacent free blocks within the specified zone.
 *
 * Merges contiguous free blocks into a single larger free block in order to
 * reduce fragmentation.
 *
 * @param zone Pointer to the memory zone where coalescing is to be performed.
 */
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

/**
 * @brief Allocates "size" bytes of memory.
 *
 * This allocator first aligns the requested size to 8 bytes. Depending on the size,
 * it allocates from a TINY, SMALL, or LARGE memory zone. If no suitable free block is available,
 * it creates a new zone via mmap (except for LARGE allocations, which get their own).
 *
 * @param size Number of bytes to allocate.
 * @return Pointer to the allocated memory, or NULL if allocation fails or size is 0.
 */
void *malloc(size_t size)
{
    t_zone *zone_found = NULL;
    t_block *block = NULL;
    size_t aligned_size;

    if (size == 0)
        size = 1;
    
    // Align size to 8 bytes.
    aligned_size = (size + 7) & ~7;

    pthread_mutex_lock(&g_mutex);

    if (aligned_size <= TINY_MAX)
    {
        block = find_free_block(TINY, aligned_size, &zone_found);
        if (!block)
        {
            zone_found = create_zone(TINY, TINY_ZONE_SIZE);
            if (!zone_found) {
                pthread_mutex_unlock(&g_mutex);
                return NULL;
            }
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
        size_t total_size = sizeof(t_zone) + BLOCK_SIZE + aligned_size;
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
    split_block(block, aligned_size);
    block->free = 0;
    // void *user_ptr = (void*)(block + 1);
    // VALGRIND_MALLOCLIKE_BLOCK(user_ptr, aligned_size, 0, 0);
    
    pthread_mutex_unlock(&g_mutex);
    return (void *)(block + 1);
}

/**
 * @brief Reallocates the given memory block to a new size.
 *
 * If the new size is less than or equal to the current block's size, the block is
 * split (if possible). Otherwise, a new block is allocated, the data copied, and the old block freed.
 *
 * @param ptr Pointer to the existing memory block (or NULL, in which case ft_malloc is called).
 * @param size The new size in bytes for the reallocation.
 * @return Pointer to the reallocated memory block, or NULL if allocation fails.
 */
void *realloc(void *ptr, size_t size)
{
    if (!ptr)
        return malloc(size);
    if (size == 0)
    {
        free(ptr);
        return NULL;
    }
    
    t_block *block = (t_block *)ptr - 1;
    size_t aligned_size = (size + 7) & ~7;
    
    if (block->size >= aligned_size)
    {
        split_block(block, aligned_size);
        return ptr;
    }
    
    void *new_ptr = malloc(size);
    if (!new_ptr)
        return NULL;
    size_t copy_size = (block->size < aligned_size) ? block->size : aligned_size;
    memcpy(new_ptr, ptr, copy_size);
    free(ptr);
    return new_ptr;
}