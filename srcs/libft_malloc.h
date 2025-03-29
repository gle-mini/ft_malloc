#ifndef LIBFT_MALLOC_H
# define LIBTF_MALLOC_H


#define TINY_MAX        64
#define SMALL_MAX       1024

// Zone sizes are multiples of the system page size.
#define TINY_ZONE_MULTIPLIER   16
#define SMALL_ZONE_MULTIPLIER  128

// These macros compute the zone sizes.
#define TINY_ZONE_SIZE   (sysconf(_SC_PAGESIZE) * TINY_ZONE_MULTIPLIER)
#define SMALL_ZONE_SIZE  (sysconf(_SC_PAGESIZE) * SMALL_ZONE_MULTIPLIER)

# include <stdlib.h>
# include <stddef.h>
# include <sys/types.h>


//=============================================================================
// Data Structures
//=============================================================================

typedef enum e_zone_type {
    TINY,
    SMALL,
    LARGE
} t_zone_type;

// Block header structure: each allocated block has a header.
typedef struct s_block {
    size_t          size;   // Size of the payload (user data)
    int             free;   // 1 if free, 0 if allocated
    struct s_block  *next;
    struct s_block  *prev;
} t_block;

#define BLOCK_SIZE (sizeof(t_block))

// Zone structure: represents a memory zone allocated with mmap.
typedef struct s_zone {
    t_zone_type     type;
    size_t          size;   // Total size of the zone (including header)
    struct s_zone   *next;
    t_block         *blocks; // Pointer to the first block in the zone
} t_zone;

//=============================================================================
// Global Zone Lists
//=============================================================================

extern t_zone *g_tiny_zones;
extern t_zone *g_small_zones;
extern t_zone *g_large_zones;

/*
 * Allocates "size" bytes of memory and returns a pointer to the allocated memory.
 */
void	*ft_malloc(size_t size);

/*
 * Deallocates the memory allocation pointed to by "ptr".
 * If "ptr" is a NULL pointer, no operation is performed.
 */
void	ft_free(void *ptr);

/*
 * Changes the size of the memory allocation pointed to by "ptr" to "size".
 * If there is not enough room to enlarge the allocation, a new allocation is created,
 * the old data is copied, and the old allocation is freed.
 */
void	*ft_realloc(void *ptr, size_t size);

/*
 * Displays the current state of the allocated memory zones.
 */
void	show_alloc_mem(void);


t_zone *get_zone_for_ptr(void *ptr);
void coalesce(t_zone *zone);
void remove_zone(t_zone **zone_list, t_zone *zone);



#endif
