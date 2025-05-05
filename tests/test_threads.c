#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libft_malloc.h"

#define NUM_THREADS 10
#define NUM_ITERATIONS 1000

void *thread_func() {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        int op = rand() % 3;
        size_t size = (rand() % 256) + 1;

        if (op == 0) {
            char *ptr = ft_malloc(size);
            if (ptr) {
                memset(ptr, 0xAA, size);
                ft_free(ptr);
            }
        } else if (op == 1) {
            char *ptr = ft_malloc(size);
            if (ptr) {
                memset(ptr, 0xBB, size);
                size_t new_size = (rand() % 256) + 1;
                char *new_ptr = ft_realloc(ptr, new_size);
                if (new_ptr) {
                    memset(new_ptr, 0xCC, new_size);
                    ft_free(new_ptr);
                } else {
                    ft_free(ptr);
                }
            }
        } else {
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

    srand((unsigned)time(NULL));

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, NULL) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Multithreaded test completed successfully.\n");

    return 0;
}
