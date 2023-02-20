#ifndef _WINDOW_BIT_COUNT_APX_
#define _WINDOW_BIT_COUNT_APX_

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

uint64_t N_MERGES = 0; // keep track of how many bucket merges occur

/*
    TODO: You can add code here.
*/

typedef struct {
    uint32_t timestamp;
    uint32_t size;
} Bucket;

typedef struct {
    uint32_t total_bucket;
    uint32_t wnd_size;
    Bucket* wnd_buffer;
    uint32_t count; // not sure if it is needed
    uint32_t index_oldest; // index pointing to the oldest element    
    uint32_t index_next;
    uint32_t now;
    uint32_t k;
    uint32_t m;
    // uint32_t* groups;
} StateApx;

// k = 1/eps
// if eps = 0.01 (relative error 1%) then k = 100
// if eps = 0.001 (relative error 0.1%) the k = 1000
uint64_t wnd_bit_count_apx_new(StateApx* self, uint32_t wnd_size, uint32_t k) {
    assert(wnd_size >= 1);
    assert(k >= 1);
    
    self->wnd_size = wnd_size;
    self->index_oldest = 0;
    self->index_next = 0;
    self->k = k;
    self->now = 0;
    self->count = 0;
    // self->total_bucket = (2*k)*ceil(log2(wnd_size/k));
    // // self->total_bucket = 200;
    self->total_bucket = 0;
    self->m = ceil(log2(wnd_size/k));
    for (int i = 0; i <= self->m; i++) {
        self->total_bucket += (uint32_t)ceil(((int)(k+2)*pow(2, -i)));
    }
    uint64_t memory = ((uint64_t)self->total_bucket) * sizeof(Bucket);
    self->wnd_buffer = (Bucket*) malloc(memory);
    for (uint32_t i=0; i<self->total_bucket; i++) {
        self->wnd_buffer[i].size = 0;
        self->wnd_buffer[i].timestamp = 0;
    }
    // printf("self->total_bucket = %d\n", self->total_bucket);
    return memory;
}

void wnd_bit_count_apx_destruct(StateApx* self) {
    free(self->wnd_buffer);
}

void wnd_bit_count_apx_print(StateApx* self) {
    // This is useful for debugging.
}

uint32_t wnd_bit_count_apx_next(StateApx* self, bool item) {
    // Add timestamp
    self->now += 1;

    // Check if the oldest bucket is expired
    if (self->now - self->wnd_buffer[0].timestamp >= self->wnd_size){
        memmove(self->wnd_buffer,self->wnd_buffer+1, (self->index_next-2)*sizeof(Bucket));
        self->wnd_buffer[(self->index_next-1)].size = 0;
        self->wnd_buffer[(self->index_next-1)].timestamp = 0;
        self->index_next --;
        self->count = self->count - self->wnd_buffer[0].size;

    }

    // Create new bucket and add to window
    if(item){
        if(self->index_next > self->total_bucket){
            printf("error1111\n");
            printf("self->total_bucket = %d\n", self->total_bucket);
            printf("self->index_next = %d\n", self->index_next);
        }
        else {
            self->count ++;
            self->wnd_buffer[self->index_next].timestamp = self->now;
            self->wnd_buffer[self->index_next].size = 1;
            self->index_next = self->index_next + 1; 
            // printf("self->index_next = %u\n", self->index_next);
            
            int group_count = 1; 
            int group_size = 0;
            // Merge gv
            uint32_t threshhold = self->k + 2;
            for (size_t i=self->index_next-1; (int)i>=0; i--) {
                size_t pointer_bucket = i;
                // printf("pointer_bucket %zu\n" , pointer_bucket);
                if (self->wnd_buffer[pointer_bucket].size == group_count) {
                    group_size += 1;
                    // printf("group_size = %d\n", group_size);
                    if (group_size >= threshhold) {

                        N_MERGES ++;
  
                        self->wnd_buffer[pointer_bucket].size *= 2;
                        self->wnd_buffer[pointer_bucket].timestamp = self->wnd_buffer[(pointer_bucket+1)].timestamp;
                        self->wnd_buffer[(pointer_bucket+1)].size = 0;
                        self->wnd_buffer[(pointer_bucket+1)].timestamp = 0;

                        memmove(self->wnd_buffer+pointer_bucket+1, self->wnd_buffer+pointer_bucket+2,(self->index_next-1-(pointer_bucket+1))*sizeof(Bucket));
                        self->wnd_buffer[self->index_next-1].size = 0; 
                        self->wnd_buffer[self->index_next-1].timestamp = 0; 
                        self->index_next --;
                        group_count *= 2;
                        group_size = 1;
                        threshhold /= 2;
                    }    
                } else {
                    break;
                }
            }
        }
        
    }

    int count_total = self->count - self->wnd_buffer[0].size + 1;

    return count_total;
} 

#endif // _WINDOW_BIT_COUNT_APX_