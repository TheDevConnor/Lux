#pragma once

#include <stddef.h>
#include <stdalign.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG_ARENA_ALLOC 1  // Comment this line to disable debug logs

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
    Buffer *buffer;
    Buffer *head;
} ArenaAllocator;

typedef struct {
    void *data;           // Pointer to the actual array in arena
    size_t count;         // Number of elements in use
    size_t capacity;      // Total elements allocated
    size_t item_size;     // Size of each element
    ArenaAllocator *arena;
} GrowableArray;

/* Buffer management functions */
Buffer* buffer_create(size_t s, size_t alignment);

/* Arena allocator functions */
int arena_allocator_init(ArenaAllocator *arena, size_t size);
void* arena_alloc(ArenaAllocator *arena, size_t size, size_t alignment);
void arena_reset(ArenaAllocator *arena);
void arena_destroy(ArenaAllocator *arena);
char *arena_strdup(ArenaAllocator *arena, const char *str);

/* Growable array functions */
bool growable_array_init(GrowableArray *arr, ArenaAllocator *arena, size_t initial_capacity, size_t item_size);
void* growable_array_push(GrowableArray *arr);

#ifdef __cplusplus
}
#endif