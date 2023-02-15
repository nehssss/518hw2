#ifndef _WINDOW_BIT_COUNT_APX_
#define _WINDOW_BIT_COUNT_APX_

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

uint64_t N_MERGES = 0; // keep track of how many bucket merges occur

/*
    TODO: You can add code here.
*/

typedef struct {
    // TODO: Fill me.
    uint32_t k; // The parameter k determines the relative error of the approximation
    uint32_t wnd_size; // The size of the sliding window
    uint32_t n_buckets; // The number of buckets used by the algorithm
    uint32_t mask; // A mask used to quickly compute the bucket index of a given position
    uint32_t* buckets; // The array of buckets used by the algorithm
    uint32_t n_items; // The number of items processed by the algorithm so far
    uint32_t i_head; // The index of the head of the sliding window
} StateApx;

// k = 1/eps
// if eps = 0.01 (relative error 1%) then k = 100
// if eps = 0.001 (relative error 0.1%) the k = 1000
uint64_t wnd_bit_count_apx_new(StateApx* self, uint32_t wnd_size, uint32_t k) {
    assert(wnd_size >= 1);
    assert(k >= 1);

    // Initialize the state struct
    self->k = k;
    self->wnd_size = wnd_size;
    self->n_buckets = ceil(log2(wnd_size)) + 1;
    self->mask = (1 << self->n_buckets) - 1;
    self->buckets = (uint32_t*) calloc(1 << self->n_buckets, sizeof(uint32_t));
    self->n_items = 0;
    self->i_head = 0;

    // Compute the number of bytes allocated on the heap
    uint64_t bytes_allocated = sizeof(StateApx) + (1 << self->n_buckets) * sizeof(uint32_t);

    return bytes_allocated;
}

void wnd_bit_count_apx_destruct(StateApx* self) {
    // TODO: Fill me.
    // Make sure you free the memory allocated on the heap.
     free(self->buckets);
}

void wnd_bit_count_apx_print(StateApx* self) {
    // This is useful for debugging.
    printf("k = %u, wnd_size = %u, n_buckets = %u, n_items = %u, i_head = %u\n", 
        self->k, self->wnd_size, self->n_buckets, self->n_items, self->i_head);
    for (int i = 0; i < (1 << self->n_buckets); i++) {
        printf("%u ", self->buckets[i]);
    }
    printf("\n");
}

uint32_t wnd_bit_count_apx_next(StateApx* self, bool item) {
 // Increment the number of items processed
    self->n_items++;

    // Remove the oldest item from the sliding window
    uint32_t i_tail = (self->i_head + self->wnd_size - 1) & (self->mask);
    uint32_t bucket_tail = i_tail >> (self->n_buckets - 1);
    self->buckets[bucket_tail]--;

    // Add the new item to the sliding window
    uint32_t bucket_head = self->i_head >> (self->n_buckets - 1);
    self->buckets[bucket_head] += item;

    // Increment the index of the head of the sliding window
    self->i_head = (self->i_head + 1) & (self->mask);

    // If the number of items processed is a power of 2, merge the buckets
    if ((self->n_items & (self->n_items - 1)) == 0) {
        for (int i = 0; i < self->n_buckets - 1; i++) {
            uint32_t j = i << 1;
            self->buckets[i] = self->buckets[j] + self->buckets[j+1];
            N_MERGES++;
        }
    }

    // Compute the approximate bit count and return it
    uint32_t bit_count_apx = 0;
    for (int i = 0; i < self->n_buckets - 1; i++) {
        bit_count_apx += self->table[self->buckets[i]];
    }
    bit_count_apx += self->table[self->buckets[self->n_buckets - 1]] / self->k;
    return bit_count_apx;
}

#endif // _WINDOW_BIT_COUNT_APX_
