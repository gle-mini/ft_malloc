#include "libft_malloc.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_ZONES_PER_TYPE 128

static void sort_zones(t_zone *zones[], size_t count)
{
    size_t i, j;
    t_zone *temp;
    for (i = 1; i < count; i++) {
        temp = zones[i];
        j = i;
        while (j > 0 && zones[j - 1] > temp) {
            zones[j] = zones[j - 1];
            j--;
        }
        zones[j] = temp;
    }
}

/*
 * show_alloc_mem - displays the current state of allocated memory
 *
 * For each memory zone (TINY, SMALL and LARGE) the function prints:
 *
 *   [ZONE TYPE] : [Zone starting address]
 *   [block start address] - [block end address] : [block size] bytes
 *
 * Finally, the total of allocated bytes is printed.
 */
void show_alloc_mem(void)
{
    printf("-------------------------------- SHOW_ALLOC_MEM -------------------------------------\n");

    t_zone *zone;
    size_t total = 0;

    // Temporary arrays to hold zone pointers by type.
    t_zone *tiny_zones[MAX_ZONES_PER_TYPE];
    t_zone *small_zones[MAX_ZONES_PER_TYPE];
    t_zone *large_zones[MAX_ZONES_PER_TYPE];
    size_t tiny_count = 0, small_count = 0, large_count = 0;

    // Lock the global mutex to obtain a consistent snapshot of the zones.
    pthread_mutex_lock(&g_mutex);

    // First, walk the global list to group zones by type.
    for (zone = g_zones; zone; zone = zone->next) {
        if (zone->type == TINY && tiny_count < MAX_ZONES_PER_TYPE)
            tiny_zones[tiny_count++] = zone;
        else if (zone->type == SMALL && small_count < MAX_ZONES_PER_TYPE)
            small_zones[small_count++] = zone;
        else if (zone->type == LARGE && large_count < MAX_ZONES_PER_TYPE)
            large_zones[large_count++] = zone;
    }

    sort_zones(tiny_zones, tiny_count);
    sort_zones(small_zones, small_count);
    sort_zones(large_zones, large_count);

    if (tiny_count > 0) {
        for (size_t i = 0; i < tiny_count; i++) {
            printf("TINY : %p\n", (void *)tiny_zones[i]);
            t_block *block = tiny_zones[i]->blocks;
            while (block) {
                if (!block->free) {
                    void *block_start = (void *)(block + 1);
                    void *block_end   = (void *)((char *)block_start + block->size);
                    printf("%p - %p : %zu bytes\n", block_start, block_end, block->size);
                    total += block->size;
                }
                block = block->next;
            }
        }
    }

    if (small_count > 0) {
        for (size_t i = 0; i < small_count; i++) {
            printf("SMALL : %p\n", (void *)small_zones[i]);
            t_block *block = small_zones[i]->blocks;
            while (block) {
                if (!block->free) {
                    void *block_start = (void *)(block + 1);
                    void *block_end   = (void *)((char *)block_start + block->size);
                    printf("%p - %p : %zu bytes\n", block_start, block_end, block->size);
                    total += block->size;
                }
                block = block->next;
            }
        }
    }

    if (large_count > 0) {
        for (size_t i = 0; i < large_count; i++) {
            printf("LARGE : %p\n", (void *)large_zones[i]);
            t_block *block = large_zones[i]->blocks;
            if (block && !block->free) {
                void *block_start = (void *)(block + 1);
                void *block_end   = (void *)((char *)block_start + block->size);
                printf("%p - %p : %zu bytes\n", block_start, block_end, block->size);
                total += block->size;
            }
        }
    }

    printf("Total : %zu bytes\n", total);
    printf("\n\n\n\n");
    pthread_mutex_unlock(&g_mutex);
}
