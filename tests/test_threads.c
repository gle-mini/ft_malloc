#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libft_malloc.h"  // This header declares ft_malloc, ft_free, ft_realloc, show_alloc_mem

#define NUM_THREADS 10
#define NUM_ITERATIONS 1000

void *thread_func(void *arg) {
    (void)arg;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        int op = rand() % 3;
        size_t size = (rand() % 256) + 1;  // Random size between 1 and 256 bytes

        if (op == 0) {
            // Test ft_malloc and ft_free.
            char *ptr = ft_malloc(size);
            if (ptr) {
                memset(ptr, 0xAA, size);
                ft_free(ptr);
            }
        } else if (op == 1) {
            // Test ft_malloc followed by ft_realloc and then ft_free.
            char *ptr = ft_malloc(size);
            if (ptr) {
                memset(ptr, 0xBB, size);
                // Choose a new random size for reallocation.
                size_t new_size = (rand() % 256) + 1;
                char *new_ptr = ft_realloc(ptr, new_size);
                if (new_ptr) {
                    memset(new_ptr, 0xCC, new_size);
                    ft_free(new_ptr);
                } else {
                    // In case ft_realloc fails, free the original allocation.
                    ft_free(ptr);
                }
            }
        } else {
            // Test ft_realloc with a NULL pointer (should behave like ft_malloc).
            size_t new_size = (rand() % 256) + 1;
            char *ptr = ft_realloc(NULL, new_size);
            if (ptr) {
                memset(ptr, 0xDD, new_size);
                ft_free(ptr);
            }
        }
    }
    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREADS];

    // Seed the random generator.
    srand((unsigned)time(NULL));

    // Create multiple threads.
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, NULL) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all threads to finish.
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Multithreaded test completed successfully.\n");

    // Optionally, display the memory state.
    show_alloc_mem();

    return 0;
}
