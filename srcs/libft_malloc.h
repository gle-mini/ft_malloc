#ifndef LIBFT_MALLOC_H
# define LIBTF_MALLOC_H


#define TINY_MAX        64
#define SMALL_MAX       1024

#define TINY_ZONE_MULTIPLIER   16
#define SMALL_ZONE_MULTIPLIER  128

#define TINY_ZONE_SIZE   (sysconf(_SC_PAGESIZE) * TINY_ZONE_MULTIPLIER)
#define SMALL_ZONE_SIZE  (sysconf(_SC_PAGESIZE) * SMALL_ZONE_MULTIPLIER)

# include <stdlib.h>
# include <stddef.h>
# include <sys/types.h>
#include <pthread.h>


//=============================================================================
// Data Structures
//=============================================================================

/**
 * @brief Enumeration of memory zone types.
 */
typedef enum e_zone_type {
    TINY,
    SMALL,
    LARGE
} t_zone_type;

/**
 * @brief Header structure for a memory block.
 *
 * Each allocated block has a header storing the size, free flag,
 * and pointers to the next and previous blocks.
 */
typedef struct s_block {
    size_t          size;
    int             free;
    struct s_block  *next;
    struct s_block  *prev;
} t_block;

#define BLOCK_SIZE (sizeof(t_block))

// Zone structure: represents a memory zone allocated with mmap.
/**
 * @brief Structure representing a memory zone allocated via mmap.
 *
 * A zone contains a header and one or more blocks.
 */
typedef struct s_zone {
    t_zone_type     type;
    size_t          size;
    struct s_zone   *next;
    t_block         *blocks;
} t_zone;

//=============================================================================
// Global Zone Lists
//=============================================================================

extern pthread_mutex_t g_mutex;
extern t_zone *g_zones;

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
void remove_zone(t_zone *zone);



#endif
