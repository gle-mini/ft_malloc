#include "libft_malloc.h"
#include <stdio.h>
#include <pthread.h>

// Helper to print a line of hex bytes with address
static void print_hex_line(const unsigned char *data, size_t len, const void *addr) {
    printf("%p  ", addr);
    for (size_t i = 0; i < len; ++i) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}

// Dump the contents of a block in hex, 16 bytes per line
static void hex_dump_block(const void *start, size_t size) {
    const unsigned char *data = (const unsigned char *)start;
    size_t offset = 0;
    while (offset < size) {
        size_t line_len = (size - offset >= 16) ? 16 : (size - offset);
        print_hex_line(data + offset, line_len, data + offset);
        offset += line_len;
    }
}

/**
 * show_alloc_mem_hex - prints a hexadecimal dump of all allocated blocks
 * in all zones (TINY, SMALL, LARGE).
 */
void show_alloc_mem_hex(void) {
    pthread_mutex_lock(&g_mutex);
    printf("------ HEX DUMP OF ALLOCATED ZONES ------\n");
    t_zone *zone = g_zones;
    while (zone) {
        const char *type_str = (zone->type == TINY ? "TINY" : 
                               (zone->type == SMALL ? "SMALL" : "LARGE"));
        printf("%s zone at %p (size %zu):\n", type_str, (void *)zone, zone->size);
        t_block *block = zone->blocks;
        while (block) {
            if (!block->free) {
                void *start = (void *)(block + 1);
                printf("Block at %p - %zu bytes:\n", start, block->size);
                hex_dump_block(start, block->size);
            }
            block = block->next;
        }
        zone = zone->next;
    }
    printf("------------------------------------------\n");
    pthread_mutex_unlock(&g_mutex);
}
