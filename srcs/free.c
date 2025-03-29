#include "libft_malloc.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

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

    // Retrieve the block header; user pointer is located right after the header.
    block = (t_block *)ptr - 1;

    // Find the zone that contains this block.
    zone = get_zone_for_ptr((void *)block);
    if (!zone)
        return;

    // Mark the block as free.
    block->free = 1;

    // Attempt to merge (coalesce) adjacent free blocks.
    coalesce(zone);

    // If the allocation belongs to a TINY or SMALL zone, check if every block is free.
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
            if (zone->type == TINY)
                remove_zone(&g_tiny_zones, zone);
            else  // SMALL
                remove_zone(&g_small_zones, zone);

            munmap(zone, zone->size);
        }
    }
    // For LARGE allocations, unmap the zone immediately.
    else if (zone->type == LARGE)
    {
        remove_zone(&g_large_zones, zone);
        munmap(zone, zone->size);
    }
}
