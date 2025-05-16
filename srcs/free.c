#include "libft_malloc.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

extern t_zone *g_zones;
extern pthread_mutex_t g_mutex;

/**
 * @brief Frees the memory block pointed to by ptr.
 *
 * This function retrieves the block header (located immediately before the user pointer),
 * marks the block as free, coalesces adjacent free blocks to reduce fragmentation, and
 * unmaps the zone if all blocks in it are free (for TINY and SMALL zones) or if it is a
 * LARGE allocation.
 *
 * @param ptr Pointer to the memory to be freed. If NULL, no operation is performed.
 */
void free(void *ptr)
{
    t_block *block;
    t_zone *zone;

    if (!ptr)
        return;
    block = (t_block *)ptr - 1;
    // /* only announce the free *once* per allocation */
    // if (! block->free) {
    //     VALGRIND_FREELIKE_BLOCK(ptr, 0);
    //     block->free = 1;
    // }
    pthread_mutex_lock(&g_mutex);
    zone = get_zone_for_ptr((void *)block);
    if (!zone)
    {
        pthread_mutex_unlock(&g_mutex);
        return;
    }

    block->free = 1;
    coalesce(zone);

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
        if (all_free)
        {
            remove_zone(zone);
            munmap(zone, zone->size);
        }
    }
    else if (zone->type == LARGE)
    {
        remove_zone(zone);
        munmap(zone, zone->size);
    }

    pthread_mutex_unlock(&g_mutex);
}
