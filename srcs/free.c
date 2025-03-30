#include "libft_malloc.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

// Assume these globals are declared in your header or elsewhere.
extern t_zone *g_zones;
extern pthread_mutex_t g_mutex;

/*
 * ft_free:
 *   Frees the memory block pointed to by ptr. The function:
 *   - Returns immediately if ptr is NULL.
 *   - Retrieves the block header (which is located immediately before the user pointer).
 *   - Marks the block as free.
 *   - Coalesces adjacent free blocks to reduce fragmentation.
 *   - For TINY and SMALL zones, if all blocks in the zone are free, the entire zone is unmapped.
 *   - For LARGE allocations (which are mmap’d individually), the zone is unmapped immediately.
 */
void ft_free(void *ptr)
{
    t_block *block;
    t_zone *zone;

    if (!ptr)
        return;

    // Retrieve the block header; the user pointer is located right after the header.
    block = (t_block *)ptr - 1;

    // Lock the global mutex to protect the global zones list.
    pthread_mutex_lock(&g_mutex);

    // Find the zone that contains this block from the unified global list.
    zone = get_zone_for_ptr((void *)block);
    if (!zone)
    {
        pthread_mutex_unlock(&g_mutex);
        return;
    }

    // Mark the block as free.
    block->free = 1;

    // Attempt to merge (coalesce) adjacent free blocks.
    coalesce(zone);

    // For TINY and SMALL zones, check if every block is free.
    if (zone->type == TINY || zone->type == SMALL)
    {
        t_block *b = zone->blocks;
        int all_free = 1;
        while (b)
        {
            if (!b->free)
            {
                all_free = 0;
                break;
            }
            b = b->next;
        }
        // If the entire zone is free, remove it from the global list and unmap the zone.
        if (all_free)
        {
            remove_zone(zone);  // remove_zone() now uses the single global list (g_zones).
            munmap(zone, zone->size);
        }
    }
    // For LARGE allocations, unmap the zone immediately.
    else if (zone->type == LARGE)
    {
        remove_zone(zone);
        munmap(zone, zone->size);
    }

    pthread_mutex_unlock(&g_mutex);
}
