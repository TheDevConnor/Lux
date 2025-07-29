#pragma once

#include <stddef.h>
#include <stdalign.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Growth configuration
#define ARENA_MIN_BUFFER_SIZE (64 * 1024)    // 64KB minimum
#define ARENA_GROWTH_FACTOR 2                 // Double the size each time
#define ARENA_MAX_BUFFER_SIZE (16 * 1024 * 1024) // 16MB maximum per buffer

// #define DEBUG_ARENA_ALLOC 1  // Comment this line to disable debug logs

#ifdef DEBUG_ARENA_ALLOC
#define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...) ((void)0)
#endif

#ifdef DEBUG_ARENA_ALLOC
static int active_buffers = 0;
#define TRACK_ALLOC(ptr) do { if (ptr) active_buffers++; DEBUG_PRINT("ALLOC %p (count=%d)\n", ptr, active_buffers); } while(0)
#define TRACK_FREE(ptr)  do { if (ptr) active_buffers--; DEBUG_PRINT("FREE  %p (count=%d)\n", ptr, active_buffers); } while(0)
#else
#define TRACK_ALLOC(ptr) ((void)0)
#define TRACK_FREE(ptr) ((void)0)
#endif

typedef struct Buffer {
    size_t size;
    struct Buffer *next;
    char *ptr;
} Buffer;

typedef struct ArenaAllocator {
    size_t offset;
    Buffer *buffer;      // Current buffer
    Buffer *head;        // First buffer
    size_t next_buffer_size; // Size for next buffer allocation
    size_t total_allocated;  // Total memory allocated across all buffers
} ArenaAllocator;

typedef struct {
    void *data;           
    size_t count;         
    size_t capacity;      
    size_t item_size;     
    ArenaAllocator *arena;
} GrowableArray;

/* Buffer management functions */
Buffer* buffer_create(size_t s, size_t alignment);

/* Arena allocator functions */
int arena_allocator_init(ArenaAllocator *arena, size_t initial_size);
void* arena_alloc(ArenaAllocator *arena, size_t size, size_t alignment);
void arena_reset(ArenaAllocator *arena);
void arena_destroy(ArenaAllocator *arena);
char *arena_strdup(ArenaAllocator *arena, const char *str);

/* Arena statistics and debugging */
void arena_print_stats(ArenaAllocator *arena);
size_t arena_get_total_allocated(ArenaAllocator *arena);

/* Growable array functions */
bool growable_array_init(GrowableArray *arr, ArenaAllocator *arena, size_t initial_capacity, size_t item_size);
void* growable_array_push(GrowableArray *arr);

#ifdef __cplusplus
}
#endif