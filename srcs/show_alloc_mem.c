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

static size_t collect_zones_by_type(enum e_zone_type type,
                                    t_zone *out_zones[],
                                    size_t max)
{
    size_t count = 0;
    t_zone *zone = g_zones;

    for (; zone && count < max; zone = zone->next) {
        if (zone->type == type)
            out_zones[count++] = zone;
    }
    return count;
}

static void print_blocks_in_zone(t_zone *zone, size_t *total)
{
    t_block *block = zone->blocks;
    while (block) {
        if (!block->free) {
            void *start = (void *)(block + 1);
            void *end   = (void *)((char *)start + block->size);
            printf("%p - %p : %zu bytes\n", start, end, block->size);
            *total += block->size;
        }
        block = block->next;
    }
}

static void print_zones(const char *name,
                        t_zone *zones[],
                        size_t count,
                        size_t *total)
{
    for (size_t i = 0; i < count; i++) {
        printf("%s : %p\n", name, (void *)zones[i]);
        print_blocks_in_zone(zones[i], total);
    }
}

void show_alloc_mem(void)
{
    printf("-------------------------------- SHOW_ALLOC_MEM -------------------------------------\n");

    size_t total = 0;
    t_zone *tiny_zones[MAX_ZONES_PER_TYPE];
    t_zone *small_zones[MAX_ZONES_PER_TYPE];
    t_zone *large_zones[MAX_ZONES_PER_TYPE];

    pthread_mutex_lock(&g_mutex);

    size_t tiny_count  = collect_zones_by_type(TINY,  tiny_zones,  MAX_ZONES_PER_TYPE);
    size_t small_count = collect_zones_by_type(SMALL, small_zones, MAX_ZONES_PER_TYPE);
    size_t large_count = collect_zones_by_type(LARGE, large_zones, MAX_ZONES_PER_TYPE);

    sort_zones(tiny_zones,  tiny_count);
    sort_zones(small_zones, small_count);
    sort_zones(large_zones, large_count);

    if (tiny_count > 0)
        print_zones("TINY",  tiny_zones,  tiny_count,  &total);
    if (small_count > 0)
        print_zones("SMALL", small_zones, small_count, &total);
    if (large_count > 0)
        print_zones("LARGE", large_zones, large_count, &total);

    printf("Total : %zu bytes\n", total);
    printf("\n\n\n\n");

    pthread_mutex_unlock(&g_mutex);
}